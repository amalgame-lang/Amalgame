# 6 · Build & tooling

This chapter walks through every script and workflow that builds,
tests, and ships Amalgame.

## The compiler and its snapshot

Amalgame keeps the self-hosted compiler in tree, plus a portable
known-good snapshot:

- **`./amc`** — self-hosted, written in Amalgame. The everyday
  compiler. Source in `src/`, output of `./build_amc.sh`.
- **`./snapshot/amc`** — last known-good `amc` captured by
  `tools/save-snapshot.sh`. The portable `snapshot/amc_lib.c` is
  committed; one `gcc` invocation rebuilds the binary on any platform
  (see `snapshot/INFO.md`). This is the cold-start entry point and
  the recovery rung when `./amc` is broken mid-development.

## Cold-start bootstrap

From a clean clone:

```bash
gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc
./build_amc.sh
```

Dependencies (Linux):

- gcc
- libgc-dev
- libcurl4-openssl-dev (for the Amalgame stdlib's Net module)

macOS uses `brew install bdw-gc curl`; Windows uses MSYS2 MinGW64
(`mingw-w64-x86_64-{gcc,gc,curl}`). The same `snapshot/amc_lib.c` is
the cross-platform entry point everywhere.

## `./build_amc.sh` — self-host build

The five-second loop:

```
Step 1   ./amc src/lexer/*.am src/parser/*.am … src/generator/gen_test.am -o gen_test
         gcc -O2 -Iruntime gen_test.c -o gen_test
Step 2   ./gen_test                 # generates src/amc_lib.c (and inspection bundles)
Step 3   gcc -Iruntime src/amc_lib.c -lgc -lm -lcurl -o amc
```

Notes:

- Step 1 uses `./amc` if it exists, otherwise `./snapshot/amc`. From
  a clean clone, build `./snapshot/amc` first via the cold-start
  command above.
- Step 1 tolerates non-zero exit from `amc` as long as the `.c`
  output was produced. This is the recurring "I just added a builtin
  and the running amc doesn't know it yet" case — gcc remains the
  real correctness gate.
- `main.am` is intentionally **excluded** from the gen_test source
  list. It declares its own `Program.Main` (the CLI entry), which
  would clash with `gen_test.am`'s `Program.Main` in the bundled
  binary. `main.am` is only compiled into `amc_lib.c` via gen6 in
  step 2.
- File order in `AMC_SOURCES` matters for the bootstrap CGen: classes
  must appear before their dependents (since pass-2 emits forward
  decls + bodies file-by-file). `diagnostics.am` is listed before
  `resolver.am` for that reason — `SourceMap` and `SourceSnippet`
  have to be visible.

The build is hermetic — no environment variables, no global state.
Re-running `./build_amc.sh` is always safe.

## `./tests/run_all_tests.sh` — test suite

Two suites:

- **Core / advanced / namespace / interfaces / enums / match / lib /
  stdlib basics** (`tests/run_tests.sh`) — runs each `tests/samples/*.am`
  through `./amc` and grep-checks the output.
- **Stdlib** (`tests/run_stdlib_tests.sh`) — focused on
  String/Collections/Net runtime.

```bash
./tests/run_all_tests.sh
# → 363 PASS / 0 FAIL / 0 SKIP
```

## Continuous integration

`.github/workflows/ci.yml` — runs on every push and PR to `main` /
`develop`:

| Job     | Runs on                | What it does                                                       |
| ------- | ---------------------- | ------------------------------------------------------------------ |
| linux   | ubuntu-latest          | apt deps · `gcc snapshot/amc_lib.c` · `./build_amc.sh` · tests     |
| macos   | macos-latest (arm64)   | brew deps · `gcc src/amc_lib.c` · smoke compile hello              |
| windows | windows-latest (MSYS2) | pacman deps · `gcc src/amc_lib.c` · smoke compile hello            |

All three platforms validate that the tracked `amc_lib.c` is
portable and produces a working binary.

## Releases

`.github/workflows/release.yml` — runs on every push of a `v*` tag:

| Job           | What it produces                                         |
| ------------- | -------------------------------------------------------- |
| build-linux   | `amc-X.Y.Z-linux-x86_64.tar.gz` + `.sha256`              |
| build-macos   | `amc-X.Y.Z-macos-arm64.tar.gz` + `.sha256`               |
| build-windows | `amc-X.Y.Z-windows-x86_64.zip` (DLLs bundled) + `.sha256`|
| publish       | aggregates checksums, creates a GitHub Release           |

The Windows zip bundles the MinGW DLLs the binary actually links
against (libgc, libcurl, libgcc_s_seh, libwinpthread, etc.) — users
install the zip and run `amc.exe` without any external dependency.

Trigger a release:

```bash
git tag v0.4.0
git push origin v0.4.0
# CI takes ~10 minutes, the release shows up on GitHub when done.
```

`workflow_dispatch` is also enabled for testing the workflow without
cutting a real release.

## Inno Setup installer (Windows)

`install/windows/amalgame.iss` produces a `.exe` installer for
Windows users via [Inno Setup 6+](https://jrsoftware.org/isinfo.php):

```bash
# Drop a portable MinGW64 (e.g. from winlibs.com) into install/windows/gcc-bundle/
iscc install/windows/amalgame.iss
# → Output/amalgame-X.Y.Z-setup.exe
```

The installer ships `amc.exe`, `runtime/_runtime.h`, the docs, and
the bundled MinGW64 toolchain so users get a working `amc` + `gcc`
out of the box.

## Homebrew formula

`install/homebrew/amalgame.rb` — for `brew tap` distribution. Update
the version and SHA256 every release.

## Local hygiene

- `git clean -fdx` will remove generated `.c` and binaries in the
  working tree. Be careful — it also removes `gen_test`, `amc`, and
  `src/amc_lib.c`. Use `./build_amc.sh` afterwards to regenerate.
- `./CLEANUP.sh` is a legacy helper that removed older debug
  artefacts. It's mostly a no-op today.

## Editor support

`editors/vscode/` is a complete VS Code extension (TextMate grammar,
language configuration, LSP client, README). Install for development:

```bash
ln -s "$(pwd)/editors/vscode" ~/.vscode/extensions/amalgame-0.1.0
# Reload window: Ctrl+Shift+P → Developer: Reload Window
```

The extension spawns `amc lsp` on `.am` files via stdio JSON-RPC.
Point it at your local build by setting in your VS Code settings:

```json
"amalgame.serverPath": "/abs/path/to/Amalgame/amc"
```

The LSP server (since v0.3.4 for diagnostics, v0.3.5 for hover +
completion) currently provides:

- **Diagnostics** — resolver + typechecker errors published on
  every `didOpen` / `didChange`, with the offending token
  underlined.
- **Hover** — Markdown tooltip showing the inferred type of the
  identifier under the cursor (`name: type`). `null` when the
  cursor isn't on a typed expression.
- **Completion** — global symbol list (built-in types,
  functions, user classes / enums) with `CompletionItemKind`
  hints. The `.` trigger is reserved for v2.5 member completion
  (`obj.<cursor>` narrowed to the receiver's type).

`amc lsp` runs in stdio mode by default. Manually drive it for
debugging via:

```bash
echo -e 'Content-Length: 41\r\n\r\n{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}' | ./amc lsp
```
