# Continuation prompt — start a new chat with this

> Last refreshed 2026-05-08 after shipping v0.3.3.
> The block below is a self-contained prompt designed to bootstrap a
> new Claude session with full context. Copy-paste it as your first
> message in a fresh conversation.

---

```
I'm working on Amalgame, a self-hosted programming language that
transpiles to C. I keep the project in
/home/neitsab/Développement/Amalgame.

Current state (May 2026, v0.3.3):

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
  Currently 150 PASS / 0 FAIL / 0 SKIP — first time the suite is
  fully green with an empty SKIP list.
- Multi-OS CI (.github/workflows/ci.yml) — Linux + macOS + Windows
  MSYS2. Linux uses snapshot + self-hosted amc; no Vala in the graph.
- Releases automated on `v*` tag (.github/workflows/release.yml).
  Latest is v0.3.3 — see CHANGELOG.md for the per-release detail.
  develop → main → tag is the release flow.
- VS Code syntax highlighting in editors/vscode/.
- Formatter: `amc fmt file.am` re-emits canonical source with
  comments preserved (incl. trailing same-line comments and
  `import` directives). Idempotent on every compiler source.
- Linter: `amc --lint file.am` runs static analysis. MVP catches
  unreachable code; the skeleton in src/linter.am is set up to
  grow more checks (unused locals, shadowed names, …).
- User guide at docs/guide/README.md (chapters 1–7).
- README + CHANGELOG at the repo root.

Recently shipped (v0.3.2 + v0.3.3):
- try/catch/throw/finally end-to-end in the self-hosted parser
  (was a regression vs the Vala bootstrap).
- Type.Variant patterns in match (`Direction.North => …`).
- Generic type inference for List<T> and Map<K,V>: locals, params,
  returns, and explicit annotations all carry the elem type, so
  `xs.Get(i)` lowers with the right cast (no manual `(int)`).
- obj.Method() instance syntax for strings (`s.Length()`,
  `"foo".Trim()`, …).
- Multi-file type checking + filename-per-program in error
  reporting + bare `return` recognition (build_amc.sh is silent now).
- Three null-safety bugs: NodeKey collision, lexer dropped bare `?`,
  TypeToC doubled the C pointer for `T?`.
- amc --lint MVP (dead code).

Workflow rules:

- gitflow simplified: feature/<name> → develop → main → tag.
  Never commit directly to main or develop.
- Push feature branches to origin without asking. The user handles
  merges of develop and main and tags.
- Use `gh pr create --body-file /tmp/pr_body.md` (not heredoc) to
  avoid prompt-` issues with markdown quoting in PR bodies.
- TodoWrite for multi-step plans; one-liners can skip it.
- `git branch --show-current` before every commit (I once or twice
  committed straight to develop in earlier sessions; the user
  caught it; please don't repeat).
- Code, commit messages, PR bodies stay in English. Chat replies
  in French.

Where to head next (from ROADMAP_COMPLET.md, by unlocked-value /
days-of-work):

  1. Capturing closures — `let counter = make_counter()`. Capture
     analysis at parse time + heap-allocated env structs.
     Touches Parser + CGen; medium-large.
  2. Minimal LSP — `amc lsp` mode, stdio JSON-RPC over the existing
     Lexer/Parser/Resolver/TypeChecker. Smallest LSP that does
     completion + hover is a few hundred lines.
  3. amc test runner — discover *_test.am, compile, run, aggregate.
     Needs a Process API in the stdlib first (no `Process_Run`
     helper today; runtime uses `system()` only for clear-screen).
  4. amc lint extensions — unused locals, shadowed names,
     suspicious patterns. Skeleton already in src/linter.am.
  5. Generic interfaces (`IComparable<T>`) — modest extra work
     after the generic inference that's now in.

Quick checks before claiming a feature is done:

  ./build_amc.sh
  ./tests/run_all_tests.sh

For non-trivial compiler changes, read docs/guide/07-internals.md
first. The pipeline is single-pass: lex → parse → resolve →
typecheck → (lint?) → cgen.
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
├── formatter/            formatter.am          (`amc fmt`)
├── linter.am             static analysis      (`amc --lint`, since v0.3.3)
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
dist/                   ← release.yml staging dir (gitignored since v0.3.3)
install/                ← Homebrew / Inno Setup / install.sh sketches
README.md               ← elevator pitch + tested samples
ROADMAP_COMPLET.md      ← canonical "what's next" board
CHANGELOG.md            ← per-release notes (since v0.3.2)
```

## Memory the assistant has about this project

The session-persistent memory store at
`~/.claude/projects/-home-neitsab-D-veloppement-Amalgame/memory/`
already holds:

- `project_amalgame.md` — project overview
- `reference_build.md` — build/test commands, file roles
- `feedback_gitflow.md` — features always on develop, never on main
- `feedback_push.md` — push feature branches without asking
- `feedback_language.md` — chat in French; code/commits stay English

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

3. **Generics still erase to `void*` at the C level** — primitive
   collection elements are boxed via `(void*)(intptr_t)`. Since
   v0.3.3, the cgen tracks elem types for `List<T>` / `Map<K,V>`
   (locals, params, returns, annotations) and emits `(T)…_get(…)`
   so callers see the right type without a manual cast. The
   underlying C representation hasn't changed.

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
   In practice it's almost always cleaner to bump the patch number
   instead (we did that for v0.3.2 → v0.3.3).

7. **`dist/` is now gitignored** (since v0.3.3). Prior to that, a
   stale v0.3.0 tarball was tracked there and was bleeding into
   every release via the `dist/*.tar.gz` upload glob. Don't add
   binaries back to it.

8. **`.github/workflow/` (no `s`)** is a leftover typo dir alongside
   `.github/workflows/`. GitHub ignores the typoed one. Cleanup
   pending.
