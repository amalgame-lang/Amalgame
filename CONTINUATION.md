# Continuation prompt — start a new chat with this

> Last refreshed 2026-05-08 after shipping v0.3.6.
> The block below is a self-contained prompt designed to bootstrap a
> new Claude session with full context. Copy-paste it as your first
> message in a fresh conversation.

---

```
I'm working on Amalgame, a self-hosted programming language that
transpiles to C. I keep the project in
/home/neitsab/Développement/Amalgame.

Current state (May 2026, v0.3.6):

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
  Currently 187 PASS / 0 FAIL / 0 SKIP across 12 suites.
- Multi-OS CI (.github/workflows/ci.yml) — Linux + macOS + Windows
  MSYS2. Linux uses snapshot + self-hosted amc; no Vala in the graph.
  CI compiles with -Wint-conversion as an error on macOS/Windows;
  pin int-typed locals via `let n: int = …` when the codegen erases
  the return type to void* across a method-call boundary.
- Releases automated on `v*` tag (.github/workflows/release.yml).
  Latest is v0.3.6 — see CHANGELOG.md for the per-release detail.
  develop → main → tag is the release flow. Both develop and main
  are protected (force-push + delete blocked, PR required, admin
  bypass allowed).
- VS Code extension in editors/vscode/ — TextMate grammar +
  language config + LSP client (vscode-languageclient over stdio).
  Configurable via `amalgame.serverPath` in user settings to point
  at a local amc build.
- Formatter: `amc fmt file.am` re-emits canonical source with
  comments preserved. Idempotent on every compiler source.
- Linter: `amc --lint file.am` runs static analysis: unreachable
  code, unused locals, shadowed names.
- LSP: `amc lsp` over stdio JSON-RPC. v0.3.4 ships diagnostics on
  didOpen/didChange. v0.3.5 adds textDocument/hover (Markdown
  tooltip with the inferred type) and textDocument/completion
  (every global symbol the resolver knows about).
- User guide at docs/guide/README.md (chapters 1–7).
- Grammar: docs/language/grammar.ebnf mirrors src/parser/parser.am
  exactly. grammar.md is the prose companion.
- README + CHANGELOG at the repo root.

Recently shipped:

  v0.3.4 (the "tooling" release):
  - Capturing closures (single-param expression-bodied)
  - amc --lint extensions: unused locals + shadowed names
  - Process stdlib module: Run + RunCapture
  - amc test runner: discovers *_test.am, aggregates [PASS]/[FAIL]/[SKIP]
  - amc lsp v1: diagnostics-only, plus VS Code client

  v0.3.5 (the "developer experience" release):
  - Generic interfaces: `interface IComparable<T>` parses;
    TypeChecker substitutes T at the implements site and asserts
    every method exists on the class with the matching signature.
    Static contract check, no vtable / dynamic dispatch.
  - amc lsp hover + global completion: AST walk for the deepest
    named node covering the cursor; LookupNodeType reads from
    ExprTypeKeys/Vals; completion enumerates all globals from the
    resolver with CompletionItemKind hints.
  - Lambda v2: multi-param `(x, y) => x+y`, block bodies
    `x => { let d = x*2; return d+1 }`, three-arg `(a,b,c) => …`.
    Routes through new Closure_call2/3 runtime helpers; block
    bodies emit through EmitBlock with an InLambdaBody flag that
    boxes RETURN_STMT values as void*.
  - Hotfix CI -Wint-conversion via explicit `let pn: int`.

  v0.3.6 (the "lambdas in the wild" release):
  - Higher-order List methods: xs.Filter / .Map / .Reduce /
    .ForEach / .Any / .All / .CountIf. The lambda is passed as
    AmalgameClosure* (env follows). CGen emits a GCC compound-
    statement-expression at the call site that allocates the env,
    copies captures in, and yields a fresh closure. InferTypeFromExpr
    knows Filter/Map return AmalgameList*; EmitVarDecl propagates
    the receiver's element type so big.Get(0) lowers with the
    right cast.

Known limitations (deferred to v2.5 final):
- Lambda arguments and results are still (i64) → i64 at the C
  level. xs.Map(x => x.Name) over a List<Class> doesn't yet work;
  needs a TypeChecker layer that infers the lambda signature
  from the formal param at the call site, plus CGen to emit
  non-int lam_N_fn signatures.
- String interpolation `"x: {coll.Count()}"` doesn't propagate
  the inferred AmalgameList* through to the embedded call.
  Workaround: stage in named locals before printing.
- ForEach mutating an enclosing var doesn't accumulate (closures
  capture by value). Reduce is the right tool.

Workflow rules:

- gitflow simplified: feature/<name> → develop → main → tag.
  Never commit directly to main or develop. Both branches are
  protected on GitHub.
- Execute git/gh commands directly (don't ask first). Same for
  bash and file edits — the user wants action, not confirmation
  loops. Asking is for design questions ("scope A or B?",
  "which approach?"), not for permission to run a command. When
  the user says "fais-le", they mean "go".
- Destructive ops (git reset --hard, push --force, branch -D,
  tag deletion) still get confirmed first.
- Use `gh pr create --body "$(cat <<'EOF' … EOF)"` for PR bodies.
  The heredoc avoids shell quoting issues. Keep markdown
  reasonable (don't paste raw triple-backtick blocks containing
  special chars).
- Code, commit messages, PR bodies stay in English. Chat replies
  in French.
- TodoWrite for multi-step plans; one-liners can skip it.

Where to head next (from ROADMAP_COMPLET.md, by unlocked-value /
days-of-work):

  1. Lambda v2.5 — non-int signatures: TypeChecker layer that
     infers the lambda signature from the formal param at the
     call site, plus CGen non-int lam_N_fn emission. Unlocks
     xs.Map(x => x.Name) etc.
  2. Stdlib expansion: pick one or two from DateTime / Json /
     Regex / Random / Encoding / Compress / Crypto / Threading.
     Each is 200-400 LoC. Tied to the open "Stdlib delivery
     model" design question (currently header-only).
  3. LSP member completion: `obj.<cursor>` narrowed to the
     receiver's type. ~150 LoC on top of the v0.3.5 global
     completion.
  4. amc test polish: --runtime <path> flag, per-file timeouts,
     parallel execution.
  5. Process v2: split stderr from stdout via real pipes, add
     timeouts, async streaming.

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
├── linter.am             static analysis      (`amc --lint`)
├── lsp.am                LSP server           (`amc lsp`)
├── typechecker.am
├── diagnostics.am
├── main.am               (CLI: compile, fmt, test, lsp, --lint, --check)
└── amc_lib.c             (generated)

runtime/                ← C runtime (bdwgc, strings, IO, collections, net)
                          _runtime.h: AmalgameClosure + _callN at the top,
                          higher-order list helpers below; both used by
                          v0.3.4-v0.3.6 closure work.
stdlib/strings.am       ← stdlib API reference (declarations only)
tests/                  ← samples + run_*.sh runners (output in /tmp)
docs/guide/             ← user guide chapters 1–7
docs/language/          ← grammar.ebnf + grammar.md
snapshot/               ← amc_lib.c (committed) + amc binary (gitignored)
                          + INFO.md (provenance)
tools/save-snapshot.sh  ← capture a known-good amc after green tests
archive/vala-bootstrap/ ← original Vala compiler (recovery only)
editors/vscode/         ← VS Code extension (TextMate grammar + LSP client)
.github/workflows/      ← CI + Release automation
install/                ← Homebrew / Inno Setup / install.sh sketches
README.md               ← elevator pitch + tested samples
ROADMAP_COMPLET.md      ← canonical "what's next" board
CHANGELOG.md            ← per-release notes (v0.3.2 onwards)
CONTINUATION.md         ← this file
```

