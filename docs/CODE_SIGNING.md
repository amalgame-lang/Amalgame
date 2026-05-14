# Windows Code Signing — Plan & Status

> Status as of 2026-05-14: **unsigned**. SmartScreen warns on every
> first run; some corporate antivirus products refuse `amc.exe`
> outright or prompt on every invocation.

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

- Public repo on GitHub or GitLab
- OSI-approved license — Amalgame is Apache-2.0 ✓
- Active development with at least a handful of releases
- "Meaningful community traction" — informally ~30 ★, project
  >3 months old. **Amalgame is too young right now** (1 ★, started
  2026-04-23). Re-apply once we cross ~30 ★.

### Application checklist (when ready)

URL: <https://about.signpath.io/product/open-source>

Fill out:
- Project name: **Amalgame**
- Tagline: **Self-hosted programming language that compiles to C**
- Homepage: <https://amalgame.me>
- Source repo: <https://github.com/amalgame-lang/Amalgame>
- License: Apache-2.0
- Maintainer / signing-authority contact: Bastien Mouget
- Build system: **GitHub Actions** (`release.yml`)
- Artifacts to sign:
  - `amalgame-<version>-setup.exe`
  - `amc.exe` (inside `amc-<version>-windows-x86_64.zip`)
  - Optionally: the bundled MinGW DLLs (less critical — they're
    already signed by Microsoft via the original MinGW build, but
    re-signing avoids any "mismatched certs" weirdness)
- Notes for review: mention that we already publish multi-platform
  releases on every tag, that the project is self-hosted, that the
  install pipeline is documented in `install/PUBLISHING.md`.

Review typically takes **1–2 weeks**. They may come back with
follow-up questions; respond promptly.

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

- [ ] Apply to SignPath OSS once Amalgame crosses ~30 ★ on GitHub
  (or once a contributor / sponsor explicitly pushes for it).
- [ ] Decide whether to ALSO sign the bundled MinGW DLLs. The
  binaries are already signed by the MinGW project, but our
  re-distribution puts our name on the package. Probably yes —
  AVs sometimes object to re-distributed-but-not-re-signed DLLs.
- [ ] Document the signing process for the Linux `amc` binary
  (deb signing? rpm signing? OSI signing for `.deb`/`.rpm` is
  trivial, but for raw tarballs we'd need a separate
  `.tar.gz.asc` GPG signature — file under a "L1 hardening"
  follow-up).

---

[authenticode]: https://learn.microsoft.com/en-us/windows-hardware/drivers/install/authenticode
[signpath-oss]: https://about.signpath.io/product/open-source
[smallstep]: https://smallstep.com/docs/step-ca/
