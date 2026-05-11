# 7 · Compiler internals

Welcome, contributor. This chapter is the architecture reference for
people hacking on `amc` itself. The compiler is a single straight
pipeline: source → lexer → parser → resolver → typechecker → cgen → C.

If you came here because the tests broke after your change, jump
straight to the "Adding a feature" recipes near the end.

## Pipeline shape

```
foo.am ─▶ Lexer ─▶ tokens ─▶ Parser ─▶ AST ─▶ Resolver ─▶ TypeChecker ─▶ CGen ─▶ foo.c
                                       │           │            │           │
                                       │           │            │           └─ src/generator/c_gen.am
                                       │           │            └────────────  src/typechecker.am
                                       │           └─────────────────────────  src/resolver/{symbol,resolver}.am
                                       └─────────────────────────────────────  src/parser/{ast,parser}.am
                                                                               src/lexer/{token,lexer}.am
```

`src/main.am` (`AmalgameCompiler.Run`) drives the pipeline. Each
phase is a separate class in its own file.

## Lexer (`src/lexer/`)

- `token.am` defines `enum TokenType` (~130 variants: keywords,
  punctuation, operators) and the `Token` class.
- `lexer.am` walks the source byte-by-byte, recognising identifiers,
  numbers, strings (incl. `\xHH`, `\uHHHH`, `"""`), comments, and
  punctuation. Tokens carry `(Type, Value, Line, Column, Filename)`.

Adding a new token:

1. Add the variant to `TokenType` in `src/lexer/token.am`.
2. Recognise it in `lexer.am` — usually inside the symbol-reading
   block (`else if (c == "@") { ... }`) or the keyword lookup
   (`if (word == "guard") { ... }`).

## Parser (`src/parser/`)

- `ast.am` defines `enum NodeKind` (CLASS_DECL, METHOD_DECL, IF_STMT,
  …) and the universal `AstNode` class with fields:
  - `Kind`, `Name`, `Str`, `Str2`, `Flag`, `Flag2`
  - `Left`, `Right`, `Cond`, `Body`, `Else` (nullable AstNode refs)
  - `Children`, `Params`, `Args` (lists of AstNode)
- `parser.am` is a recursive descent parser with a Pratt-style
  precedence climbing for expressions.

Each construct has a dedicated parser function (`ParseDecl`,
`ParseClass`, `ParseMethod`, `ParseStmt`, `ParseExpr`, `ParseUnary`,
`ParsePrimary`, `ParsePostfix`, `ParseCallArgs`, `ParseMatch`, …).

Adding a new statement (`guard`, for example):

1. Add the keyword token in the lexer.
2. In `ParseStmt`, dispatch on the keyword: `if (v == "guard") { return this.ParseGuard() }`.
3. Implement `ParseGuard()` — usually building a normal `IF_STMT`
   with a transformed condition (so the rest of the pipeline doesn't
   need to learn the new construct).

## Resolver (`src/resolver/`)

Two passes:

1. **CollectDecl** — registers every top-level type (class, enum) in
   the global scope so forward references work; builds the
   **MemberTable** mapping `ClassName.MemberName → typeName`.
2. **ResolveDecl** — walks the AST, opens/closes scopes for
   methods/blocks/for-in/match-arm, registers locals on declaration,
   reports `Unknown symbol 'x'` for unresolved identifiers.

Local scope is a flat array of names with a stack of start-indices
(`ScopeStarts`). `PushScope` records the current count, `PopScope`
truncates entries declared since.

The resolver also **owns the SourceMap** — that's what powers the
rustc-style snippets in error messages.

Adding a new builtin (e.g. a new `String_*` runtime helper):

1. Add the C declaration to the right header in `runtime/`.
2. In `src/resolver/resolver.am` — `RegisterBuiltins()` — declare it
   as a global with its return type:
   ```amalgame
   this.DeclareGlobal("String_DamerauLevenshtein", "int", false)
   ```
3. (Optional, but recommended) Add a return-type entry to
   `BuiltinCallReturnType()` and `InferTypeFromExpr()` in
   `src/generator/c_gen.am` so interpolation and type inference
   know about it.

## TypeChecker (`src/typechecker.am`)

- Maintains its own scope stack (`LocalNames`, `LocalTypes`,
  `ScopeStarts`) — the resolver pops its scopes after resolution, so
  the typechecker can't reuse them.
- For expressions, `CheckExpr` populates a `(node-key → type)` map
  via `SetType` / `GetType`.