## Memory the assistant has about this project

The session-persistent memory store at
`~/.claude/projects/-home-neitsab-D-veloppement-Amalgame/memory/`
already holds:

- `project_amalgame.md` — project overview
- `reference_build.md` — build/test commands, file roles
- `feedback_gitflow.md` — features always on develop, never on main
- `feedback_push.md` — push feature branches without asking
- `feedback_git_commands.md` — execute git/gh/bash/edits directly,
  don't ask permission for each step (only for design questions)
- `feedback_language.md` — chat in French; code/commits stay English

A new session reads these automatically and applies them.

## Common pitfalls when resuming

1. **Bootstrap circularity** — when you add a runtime helper or
   builtin, the running `amc` doesn't know about it until you rebuild.
   `build_amc.sh` step 1 tolerates non-zero exit from `amc` if the
   `.c` file was produced; gcc remains the real correctness gate.

2. **`void*` erasure across method-call boundaries** — the self-host
   codegen emits `void* x = SomeFunc()` instead of carrying the
   typed return when the call chains through a generic List<T>
   field. Two known fixes:
   (a) pin the local: `let pn: int = this.X.Count()`
   (b) split into a typed helper that takes the AstNode parameter:
       `private AstNode? FindX(string name) {
            for ip in 0..this.Symbols.ProgramCount() {
                let prog = this.Symbols.ProgramAt(ip)
                let hit  = this.FindXInProg(prog, name)
                ...
            }
        }
        private AstNode? FindXInProg(AstNode prog, string name) { … }`
   Both patterns are already in use across the compiler. CI on
   macOS/Windows treats -Wint-conversion as an error, so
   warnings ignored locally on Ubuntu will fail the merge.

