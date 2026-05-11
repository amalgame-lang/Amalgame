# NOTICE

## Authorship and copyright

Amalgame is the work of **Bastien Mouget** (`@BastienMOUGET`), sole
author and copyright holder. Every source file, runtime header,
build script, test, and document under this repository is
© 2026 Bastien Mouget unless explicitly attributed otherwise in a
file header.

All rights are reserved by the author except as licensed under the
Apache License 2.0 (see [LICENSE](LICENSE)).

## You are free to use Amalgame

The Apache-2.0 licence above is permissive. In plain language,
**anyone is free to**:

- **Download, install, and run** every release artefact published
  on the [GitHub Releases page](https://github.com/amalgame-lang/Amalgame/releases) —
  the prebuilt `amc` binaries for Linux, macOS, and Windows are
  yours to use in personal, academic, and commercial work, without
  asking for permission.
- **Read, copy, modify, and redistribute the source** — fork the
  repository, vendor parts of the runtime into your own project,
  patch the compiler for an in-house use case, ship a derivative
  under a different name. Apache-2.0 requires only that you
  preserve this NOTICE + the LICENSE file when you redistribute,
  and state any modifications.
- **Build apps and services on top** — programs you write in
  Amalgame are entirely yours. The compiler's output is your
  code; the runtime that's statically linked into your binary
  is permissive too. There is no "viral" clause that would force
  your application to inherit Amalgame's licence.

Only the **contribution channel** is currently closed — see
[CONTRIBUTING.md](CONTRIBUTING.md). That's a policy on accepting
external patches into this repository; it doesn't restrict any
of the rights above.

## On the use of AI coding assistants

Portions of this codebase were written with the help of an AI
coding assistant (Anthropic's Claude). This is the same kind of
authorship-assistance relationship as using an IDE's
auto-completion, a refactoring tool, or a linter — the AI is a
tool that produces output the author reviews, edits, and commits.
**The AI is not a co-author** within the meaning of intellectual
property law.

Some commits in the git history carry a `Co-Authored-By: Claude
…` trailer. Those trailers are an artifact of the AI tool's
default behaviour and **do not reflect any actual co-authorship
or shared rights** in the work. They are retained for historical
honesty (rewriting the git history would invalidate published
release tags and CI snapshots) but should not be interpreted as
assigning any rights to the AI provider or to any party other
than Bastien Mouget.

From 2026-05-10 onward, new commits no longer carry the
`Co-Authored-By` trailer to remove this ambiguity.

## Third-party components

This section catalogues every third-party piece of code Amalgame
relies on, the licence under which it's distributed, and what each
licence requires when Amalgame is redistributed. Every licence
listed below is **compatible with Apache-2.0** for downstream
redistribution — none impose copyleft or "viral" terms that would
contaminate Amalgame's licence or a user's downstream work.

### Dynamically linked (not vendored)

These libraries are loaded at runtime from the system installation
(`apt install libgc-dev libcurl4-openssl-dev` on Debian, `brew
install bdw-gc curl` on macOS, MSYS2 packages on Windows). Their
source is **not** included in this repository or in release
tarballs, so the only notice obligation is to acknowledge their
use here.

| Library | Used by | Licence | Source |
|---------|---------|---------|--------|
| **Boehm-Demers-Weiser GC** (`libgc`) | every `GC_MALLOC` / `code_alloc` call | Permissive (MIT-style) | https://github.com/ivmai/bdwgc |
| **libcurl** | `Amalgame.Net` (HTTP/TCP) | MIT/X (curl licence) | https://curl.se/libcurl/ |

Both licences require: preserve the upstream copyright notice
when redistributing the library itself, and disclaim warranties.
Since Amalgame does not redistribute either library, those
obligations fall on whoever distributes the binary lib (the OS
package maintainers, Homebrew, MSYS2, etc.) — not on Amalgame.

### Vendored in this repository

No vendored upstream sources ship in the main repo. Optional
backends with vendored amalgamations (currently SQLite) live in
their own opt-in packages, where they carry their own
third-party-licence notice.

| Package | Vendored content | Notice |
|---------|------------------|--------|
| `amalgame-lang/amalgame-database-sqlite` (since v0.1.0) | SQLite 3 amalgamation (public-domain dedication) | https://github.com/amalgame-lang/amalgame-database-sqlite/blob/main/NOTICE.md |

### Build-time tools not bundled in any release

These are used during development or invoked from an install
script users opt into; they are **never** redistributed by us.

| Tool | Role | Licence |
|------|------|---------|
| **NSSM** | service wrapper auto-downloaded by `install.ps1` of the `amc new --template service` Windows path | Public domain |
| **GCC / Clang / MSYS2 mingw** | host C compiler invoked by `build_amc.sh` | GPL (tools, not output — the executables we *produce* are licensable however we want) |

### Licence-compatibility summary

Apache-2.0 (Amalgame) is compatible as a **downstream consumer**
of Permissive / MIT / BSD-style / Public-Domain code, which covers
every dep above. The reverse direction — re-licensing Amalgame's
own code under e.g. GPL by a fork — is allowed by Apache-2.0
(downstream may add restrictions); the reverse-reverse (forking
back to permissive after a GPL fork) is not. None of this matters
for the upstream-Amalgame side: the deps stay under their original
licences, Amalgame stays Apache-2.0.

If a future dependency arrives with a stricter licence (GPL, AGPL,
CC-BY-SA, etc.), it MUST be reviewed for compatibility before
landing — typically by isolating it behind a build flag or
dynamic-link boundary so the rest of the project doesn't inherit
its restrictions.

## Trademarks

"Amalgame" as a project name is the author's; no trademark
registration has been filed at the date of this notice.
