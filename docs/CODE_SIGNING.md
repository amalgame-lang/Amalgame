# Windows Code Signing — Plan & Status

> **Status as of 2026-05-14**: SignPath Foundation OSS Program
> application **submitted**, awaiting review (typical turnaround
> 1-2 weeks). Releases still ship **unsigned** in the meantime —
> SmartScreen warns on every first run, some corporate antivirus
> products refuse `amc.exe` outright or prompt on every invocation.

This doc captures the problem, the planned fix (SignPath.io OSS
program), interim mitigations, and the wiring code that lands once
we get an EV cert.

---

## 1. The problem

Right now everything we ship on Windows is **unsigned**:

- `amalgame-<ver>-setup.exe` (the Inno Setup installer)
- `amc.exe` and its bundled MinGW DLLs (inside the tarball + inside
  the installer payload)
- `vsce`-built `amalgame-<ver>.vsix` (signed by the VS Code
  Marketplace once published, but our pre-publish artifact has no
  signature)

Concrete user-visible consequences:

| Surface | Behaviour today |
|---|---|
| Double-click `setup.exe` on fresh Win 11 | **SmartScreen blocks** ("Microsoft Defender SmartScreen prevented an unrecognized app from starting"). User must click `More info` → `Run anyway`. Some Win 11 25H2 builds gate this behind a Microsoft-account sign-in prompt that looks scary. |
| Corporate antivirus | Varies per vendor. Most flag unsigned EXEs as `Trojan.Generic.<heuristic>` and either quarantine on download or prompt on every launch of `amc.exe`. Reported live by the author on his employer's PC. |
| `amc.exe` invoked by post-install scaffold | If the AV holds the binary, the post-install `amc new MyFirstApp --vscode` step fails silently (or pops a UAC-style elevation dialog the user wasn't expecting). |
| `gcc.exe` from the bundled MinGW toolchain | Some AVs treat the entire MinGW bundle as suspicious — gcc is a compiler so it touches `.tmp` files in odd places during link, which can trigger heuristics. |

The blast radius is broad enough that the install experience is
**actively painful** for anyone not on a personal/dev machine, even
though everything technically works.

## 2. Root cause

We never publish an [Authenticode][authenticode] signature on the
exe + DLLs. Windows treats every unsigned binary as
"editor unknown" → SmartScreen + AV heuristics fire.

The fix is to obtain a code-signing certificate and sign the
release artifacts in CI before they're attached to the GitHub
Release.

## 3. The path: SignPath.io OSS Program

[SignPath][signpath-oss] runs a free program for open-source
projects: they hold an **EV (Extended Validation) certificate** in
their HSM, expose it through a GitHub Actions integration, and
let approved projects sign their artifacts without paying for or
managing the cert directly. This is what Rust (`rustup-init.exe`),
Wireshark, OBS, FileZilla, and dozens of other major OSS projects
use today.

EV is the magic word — it makes SmartScreen trust the artifact
**instantly on first run**, with no reputation-building period.
Equivalent commercial certs cost €250–400/year (with an HSM token
that has to ship physically), so the SignPath free tier is a
~€1500/3-year saving plus less ops overhead.

### Eligibility (rough)

- Public repo on GitHub or GitLab ✓
- OSI-approved license — Amalgame is Apache-2.0 ✓
- Active development with regular releases — 30+ tagged releases
  since 2026-04-23 ✓
- "Meaningful community traction" — informally ~30 ★ + a few
  months of history. Amalgame applied early on **activity merit**
  (30 releases in 3 weeks, full multi-OS CI green, documented
  signing plan in repo) despite the low star count (1). Outcome
  pending.

### Application submitted — what we filled in

URL: <https://signpath.org/apply> (HubSpot-embedded form)

Submitted values:

| Field | Value |
|---|---|
| Project name | Amalgame |
| Tagline | A self-hosted, statically-typed programming language that compiles to portable C. |
| Homepage | <https://amalgame.me> |
| Source repository | <https://github.com/amalgame-lang/Amalgame> |
| License | Apache-2.0 |
| Maintainer | Bastien Mouget — sole maintainer & copyright holder |
| Build system | GitHub Actions (`.github/workflows/release.yml`) |
| Release frequency | 1-3/week during v0.8.x stabilization; long-term bi-weekly |
| Privacy Policy URL | <https://github.com/amalgame-lang/Amalgame#privacy> |
| Maintainer type | Individual |

Artifacts to sign (declared in the "Why does your project need code signing?" field):

- `amalgame-<version>-setup.exe` (Inno Setup installer, ~104 MB)
- `amc.exe` (compiler binary, ~500 KB, bundled inside both the
  `.exe` installer and the `.zip` tarball)
- The bundled MinGW DLLs (already MS-signed, but re-signing under
  our publisher avoids cert-mismatch heuristics in some AVs)

If SignPath comes back with follow-up questions or asks to wait
for more traction, that response goes here (TBD).

### After approval — CI wiring

SignPath will hand you:
- An organization ID
- A project slug (e.g. `amalgame`)
- A signing policy slug (e.g. `release-signing`)
- An artifact configuration slug per binary
- An API token → save it as the GitHub secret `SIGNPATH_API_TOKEN`

Then patch `.github/workflows/release.yml` — between the
`build-windows-installer` job and the `publish` job, insert a
signing step:

```yaml
sign-windows:
  name: Sign Windows artifacts (SignPath)
  runs-on: ubuntu-latest
  needs: build-windows-installer
  permissions:
    contents: read
    id-token: write          # required by signpath-actions/sign
  steps:
    - name: Download unsigned setup.exe
      uses: actions/download-artifact@v4
      with:
        name: release-windows-installer
        path: unsigned/

    - name: Submit setup.exe to SignPath
      uses: signpath/github-action-submit-signing-request@v1
      with:
        api-token: ${{ secrets.SIGNPATH_API_TOKEN }}
        organization-id: <PASTE_ORG_ID>
        project-slug: amalgame
        signing-policy-slug: release-signing
        artifact-configuration-slug: setup-exe
        github-artifact-id: ${{ steps.upload-setup.outputs.artifact-id }}
        wait-for-completion: true
        output-artifact-directory: signed/

    - name: Re-upload signed installer
      uses: actions/upload-artifact@v4
      with:
        name: release-windows-installer-signed
        path: signed/*.exe
        retention-days: 7
```

The `publish` job's `needs:` list adds `sign-windows`, and its
`merge-multiple: true` artifact download picks up the signed file
alongside the others.

For signing `amc.exe` (which is inside the
`amc-<ver>-windows-x86_64.zip`), the cleanest pattern is to sign
**before** zipping — i.e. patch the `build-windows` job to upload
the freshly built `amc.exe` to a separate artifact, sign that
artifact via a second `signpath` step, then re-stage + zip the
signed `amc.exe` into the final tarball. Adds ~2 minutes to the
release pipeline but produces a setup.exe that contains a signed
amc.exe inside.

### Verifying a signed build (post-rollout)

On Windows:
```powershell
Get-AuthenticodeSignature .\amalgame-0.x.y-setup.exe
```
should print `Status: Valid`, `SignerCertificate.Subject:
CN="SignPath Foundation - Amalgame", ...`, and a recent
`TimeStamperCertificate`.

`signtool verify /pa /v amc.exe` from a Windows SDK install
gives more detail.

## 4. Plan B — Sectigo OV via SSL.com (if SignPath delays)

If SignPath declines or the review drags past a release we
absolutely need to ship signed:

- **Sectigo OV code signing via SSL.com**: ~€70/year, fastest
  cheap route. Issuance in ~24 h after Dun & Bradstreet number
  validation (cheap workaround: register as a sole trader, the
  D-U-N-S number is free).
- The cert is delivered as a `.pfx` file → store as the GH secret
  `WINDOWS_CERT_PFX_BASE64`, password as `WINDOWS_CERT_PASSWORD`.
- CI signs with `signtool sign /f cert.pfx /p $env:WINDOWS_CERT_PASSWORD
  /tr http://timestamp.sectigo.com /td sha256 /fd sha256 setup.exe`.
- Caveat: OV does **not** give instant SmartScreen trust. Microsoft
  builds reputation per-publisher over the first ~few thousand
  downloads, after which the warnings disappear. Until then, users
  still see "Run anyway" but **at least the AV part stops firing
  in most cases** because Authenticode-signed artifacts pass most
  AV heuristics regardless of CA tier.

## 5. Plan C — EV cert direct (last resort)

- **DigiCert EV** or **Sectigo EV**: €250–400/year.
- Comes with a physical USB HSM token (FIPS 140-2 L2). The token
  has to be plugged in to sign — can be hosted on a dedicated
  signing host with [smallstep/step-ca][smallstep] + a long-lived
  GitHub self-hosted runner.
- Equivalent SmartScreen instant-trust as SignPath OSS, but you're
  paying for it and managing key ops yourself.

Only worth it if Amalgame stays below the SignPath threshold for
longer than expected AND we need silent installs for a corporate
customer.

## 6. Mitigations until signed

### For end users hitting SmartScreen

The dialog has two visible buttons (`Don't run`) and one almost
hidden link (`More info`). Click `More info` → a third button
appears: `Run anyway`. After clicking once, SmartScreen remembers
that hash and stops warning for that specific binary.

Document this in the install instructions (`docs/guide/01-getting-started.md`)
under a "Windows: SmartScreen first-run warning" subsection.

### For end users behind corporate antivirus

1. **Hash-based whitelist**. Each release publishes a
   `checksums.sha256` file with SHA-256s of every artifact. Send
   to IT: "please whitelist these hashes for our team":
   ```
   <sha256>  amc-X.Y.Z-windows-x86_64.zip
   <sha256>  amalgame-X.Y.Z-setup.exe
   ```
   Many corporate AV products (CrowdStrike Falcon, SentinelOne,
   Microsoft Defender for Endpoint) support hash-based allowlists
   that take effect immediately, fleet-wide.

2. **Per-user trust** (less common but supported by some AVs):
   add `%USERPROFILE%\Amalgame` and `C:\Program Files\Amalgame`
   to the AV's "trusted folders" exclusion list. Requires admin
   rights on the AV, which a security-conscious IT department
   may not grant.

3. **Microsoft Defender false-positive submission**: if the
   blocker is specifically Defender (not a third-party AV),
   submit the binary at <https://www.microsoft.com/en-us/wdsi/filesubmission>
   → check "I believe this file should not be detected as
   malware". Defender pulls the false-positive within 24 h for
   **that specific hash**. Has to be re-submitted on every new
   release until we sign.

### For the maintainer (testing without antivirus interference)

- Use the VirtualBox setup documented in `~/win-vm/setup-vm.sh`
  on the dev box. The snapshot+restore loop (`test-installer.sh`)
  gives a clean Win 11 with Windows Defender only (no enterprise
  AV) — SmartScreen still warns once per binary hash but doesn't
  block.

## 7. Tracking

Open issues / decisions to keep in sight:

- [x] Apply to SignPath OSS — **done 2026-05-14**, awaiting review.
- [ ] Receive SignPath approval (or decline). If declined, decide
  between (a) Sectigo OV ~70 EUR/year as a stop-gap, (b) waiting
  on ★ growth before re-applying, (c) EV cert direct.
- [ ] After approval: paste the SignPath-provided org-id, project
  slug, signing-policy slug, artifact-configuration slug, and
  `SIGNPATH_API_TOKEN` into `release.yml` + GitHub secrets.
- [ ] Verify the first signed release on a clean Win 11 box
  (use `~/win-vm/test-installer.sh` with snapshot rollback).
- [ ] Decide whether to ALSO sign the bundled MinGW DLLs. The
  binaries are already signed by the MinGW project, but our
  re-distribution puts our name on the package. Probably yes —
  AVs sometimes object to re-distributed-but-not-re-signed DLLs.
- [ ] Document the signing process for the Linux `amc` binary
  (deb signing? rpm signing? OSI signing for `.deb`/`.rpm` is
  trivial, but for raw tarballs we'd need a separate
  `.tar.gz.asc` GPG signature — file under a "L1 hardening"
  follow-up).

### Application timeline log

| Date | Event |
|---|---|
| 2026-05-14 | docs/CODE_SIGNING.md initial draft (PR #424) |
| 2026-05-14 | README Privacy section added (PR #428, #429) for the form's Privacy Policy URL field |
| 2026-05-14 | AI-provider data-transfer disclosure added to Privacy section (PR #430, #431) |
| 2026-05-14 | **SignPath OSS application submitted** |
| _pending_ | SignPath review response |
| _pending_ | CI wiring + first signed release |

---

[authenticode]: https://learn.microsoft.com/en-us/windows-hardware/drivers/install/authenticode
[signpath-oss]: https://about.signpath.io/product/open-source
[smallstep]: https://smallstep.com/docs/step-ca/
