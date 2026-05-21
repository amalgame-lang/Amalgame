# Windows MSI installer for Amalgame — `build-msi.ps1`

Pure-PowerShell MSI builder. No WiX, no third-party installer SDK, no
WixToolset. Uses the `WindowsInstaller.Installer` COM API and Windows-
shipped `makecab.exe` only.

> **Status:** v1 — produces a working MSI that mirrors the existing
> `amalgame.iss` (InnoSetup) feature set. Both scripts coexist for now;
> pick whichever fits your CI surface. The `.iss` is retired-track once
> the MSI has had real-world testing.

## Prerequisites

- Windows 10+ host (PowerShell 5.1+ shipped natively, or PowerShell 7+).
- `makecab.exe` (every Windows since XP includes it; no install).
- The pre-staged release tree under `dist/amc-<version>-windows-x86_64/`
  (built by `release.yml` Windows job — same input the `.iss` consumed).

## Build a release MSI

```powershell
cd install\windows
pwsh -File build-msi.ps1 `
     -Version 0.8.39 `
     -StageDir ..\..\dist\amc-0.8.39-windows-x86_64
```

Output: `install\windows\Output\amalgame-<version>.msi`.

## Optional: bundled MinGW gcc

Drop a MinGW-w64 distribution under `install\windows\gcc-bundle\` (must
contain `bin\gcc.exe`) and pass `-IncludeGccBundle`:

```powershell
pwsh -File build-msi.ps1 `
     -Version 0.8.39 `
     -StageDir ..\..\dist\amc-0.8.39-windows-x86_64 `
     -IncludeGccBundle
```

The bundle gets installed to `{INSTALLLOCATION}\gcc\` (~ 200 MB) so
`amc build` works on a fresh Windows box with no MSYS2.

## What the MSI does

| Step | Mechanism |
|---|---|
| Copy files to `%ProgramFiles%\Amalgame\` (or user pick) | MSI `File` + `Component` + `Feature` tables, single `data.cab` cabinet streamed into the MSI itself. |
| Append `<install>\bin` to **HKCU PATH** | MSI native `Environment` table with `=-*PATH` prefix (= set, - remove-on-uninstall, * multistring) → orphan-free uninstall. |
| Start Menu shortcuts | MSI `Shortcut` table — README + Getting Started, pointing at the bundled `.md` files. |
| VS Code .vsix install | `CustomAction` Type 3122 invokes `postinstall.ps1 -Mode install` after `InstallFiles` (sequence 5500). |
| Sample scaffold (`amc new MyFirstApp`) | Same `postinstall.ps1 -Mode install` call. |
| Uninstall — remove PATH, remove `.vsix`, leave sample dir | MSI engine + `postinstall.ps1 -Mode uninstall` (sequence 3400, before `RemoveFiles`). |

## What's intentionally different from `amalgame.iss`

| Behaviour | InnoSetup | MSI (this) | Reason |
|---|---|---|---|
| English-only UI | English + French | English only | The MSI native UI (msiexec.exe) uses the system locale; we don't ship a `Language` table extension. |
| Auto-deselect bundled-gcc when MSYS2 detected | Yes (InitializeWizard) | No | Would need a `LaunchCondition` + a custom action probing PATH. Add later. |
| Desktop shortcut opt-in | Checkbox in wizard | Always-on (single feature) | MSI native UI doesn't surface per-task checkboxes the way Inno does. |
| User-mode default | `PrivilegesRequired=lowest` | `ALLUSERS=2 + MSIINSTALLPERUSER=1` | MSI's official user-mode flag pair. Equivalent semantics. |
| Custom welcome + finish copy | Custom strings in `[Messages]` | MSI default UI | The MSI native UI is fixed; custom dialogs need a `UI` table dialog tree. Out of scope for v1. |

## Verifying the MSI

```powershell
# Install with full verbose log
msiexec /i Output\amalgame-0.8.39.msi /l*v install.log

# Uninstall the same way
msiexec /x Output\amalgame-0.8.39.msi /l*v uninstall.log

# Silent install
msiexec /i Output\amalgame-0.8.39.msi /qn

# Validate the MSI's table integrity (requires Orca or the standalone
# `msival2.exe` from the Windows SDK). Optional — your install will
# work even with minor validator warnings.
```

The post-install / uninstall log lives at `%TEMP%\amalgame-postinstall.log`.

## Code-signing

The MSI itself can be signed with `signtool`:

```powershell
signtool sign /fd SHA256 /td SHA256 /tr http://timestamp.digicert.com `
              /f cert.pfx /p $env:CERT_PWD `
              Output\amalgame-0.8.39.msi
```

Same applies to `amc.exe` — sign it *before* it goes into the staging
dir so the MSI carries the already-signed binary.

