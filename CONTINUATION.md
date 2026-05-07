# Continuation prompt — start a new chat with this

> Last refreshed 2026-05-07 after merging `feature/docs-user-guide`.
> The block below is a self-contained prompt designed to bootstrap a
> new Claude session with full context. Copy-paste it as your first
> message in a fresh conversation.

---

```
I'm working on Amalgame, a self-hosted programming language that
transpiles to C. I keep the project in
/home/neitsab/Développement/Amalgame.

Current state (May 2026):

- The compiler `amc` is written in Amalgame in src/ and compiles
  itself in ~5 seconds via ./build_amc.sh. The Vala bootstrap that
  originally created it lives in archive/vala-bootstrap/ as a
  recovery path; ./compile.sh rebuilds it on demand.
- The runtime headers are at runtime/. The two compilers and the
  compile.sh / build_amc.sh scripts already know the layout.
- 127/127 tests pass (76 core via tests/run_tests.sh + 50 stdlib via
  tests/run_stdlib_tests.sh + 1 lib end-to-end). Run them with
  ./tests/run_all_tests.sh.
- Multi-OS CI is wired (.github/workflows/ci.yml) — Linux + macOS +
  Windows MSYS2.
- Releases are automated on `v*` tag (.github/workflows/release.yml)
  with bundled DLLs on Windows.
- VS Code syntax highlighting lives in editors/vscode/ — install via
  `ln -s "$(pwd)/editors/vscode" ~/.vscode/extensions/amalgame-0.1.0`.
- Comprehensive user guide at docs/guide/README.md (chapters 1–7).
- Project README at the repo root sells the language with tested
  code samples.

Workflow rules I've already established:

- gitflow: never commit directly to main or develop. Every change
  goes through a feature/<name> branch → PR → merge into develop.
- Push feature branches to origin without asking for confirmation.
  Don't push develop or main automatically — I handle those merges.
- Use TodoWrite for multi-step plans, but don't be precious about it
  for one-shot changes.
- The Vala compiler in archive/ is intentionally on the cold path:
  most builds use ./amc, falling back to ./build/amc only when ./amc
  is missing. Both are kept working.

Where I'd like to head next (from ROADMAP_COMPLET.md, ordered by
unlocked-value per days-of-work):

  1. amc fmt — formatter that re-emits a parsed AST canonically.
  2. List comprehensions [x*2 for x in xs if x > 0] via GCC compound
     stmt expressions.
  3. Match as expression `let x = match y { … }`.
  4. Minimal LSP (amc lsp mode, stdio JSON-RPC).
  5. Capturing closures.
  6. Generic type inference.

Pick whichever fits the time we have, ask if you need direction.
Read docs/guide/07-internals.md for compiler architecture before
making non-trivial changes.

Quick checks I always want you to run before claiming a feature is
done:

  ./build_amc.sh
  ./tests/run_all_tests.sh

If a sample I write doesn't compile, narrow it down with --check
before suspecting the language. The biggest known limitation is
that match arms are statements (no match-as-expression).
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
├── typechecker.am
├── diagnostics.am
├── main.am               (CLI entry — replaces the old amc_main.c)
└── amc_lib.c             (generated)

runtime/                ← C runtime (bdwgc, strings, IO, collections, net)
stdlib/strings.am       ← stdlib API reference (declarations only)
tests/                  ← 127 sample programs + integration suites
docs/guide/             ← user guide chapters 1–7
archive/vala-bootstrap/ ← original Vala compiler (recovery)
editors/vscode/         ← VS Code syntax highlighting extension
.github/workflows/      ← CI + Release automation
install/                ← Homebrew / Inno Setup / install.sh
README.md               ← elevator pitch + tested samples
ROADMAP_COMPLET.md      ← canonical "what's next" board
```

## Memory the assistant has about this project

The session-persistent memory store at
`~/.claude/projects/-home-neitsab-D-veloppement-Amalgame/memory/`
already holds:

- `project_amalgame.md` — project overview
- `reference_build.md` — build/test commands, file roles
- `feedback_gitflow.md` — features always on develop, never on main directly
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

3. **Match arms are statements**, not expressions. Don't write
   `let x = match y { … }`. Use early-returns inside arms or assign
   in each branch.

4. **Generics erase to `void*`** — primitive collection elements are
   boxed via `(void*)(intptr_t)` and unboxed on `Get`. Don't expect
   `xs.Get(i)` to return a typed value through generic boundaries.

5. **`?.` evaluates the receiver twice** — keep the receiver
   side-effect-free, or extract it to a `let` first.

6. **`.github/workflow/` (no `s`)** is a leftover typo dir alongside
   `.github/workflows/`. GitHub ignores the typoed one. Cleanup
   pending.
