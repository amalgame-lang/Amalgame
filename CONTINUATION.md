# Continuation prompt — start a new chat with this

> Last refreshed 2026-05-08 after merging `feature/release-v0.3.1`
> and the test-artifact cleanup.
> The block below is a self-contained prompt designed to bootstrap a
> new Claude session with full context. Copy-paste it as your first
> message in a fresh conversation.

---

```
I'm working on Amalgame, a self-hosted programming language that
transpiles to C. I keep the project in
/home/neitsab/Développement/Amalgame.

Current state (May 2026, v0.3.1):

- The compiler `amc` is written in Amalgame in src/ and compiles
  itself in ~5 seconds via ./build_amc.sh.
- 3-rung bootstrap chain in build_amc.sh:
    ./amc                  → current self-hosted (may break in dev)
    ./snapshot/amc         → last known-good amc, captured by
                             tools/save-snapshot.sh after green tests.
                             snapshot/amc_lib.c is committed.
    ./build/amc            → Vala bootstrap in archive/vala-bootstrap/,
                             no longer in CI but kept locally for cold
                             starts (./compile.sh).
- The runtime headers are at runtime/. Cross-platform (POSIX +
  Windows winsock2 via #ifdef _WIN32 in Amalgame_Net.h).
- Test runner (./tests/run_all_tests.sh) drives ./amc directly.
  Build artefacts go to /tmp via mktemp; the source tree stays clean.
  Currently 121 PASS / 0 FAIL / 10 SKIP. The SKIPs are samples that
  trigger known compiler bugs tracked in ROADMAP_COMPLET.md.
- Multi-OS CI (.github/workflows/ci.yml) — Linux + macOS + Windows
  MSYS2. Linux now uses snapshot + self-hosted amc, no Vala in the
  graph.
- Releases automated on `v*` tag (.github/workflows/release.yml).
  Latest is v0.3.1 (records init, tuple typecheck, match arm
  statements). Source tag → main commit; develop merges to main
  for releases.
- VS Code syntax highlighting in editors/vscode/.
- Formatter: `amc fmt file.am` re-emits canonical source with
  comments preserved. Idempotent on every compiler source.
- User guide at docs/guide/README.md (chapters 1–7).
- README at the repo root.

Workflow rules:

- gitflow simplified: feature/<name> → develop → main → tag.
  Never commit directly to main or develop.
- Push feature branches to origin without asking. The user handles
  merges of develop and main and tags.
- Before pushing a tag retag, delete the existing GitHub Release
  in the web UI to avoid mixed-asset releases (we've been bitten).
- TodoWrite for multi-step plans; one-liners can skip it.
- `git branch --show-current` before every commit (I forgot twice
  in past sessions and committed straight to develop — the user
  caught it both times; please don't repeat).

Where to head next (from ROADMAP_COMPLET.md, ordered by
unlocked-value per days-of-work):

  1. Solder the SKIPped samples — Type.Variant patterns in match
     (enums.am), try/catch (try_catch.am, parser regression vs Vala),
     null-safety typechecker bugs (null_safety.am, null_safe_member.am).
  2. Minimal LSP — amc lsp mode, stdio JSON-RPC.
  3. Capturing closures.
  4. Generic type inference.

Quick checks before claiming a feature is done:

  ./build_amc.sh
  ./tests/run_all_tests.sh

For non-trivial compiler changes, read docs/guide/07-internals.md
first. The pipeline is single-pass: lex → parse → resolve →
typecheck → cgen.
```

---

## What's actually in the repo right now

For quick recall when reopening the project:

```
src/                    ← Amalgame compiler in Amalgame
├── lexer/                token.am, lexer.am
├── parser/               ast.am, parser.am
├── resolver/             symbol.am, resolver.am
├── generator/            c_gen.am, gen_test.am
├── formatter/            formatter.am          (NEW since v0.2.0)
├── typechecker.am
├── diagnostics.am
├── main.am               (CLI: compile or `fmt` subcommand)
└── amc_lib.c             (generated)

runtime/                ← C runtime (bdwgc, strings, IO, collections, net)
                          Net.h is cross-platform via #ifdef _WIN32.
stdlib/strings.am       ← stdlib API reference (declarations only)
tests/                  ← samples + run_*.sh runners (output in /tmp)
docs/guide/             ← user guide chapters 1–7
snapshot/               ← amc_lib.c (committed) + amc binary (gitignored)
                          + INFO.md (provenance)
tools/save-snapshot.sh  ← capture a known-good amc after green tests
archive/vala-bootstrap/ ← original Vala compiler (recovery only)
editors/vscode/         ← VS Code syntax highlighting extension
.github/workflows/      ← CI + Release automation
install/                ← Homebrew / Inno Setup / install.sh sketches
README.md               ← elevator pitch + tested samples
ROADMAP_COMPLET.md      ← canonical "what's next" board
```

## Memory the assistant has about this project

The session-persistent memory store at
`~/.claude/projects/-home-neitsab-D-veloppement-Amalgame/memory/`
already holds:

- `project_amalgame.md` — project overview
- `reference_build.md` — build/test commands, file roles
- `feedback_gitflow.md` — features always on develop, never on main
- `feedback_push.md` — push feature branches without asking

A new session reads these automatically and can apply them.

## Common pitfalls when resuming

1. **Bootstrap circularity** — when you add a runtime helper or
   builtin, the running `amc` doesn't know about it until you rebuild.
   `build_amc.sh` step 1 tolerates non-zero exit from `amc` if the
   `.c` file was produced; gcc remains the real correctness gate.

2. **File order in `AMC_SOURCES`** — `diagnostics.am` must come
   before files that reference `SourceMap` / `SourceSnippet`.
   `main.am` is intentionally **excluded** from the gen_test build
   list because it carries a `Program.Main` that conflicts with
   `gen_test.am`'s own.

3. **Generics erase to `void*`** — primitive collection elements are
   boxed via `(void*)(intptr_t)` and unboxed on `Get`. Don't expect
   `xs.Get(i)` to return a typed value through generic boundaries.

4. **`?.` evaluates the receiver twice** — keep the receiver
   side-effect-free, or extract it to a `let` first.

5. **Snapshot vs Vala** — when introducing new syntax, *take a
   snapshot first* (tools/save-snapshot.sh) so the bootstrap chain
   stays usable. The Vala bootstrap can no longer parse much of
   what's in src/, so it's only useful for a from-scratch recovery
   of an old commit, not for ongoing work.

6. **Re-tagging a release** — if you need to retag (e.g. fixed CI
   config), delete the existing GitHub Release via the web UI BEFORE
   pushing the new tag. `softprops/action-gh-release@v2` edits
   instead of replacing, leaving stray assets from the old run.

7. **`.github/workflow/` (no `s`)** is a leftover typo dir alongside
   `.github/workflows/`. GitHub ignores the typoed one. Cleanup
   pending.