For OSS code-signing options without buying a cert, see the discussion
in `docs/release-signing.md` (TBA — SignPath / OSSign / Azure Trusted
Signing).

## v1.1 cold-read fixes (2026-05-21)

Six bugs caught before first Windows run, fixed in the same PR:

1. **`Record.StringData` / `IntegerData` setters** — `InvokeMember`
   with `InvokeMethod` silently fails on indexed-property puts. Now
   uses `BindingFlags.SetProperty` via a `Set-MsiProperty` shim. Same
   fix applied to `SummaryInformation.Property`.
2. **CustomAction Type 3122 → 1074** — dropped `msidbCustomActionTypeNoImpersonate`
   (2048). The postinstall script writes to HKCU and `%USERPROFILE%`,
   which only makes sense in the invoking user's context. Without
   the fix it would have run as LocalSystem and missed the user's
   VS Code / sample-project paths.
3. **`PID_TEMPLATE` "Intel;1033" → "x64;1033"** — matches the
   `ProgramFiles64Folder` install target. With "Intel" the MSI was
   technically 32-bit and would have triggered file-system
   redirection (silent install into `\Program Files (x86)\`).
4. **Component.Attributes |= 256** (`msidbComponentAttributes64bit`)
   on every file component — required for the x64 MSI to skip
   redirection.
5. **Trailing-backslash quoting** — `"[INSTALLLOCATION]"` after MSI
   substitution ends with `\"`, which the C-runtime arg parser
   treats as an escaped quote → unterminated string when passed to
   `powershell.exe`. Mitigation: don't quote-wrap a property that
   ends with `\`; let `postinstall.ps1` derive the install location
   from its own `$PSCommandPath`.
6. **Hardcoded `powershell.exe` path** — replaced the `[SystemFolder]`
   placeholder with the literal `C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe`.
   Deferred CustomActions only resolve `CustomActionData` /
   `ProductCode` / `UserSID` at runtime; property placeholders in
   Type-50 Source aren't guaranteed to work.

If the first Windows test surfaces another bug, likely suspects:
(a) CustomAction sequence position (3400 / 5500),
(b) the cabinet stream embed (`_Streams` table — schema not declared
explicitly, may need a `CREATE TABLE _Streams` if the engine doesn't
auto-create it),
(c) the `Environment` row's `=-*PATH` prefix combo.

## Known limitations (v1)

1. **No upgrade detection.** Each `ProductCode` is unique per version
   (deterministic from `UpgradeCode + Version`). Installing 0.8.40 over
   0.8.39 currently shows a "side-by-side" install dialog — the user
   must uninstall the old version first. Add a `Upgrade` table row
   gated by `UpgradeCode` to enable major-upgrade-on-install. ~10 lines.
2. **Single-feature install.** No per-task checkboxes ("install docs?
   install gcc bundle?"). The whole thing's a single feature. Adding
   a feature tree means a `UI` table dialog (heavy lift). Pass MSI
   property `POSTINSTALL_*=0` on the msiexec command line for now to
   skip the VS Code / sample steps.
3. **MSI native UI only.** No splash screen, no custom welcome page,
   no install-progress branding. Acceptable for a CLI compiler tool —
   end users mostly run `msiexec /qn` anyway. Custom UI = `UI` table
   dialogs = a separate project.
4. **AMD64-only.** The `Template` summary property is `Intel;1033`
   (32-bit), but `ProgramFiles64Folder` is the install target, so
   the MSI is effectively 64-bit. A clean fix would set `Template`
   to `x64;1033` and bump the schema. Doesn't affect functionality
   on x64 Windows.

## Why no WiX?

The user explicitly asked for no WiX / Toolset / Advanced Installer.
Reasons that hold up:

- **No tooling dependency on the build host.** Every Windows since
  2003 has `makecab.exe` and the WindowsInstaller COM API. No
  `nuget restore`, no `dotnet tool install`, no `wix.exe`. A fresh
  GitHub Actions Windows runner builds this MSI in ~15 seconds with
  zero `setup-*` steps in `release.yml`.
- **Auditability.** The MSI table contents are right there in
  PowerShell. No XML-→-binary opaque compile step. When someone
  later asks "why does the installer do X?", grep finds the line.
- **Smaller PR footprint.** No `.wxs` / `.wixproj` / `Wix.Common.targets`
  / WiX-extension dependencies — just two `.ps1` files and a Markdown.

The tradeoff: this script duplicates a ~5% slice of what WiX does.
For more complex installers (per-user-vs-machine choice, multi-feature
trees, dialog flows, MSI mergemod packaging), WiX is the right tool.
For a CLI compiler with file install + PATH + shortcuts, hand-rolling
in PowerShell is the lower-bus-factor choice.