- `CheckMemberExpr` looks up `obj.Field` types via
  `Symbols.GetMemberType(baseType, name)`.
- `CheckVarDecl` enforces assignability when a type annotation is
  present: `let n: int = "hello"` is a type error.

Errors carry their source snippet (loaded into `Sources: SourceMap`
by `main.am`), which is rendered by `TypeError.ToString()`.

## CGen (`src/generator/c_gen.am`)

The biggest single file (~2000 lines). Two-pass:

- **Pass 1** (`AddFilePass1`) — emits forward declarations: typedefs
  for classes (`typedef struct _Foo Foo;`), enum forward decls.
- **Pass 2** (`AddFilePass2`) — emits class struct bodies, method
  forward decls, method bodies. File order matters: dependents must
  come AFTER dependencies in the source list (because the bootstrap
  CGen emits decl + body interleaved per file).

Statement and expression emission split into many small functions
(`EmitStmt`, `EmitBlock`, `EmitExprStr`, `EmitMatch`, …). Use
`this.Out.EmitLine` / `Indent_` / `Dedent` to maintain indentation.

The `Emitter` has a `Streaming` flag — when set, `EmitLine` writes
directly to a file via `File.StreamLine` instead of accumulating in
a `List<string>`. Used by `gen_test.am`'s `gen6` to write
multi-MB `amc_lib.c` quickly.

Adding a feature in CGen:

1. Decide the AST shape — is it a new NodeKind, or a flag on an
   existing one (e.g. `?.` reuses MEMBER with `Flag = true`)?
2. Add a branch in `EmitStmt` / `EmitExprStr` for the new shape.
3. If the construct uses statements that the parent block won't see
   (e.g. a binder declaration that needs to be in scope for a guard),
   reach for GCC compound expressions: `({ stmt; expr; })`.

## Formatter (`src/formatter/formatter.am`)

`amc fmt file.am` re-emits source from the AST. The formatter walks
the same `AstNode` tree the rest of the compiler builds, so anything
expressible by the parser round-trips. Comments survive because the
lexer emits them as `COMMENT` tokens (rather than skipping them as
whitespace), and the parser collects them into `Parser.Comments`
without putting them in the AST. `Formatter.Sync(line)` re-injects
them at their original source line.

A few pieces collaborate to keep round-tripping idempotent:

- `Block.Str2` and `CLASS_DECL.Str2` carry the source line of the
  closing `}`, so the formatter knows where one block ends and the
  next thing begins (used to preserve blank-line gaps).
- `EmitInline` decides whether a body is a single statement or a
  block, mirroring the parser's flexibility.
- Expression contexts that don't have a source representation yet
  (see `EmitExpr` fallthrough) emit a `_TODO_<KIND>` placeholder so
  the result still parses; idempotence is preserved at the cost of
  meaning. This is meant to be temporary and is rare in practice.

`tests/run_fmt_tests.sh` checks idempotence + semantic equivalence
on a small fixture; the regression sweep that runs `amc fmt` on every
compiler source must stay green.

## Linter (`src/linter.am`)

`amc --lint file.am` runs a static-analysis pass on top of the
parsed AST and emits non-fatal warnings. The Linter shares no state
with the typechecker — it walks the AST top-down and collects
`LintWarning` records into `linter.Warnings`.

Coverage today:

- **Unreachable code** after `return` / `throw` / `break` /
  `continue`, including inside nested `if` / `while` / `for-in` /
  `try` bodies. *(since v0.3.3)*
