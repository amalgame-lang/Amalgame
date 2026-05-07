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
- **Generic types erase to `void*`** — boxing of primitives uses
  `(void*)(intptr_t)`. The CGen emits this automatically for
  collection element types.
- **Match arms are statements** — the language doesn't have
  match-as-expression. Don't try to write
  `let x = match y { ... }` — use early-returns or assign in arms.
- **Imports are informational** — the resolver's stdlib is global.
  Don't rely on `import` for visibility.

## Where to ask

- `ROADMAP_COMPLET.md` — what's planned and what's in flight.
- `CONTINUATION.md` — context dump for resuming a session.
- `docs/transpiler/*.md` — older design docs (somewhat stale).
- The git log of `feature/*` branches — every feature shipped has
  an explanatory commit message walking through the change.