3. **File order in `AMC_SOURCES`** — `diagnostics.am` must come
   before files that reference `SourceMap` / `SourceSnippet`.
   `main.am` is intentionally **excluded** from the gen_test build
   list because it carries a `Program.Main` that conflicts with
   `gen_test.am`'s own.

4. **Generics still erase to `void*` at the C level** — primitive
   collection elements are boxed via `(void*)(intptr_t)`. Since
   v0.3.3, the cgen tracks elem types for `List<T>` / `Map<K,V>`
   (locals, params, returns, annotations) and emits `(T)…_get(…)`
   so callers see the right type without a manual cast. v0.3.6
   extends this to the result of xs.Filter / xs.Map.

5. **`?.` evaluates the receiver twice** — keep the receiver
   side-effect-free, or extract it to a `let` first.

6. **Snapshot vs Vala** — when introducing new syntax, *take a
   snapshot first* (tools/save-snapshot.sh) so the bootstrap chain
   stays usable. The Vala bootstrap can no longer parse much of
   what's in src/, so it's only useful for a from-scratch recovery
   of an old commit, not for ongoing work.

7. **Multi-line string concatenation** — the self-host parser
   chokes on `let s = "a" +\n   "b" +\n   "c"`. Keep `+`-chains
   on a single line, or use `var s = ""; s = s + "a"; s = s + "b"`.
   Hit this twice in v0.3.5 (LSP JSON builders).

8. **Lambda capture is by value** — `var sum = 0; xs.ForEach(x =>
   sum = sum + x)` won't accumulate; the closure has a copy of sum.
   Use Reduce for accumulation. (See known limitation in current-
   state above.)

9. **Re-tagging a release** — if you need to retag (e.g. fixed CI
   config), delete the existing GitHub Release via the web UI BEFORE
   pushing the new tag. `softprops/action-gh-release@v2` edits
   instead of replacing, leaving stray assets from the old run.
   In practice it's almost always cleaner to bump the patch number.

10. **`.github/workflow/` (no `s`)** is a leftover typo dir alongside
    `.github/workflows/`. GitHub ignores the typoed one. Cleanup
    pending.