- **Unused locals** — a `let` or `var` whose name is never read
  in the rest of the scope. Prefix the name with `_` to silence
  intentionally (mirrors the resolver's `_` wildcard treatment).
  *(since lint-extensions PR)*
- **Shadowed names** — a `let` / `var` / `for-in` binder reuses
  a name visible in an enclosing scope, including method params.
  *(since lint-extensions PR)*

The unused/shadow checks rely on a small in-linter scope stack
(parallel `LocalNames` / `ScopeStarts` arrays, same shape as the
resolver). Use marking is recorded into an append-only
`UsedNames` list; each local stores a `UsedNames.Count()`
snapshot at declaration so `PopScope` can answer "did this name
appear in `UsedNames` after I was declared?" without needing
`List<T>.Set`. `UsedNames` is reset to a fresh list at each
method entry to keep the snapshot indices meaningful per-method
and bound memory.

Method and lambda params are tracked in scope so they participate
in shadow detection, but they're never warned about as unused
(`_param` would be unergonomic).

The skeleton is set up to grow more checks (suspicious patterns,
implicit fallthrough in match, etc.) by extending `LintStmt` /
`LintExpr` without touching the rest of the pipeline. Warnings
always carry the per-program filename (populated from `prog.Str2`)
so multi-file invocations report the right paths.

The CLI flag (`--lint`) is wired in `main.am`, after the
typechecker pass and before code generation. Warnings don't bump
`ExitCode` — `amc --lint -o foo file.am` still produces output.

## Test runner (`amc test`)

`amc test [<dir>]` discovers `*_test.am` under `<dir>` (default `.`),
compiles + runs each, and aggregates `[PASS] <name>`,
`[FAIL] <name>: <msg>`, and `[SKIP] <name>` lines from each child's
stdout. The runner lives in `Program.RunTest` in `main.am`; the
subcommand is dispatched from `Program.Main` next to the `fmt`
subcommand.

Pipeline per test file:

0. **Pre-flight (once per run)** — resolve `amcRuntime` from
   `$AMC_RUNTIME`, else `<dirname(amc)>/runtime`, else fall back
   to `./runtime`. Then load `PackageRegistry` (manifest + lock)
   and, for every package declaring `[stdlib].sources`,
   `Program.PreCompilePackageSources` compiles each `.c` once
   with `gcc -O2 -I<amcRuntime> -w -c …` to a cached
   `/tmp/amc-pkg-<class>-<leaf>.o`. The `.o` paths feed into the
   per-test link step below.
1. **Discover** — shells out to `find <dir> -name '*_test.am' -type f`
   via `Process.RunCapture`. Cross-platform on POSIX and Windows
   MSYS2 (the CI's Windows path).
2. **Compile to C** — invokes the running `amc` binary on the file
   (path from `Args_Get(0)`) with `-o /tmp/amc_test_<idx>` and
   `--quiet`. Emits `<tmp>.c`.
3. **Compile to native** — `gcc -O2 -Iruntime -I'<amcRuntime>'
   <tmp>.c <pkg-objs…> -lgc -lm -lcurl -ldl -lpthread -o <tmp>`.
   The pre-compiled package `.o` files from step 0 are spliced in
   so vendoring backends (SQLite, future DuckDB) link cleanly with
   no user intervention. `-ldl` / `-lpthread` are unconditional —
   harmless when no package needs them, required by SQLite.
4. **Run + parse** — runs the test binary via `Process.RunCapture`
   and scans its stdout for tag-prefixed lines. Anything else is
   ignored. A non-zero exit with no tag lines is reported as
   `[FAIL] <crash> exit=N` so silent crashes still register.

The runner exits non-zero if any case FAILs or any file fails to
compile; otherwise zero. The convention deliberately stays
framework-free for v1 — a richer `Assert` module + `test_<name>`
auto-discovery is a possible v2.

## LSP server (`amc lsp`, `src/lsp.am`)

`amc lsp` runs a minimal LSP 3.x server speaking JSON-RPC 2.0
over stdio with the standard `Content-Length: N\r\n\r\n<N bytes>`
framing. v1 implements:

- `initialize` / `shutdown` / `exit` — lifecycle
- `textDocument/didOpen` / `didChange` / `didClose` — document
  state, advertised as Full sync (`textDocumentSync = 1`) so each
  `didChange` carries the entire updated text
- `textDocument/publishDiagnostics` — pushed back on every
  `did{Open,Change}`. Diagnostics merge resolver `RawErrors` and
  typechecker `Errors`, mapped from 1-based `(line, column)` to
  the 0-based LSP `Position` shape and underlined as a single
  character at the error column.

Hover, completion, and goto-definition are out of scope for v1
and will land in a follow-up.

JSON handling is **ad-hoc** rather than a real parser: the
`JsonStr(body, key)` and `JsonInt(body, key)` static helpers find
`"<key>"`, skip to the value, and read until the appropriate
terminator (handling backslash escapes for strings). LSP
messages don't have ambiguous keys at the depth we extract
(`method`, `id`, `uri`, `text`), so this trades correctness on
arbitrary JSON for ~50 lines of code instead of a full tagged
union + recursive-descent parser. If hover or completion need
deeper extraction, promote the codec to a proper `stdlib/Json`
module.

Two new runtime helpers shipped to support the framing:
`Console_ReadBytes(n)` reads exactly `n` bytes from stdin (the
LSP body after parsing `Content-Length`), and `Console_Flush()`
drains the stdout buffer so the client doesn't block waiting on
buffered replies.

The resolver gained a parallel `RawErrors: List<ResolverError>`
that's kept in lock-step with the formatted `Errors: List<string>`.
`ResolverError` is a tiny local mirror of `TypeError` — they
duplicate fields rather than share, because resolver.am is
compiled before typechecker.am in the bundle and we want the
source-file dependency graph to stay one-way.

## Snapshot bootstrap (`snapshot/`, `tools/save-snapshot.sh`)

`build_amc.sh` has a 2-rung bootstrap chain:

1. `./amc` — the freshly-built self-hosted compiler.
2. `./snapshot/amc` — last known-good amc, captured by
   `tools/save-snapshot.sh` after a green test run. The portable
   `snapshot/amc_lib.c` is committed; the binary is rebuilt by `gcc`
   on each platform that needs it.

The snapshot rung exists so we can introduce new syntax without
losing the bootstrap. If `./amc` breaks mid-development, `build_amc.sh`
falls through to `./snapshot/amc`, which still understands every
syntax shipped at the time of the last `tools/save-snapshot.sh` run.
From a clean clone, recompile `snapshot/amc` from the tracked
`snapshot/amc_lib.c` with one `gcc` invocation — see `snapshot/INFO.md`.

When introducing new syntax, take a snapshot *before* using the
new construct in `src/*.am`. That way, if the implementation has a
regression, the snapshot still works.

## main.am

Glue:

1. Parses CLI args (`Args.Count`, `Args.Get`).
2. Reads each input file, runs Lexer + Parser (Pass 1 of CGen).
3. Builds the `FullResolver`, feeds all programs, runs both passes.
4. Builds the `TypeChecker`, runs on the first program (multi-file
   typechecking is partial today).
5. Runs CGen Pass 2 over each program.
6. Emits the final `int main()` wrapper unless `--lib` or no
   `Program.Main` was found.

## gen_test.am

`src/generator/gen_test.am` is the "build the build" — when run, it
parses every `.am` file in the compiler, drives a single `CGen`
across all of them, and writes the result to `src/amc_lib.c`. This
is the canonical self-host artefact.

It also runs in **streaming** mode (`SetStreaming(true)`), bypassing
the in-memory line list and writing directly to `File_StreamLine`.

## Tests

`tests/samples/*.am` — input programs.
`tests/run_tests.sh` and `run_stdlib_tests.sh` — orchestrate them.
`tests/samples/lib_e2e_consumer.c` — the C consumer for the `--lib`
end-to-end test.

When you add a feature, drop a sample in `tests/samples/` and a line
in `tests/run_tests.sh` to grep for the expected output. Keep the
sample minimal — one feature, one observable.

## Common gotchas

- **Bootstrap circularity** — when you add a runtime helper or
  builtin, the running `amc` doesn't know about it until you rebuild.
  `build_amc.sh` tolerates a non-zero exit from step 1's `amc` so
  the pipeline can still produce a working binary at the next step.
- **File order in AMC_SOURCES** — see CGen Pass 2 above. If you see
  `error: implicit declaration of function 'Foo_Bar'` followed by
  `error: conflicting types`, swap the file order.
- **Generic types erase to `void*` at the C level** — boxing of
  primitives uses `(void*)(intptr_t)`. Since v0.3.3, the CGen
  tracks the elem type of `List<T>` / `Map<K,V>` for locals,
  parameters, return values, and explicit annotations; `xs.Get(i)`
  lowers with the right cast (`(int)AmalgameList_get(...)` etc.)
  so the result is typed at the call site without a manual cast.
  The underlying C representation hasn't changed.
- **Match arms can be statements OR expressions** — `1 => return "x"`
  and `1 => "x"` both parse. `let x = match y { ... }` also works
  since v0.3.0 — the codegen wraps it in a GCC compound statement
  expression. Algebraic-enum patterns and arm guards in expression
  position aren't supported yet, though.
- **Imports are informational** — the resolver's stdlib is global.
  Don't rely on `import` for visibility. Since v0.3.2, `amc fmt`
  preserves them on round-trip (parser stores each on `prog.Args`).

## Where to ask

- `ROADMAP_COMPLET.md` — what's planned and what's in flight.
- `CONTINUATION.md` — context dump for resuming a session.
- The git log of `feature/*` branches — every feature shipped has
  an explanatory commit message walking through the change.
