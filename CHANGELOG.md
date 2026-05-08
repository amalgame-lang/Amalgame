# Changelog

All notable changes to Amalgame are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) loosely.

For releases prior to v0.3.2, see the git log and `ROADMAP_COMPLET.md`.

---

## [v0.3.6] — 2026-05-08

The "lambdas in the wild" release. Higher-order `List<T>`
methods now take a lambda directly: `xs.Filter / .Map / .Reduce
/ .ForEach / .Any / .All / .CountIf`. Suite goes from 180/180
to **187/187** under `./amc`.

### Stdlib

- **Higher-order List methods** — Filter, Map, Reduce (with a
  `(acc, x) → acc` reducer dispatched through `Closure_call2`),
  ForEach, Any, All, CountIf. Each takes an `AmalgameClosure*`
  so the captured environment travels with the lambda.
  Implemented as static-inlines in `runtime/_runtime.h` and
  `runtime/Amalgame_Collections.h`. The closure struct +
  `_callN` wrappers are hoisted to the top of `_runtime.h` so
  the helpers a few lines down can use them; the
  `AmalgamePredicate` / `AmalgameAction` typedefs from v0.3.4
  are gone (no caller left). *(PR #133)*

### Compiler

- **CGen — closure-as-argument lowering** — `TryEmitListCall`
  recognises the seven new method names and lowers each via a
  new `EmitClosureArg` helper. The lambda is passed inline as a
  GCC compound-statement-expression that allocates the env,
  copies captured locals in, and yields a fresh
  `AmalgameClosure_new(...)`. `EmitLambdaCaptureCopy` is a tiny
  helper around the per-capture line so the self-host codegen
  sees `cap.Name` through a typed parameter (chained
  `.Get(i).Name` would erase to `void*` at the call boundary).
- **CGen — type inference for higher-order returns** —
  `InferTypeFromExpr` knows that a CALL whose member name is
  Filter / Map returns `AmalgameList*`, Any / All return
  `code_bool`, CountIf returns `i64`. So
  `let big = xs.Filter(...)` typechecks without an explicit
  annotation, and `EmitVarDecl` propagates the receiver's
  list-element type to the result so `big.Get(0)` lowers with
  the right cast. *(PR #133)*

### Documentation

- `docs/guide/02-language-tour.md` — Lambdas section gains a
  higher-order `Filter / Map / Reduce` example and a rewritten
  limitations note around the i64 boundary.
- `docs/guide/04-stdlib.md` — List section gains the table of
  higher-order methods.
- `docs/guide/05-runtime-and-interop.md` — new "Closures and
  higher-order calls" section with the AmalgameClosure struct,
  the three `_callN` wrappers, and the GCC compound-statement-
  expression idiom the CGen emits.
- `ROADMAP_COMPLET.md` — Lambda v2.5 split into a `[x]` partial
  (this release) + a `[ ]` non-int signatures pending. *(PR #134)*

### Known limitations (deferred to v2.5 final)

- Lambda arguments and results are still `(i64) → i64` at the
  C level. `xs.Map(x => x.Name)` over a `List<Class>` doesn't
  yet work — needs a TypeChecker layer that infers the lambda
  signature from the formal param at the call site, plus CGen
  to emit non-int `lam_N_fn` signatures.
- String interpolation `"x: {coll.Count()}"` doesn't yet
  propagate the inferred `AmalgameList*` to the embedded call.
  Workaround: stage in named locals before printing.
- ForEach mutating an enclosing `var` doesn't accumulate
  (closures capture by value). Reduce is the right tool.

## [v0.3.5] — 2026-05-08

The "developer experience" release. Three coordinated pieces:
**generic interfaces** with static contract verification,
**LSP hover + global completion** on top of the v0.3.4 diagnostics,
and **lambda v2** (multi-param + block body) on top of v0.3.4
single-param closures. Suite goes from **170/170 to 180/180**
under `./amc`.

### Language

- **Generic interfaces** — `interface IComparable<T> { Compare(T) -> int }`
  now type-parameterise. Classes declared `class Box implements
  IComparable<int>` go through a new `CheckImplementsContract`
  pass in the TypeChecker that walks every implemented interface,
  substitutes its generic params by the concrete args (recursing
  into nested generics like `List<T>` → `List<int>`, preserving
  trailing `?` nullability), and asserts every interface method
  exists on the class with the matching signature. Diagnostics
  point at the offending method/param with expected vs got
  types. Two new fields on `AstNode` (`Str3` for generic params
  CSV, `Str4` for the implements list CSV). No vtable / dynamic
  dispatch yet — duck-typed dispatch as before. *(PR #120)*

- **Lambda v2: multi-param + block body** — `(x, y) => x + y`
  and `x => { let d = x * 2; return d + 1 }` now parse and
  compile. Multi-param routes through new `AmalgameClosure_call2`
  / `_call3` runtime helpers; block bodies emit through `EmitBlock`
  with an `InLambdaBody` flag that boxes `RETURN_STMT` values as
  `void*`. The lambda AST node migrates from `Str=name` to
  `Params: List<PARAM>` to support N parameters. v1 closures
  (capturing single-param expression-bodied) keep their existing
  semantics. Lambdas in argument position with non-int signatures
  (e.g. `xs.Filter(x => x > 0)`) need a lambda-typing layer in
  the TypeChecker — that lands in v2.5. *(PR #129)*

### Tooling

- **`amc lsp` hover + global completion** — the LSP server
  shipped in v0.3.4 (diagnostics-only) now responds to
  `textDocument/hover` and `textDocument/completion` too. Hover
  walks the AST via a new `FindNodeAtPosition` helper to find
  the deepest named node covering the cursor, then returns its
  inferred type as Markdown via the new `TypeChecker.LookupNodeType`.
  Completion lists every global the resolver knows about
  (builtins + user classes / enums / functions), mapped to LSP
  `CompletionItemKind` (Class / Function / Variable). The VS
  Code client picks up the new capabilities automatically via
  the initialize handshake — no client changes. Member completion
  (`obj.<cursor>` narrowed to the receiver's type) and
  goto-definition / signature help remain follow-ups. *(PR #126)*

### Documentation

- **Stdlib expansion roadmap** — `ROADMAP_COMPLET.md` gains a
  "Core stdlib expansion" backlog entry listing eight modules
  everyday Amalgame code currently has to fake or shell out for:
  `DateTime`, `Json`, `Regex`, `Random`, `Encoding`, `Compress`,
  `Crypto`, `Threading`. Plus a new "Stdlib delivery model"
  open design question that lays out five alternatives to the
  current header-only-inline approach. *(PR #121)*

### Internal

- **Typed accessors on FullResolver** — `ProgramCount` /
  `ProgramAt` (used by the contract verifier), then `GlobalCount`
  / `GlobalNameAt` / `GlobalTypeAt` (used by the completion
  provider). Both batches work around the same self-host quirk
  where chained member access through a `List<T>` field emits
  `Programs_Count(...)` instead of `Programs->Count(...)`.
- **`-Wint-conversion` hotfix** — `let pn = ...ProgramCount()`
  in `FindInterface` lost its int type to `void*` erasure across
  the method call boundary. Pinned via explicit `let pn: int`,
  matching the precedent in `src/lsp.am` for `String_Length`
  results. Local builds had ignored the warning; CI on Linux
  and macOS treats it as an error. *(PR #124)*

## [v0.3.4] — 2026-05-08

The "tooling" release. Five new pieces ship together: capturing
closures in the language, two new lint checks, a `Process` stdlib
module, an `amc test` runner, and a minimal LSP server (`amc lsp`)
paired with a VS Code client. Suite goes from **150/150 to
170/170** under `./amc`.

### Language

- **Capturing closures** — `let f = x => x + n` now captures `n`
  (and any other enclosing local) by value at creation time, not
  by textual substitution. Replaces the v0.3.x macro path
  (`#define f(x) (n+x)`) which couldn't carry an environment.
  Pieces: a new `AmalgameClosure { fn, env }` runtime type with
  `Closure_new` / `Closure_call1` helpers; capture analysis in
  the resolver (free identifiers below `LambdaBoundary`); a
  per-lambda `LamEnv_N` struct + `lam_N_fn` top-level function
  emitted by a Phase A walk of CGen Pass 2; identifier-callable
  closures dispatched via `AmalgameClosure_call1` with arg/result
  boxing through `intptr_t`. v1 is single-param, expression-bodied,
  arity-1 `(i64) -> i64`. Multi-param, block bodies, lambdas in
  argument position, and non-int signatures are tracked for v2.
  *(PR #101)*

### Tooling

- **`amc --lint` extensions** — two new categories on top of the
  v0.3.3 unreachable-code check:
  - **Unused locals** (`let x = …` never read; prefix `_` to silence)
  - **Shadowed names** (a `let` reuses a name visible in an
    enclosing scope, including method params)

  The linter keeps its own scope stack with append-only
  `UsedNames` + per-local snapshot, sidestepping the missing
  `List<T>.Set` op. Method/lambda params participate in shadow
  detection but are never warned-on as unused. *(PR #104)*

- **`Process` stdlib module** — `Process.Run(cmd) -> int` and
  `Process.RunCapture(cmd) -> AmalgameProcessResult { Exit,
  Stdout, Stderr }`. Cross-platform via `popen`/`_popen`; exit
  codes decoded with `WEXITSTATUS` / `WTERMSIG` (signals
  surface as `128 + signum`, shell convention). Stderr is
  currently merged into Stdout via shell `2>&1`; v2 will split
  them with a real pipe pair. *(PR #106)*

- **`amc test [<dir>]`** — discovers `*_test.am` files under
  `<dir>` (default `.`), compiles + runs each via `Process`, and
  aggregates `[PASS] <name>` / `[FAIL] <name>: <msg>` /
  `[SKIP] <name>` lines from each child's stdout. Crash-with-no-tags
  shows up as `[FAIL] <crash> exit=N`. Tests stay framework-free
  (just normal `Main`s that print tag lines). Exits non-zero on
  any FAIL or compile error. *(PR #109)*

- **`amc lsp` server + VS Code client** — minimal LSP 3.x server
  speaking JSON-RPC 2.0 over stdio. v1 implements `initialize` /
  `shutdown` / `exit`, `textDocument/didOpen` / `didChange` /
  `didClose` (Full sync), and `textDocument/publishDiagnostics`
  (push on every did{Open,Change}). Diagnostics merge resolver
  and typechecker errors and underline the whole token at the
  error column. JSON handling is ad-hoc — `JsonStr` / `JsonInt`
  helpers find `"<key>"` and read the value, no real parser.
  Hover and completion are out of scope for this release.

  Editor side: `editors/vscode/` (now version 0.2.0) gains an
  `extension.js` that spawns `amc lsp` via `vscode-languageclient`.
  Configurable via `amalgame.serverPath` (default `amc`) and
  `amalgame.enableLsp` settings. `npm install` is a one-time
  prerequisite. *(PR #110)*

### Compiler

- **Lexer-bootstrap CR escape fix** — the bootstrapping amc binary
  lexed `"\r"` as the two chars `\` + `r` instead of a single CR
  (0x0d) byte, which made `EscapeStringForC`'s `String_Replace(s,
  "\r", "\\r")` a no-op and propagated through `lsp.am`'s framing,
  causing the LSP wire to emit literal `\r\n` (5c 72 0a) instead
  of CR LF (0d 0a). VS Code's LanguageClient never matched the
  framing and stayed in Starting forever. Fixed by routing CR
  through `String_FromByte(13)`, which is just an int→byte builtin
  and has no escape-table dependency; same idea for `\x1b` (ESC).
  *(PR #110)*

- **CGen return-type table extended** — `Console_{Read,Flush,Write}`,
  `Process_{Run,RunCapture}`, and the `String_*` int/bool returns
  (`Length`, `IndexOf`, `LastIndexOf`, `ToInt`, `StartsWith`,
  `EndsWith`, `Contains`, `IsEmpty`) gain explicit entries. Fixes
  a pre-existing latent bug: an unannotated
  `let n = String_IndexOf(...)` defaulted to `code_string`, breaking
  arithmetic on the result. *(PR #110)*

- **Resolver structured errors** — new
  `RawErrors: List<ResolverError>` kept in lock-step with the
  formatted `Errors: List<string>`. Exposes `(file, line, col, msg)`
  for downstream tools (the new LSP, future IDE features). The
  formatted-string CLI path is unchanged. *(PR #110)*

### Runtime

- **`Console_ReadBytes(n)`** — reads exactly `n` bytes from stdin.
  Used by the LSP framing reader after parsing `Content-Length`.
- **`Console_Flush()`** — drains stdout. Required by the LSP
  server so the client doesn't block on buffered replies.
- **`AmalgameClosure { fn, env }`** + `AmalgameClosure_new` /
  `AmalgameClosure_call1` — backbone of the new closure feature.
- **`AmalgameProcessResult { Exit, Stdout, Stderr }`** + the
  `Process_Run` / `Process_RunCapture` helpers.

### Tests / infra

- Suite is now **170/170** (108 core + 50 stdlib + 12 fmt) under
  `./amc` — +20 vs v0.3.3. New samples: `tests/samples/closures_capture.am`,
  `lint_unused_shadow.am`, `process_api.am`,
  `test_runner/{arith,mixed}_test.am`. New helpers in
  `run_tests.sh`: `run_amc_test_check`, `run_lsp_check`.

### Docs

- `docs/guide/07-internals.md` gains sections on the **test runner**
  (`amc test`) and the **LSP server** (`amc lsp`, `src/lsp.am`).
  The Linter section is rewritten to cover the new unused / shadow
  checks.
- `docs/language/grammar.{ebnf,md}` — full rewrite. The previous
  files dated from v0.1.0 (back when the language was called *CODE*)
  and described constructs the parser never implemented while
  missing everything added since (try/catch, match-as-expression,
  closures, list comp, null-safety, decorators, generics, …). The
  new EBNF mirrors `src/parser/parser.am` exactly.
- README adds bullets for `--lint`, `amc test`, and `amc lsp`.

---

## [v0.3.3] — 2026-05-08

Quality-of-life follow-up to v0.3.2. Cleans up cosmetic typechecker
noise during multi-file compilation, ships a first `amc --lint`
flag (basic static analysis), and stops the workflow from leaking
a stale v0.3.0 artefact into every release. Suite goes from
149/149 to **150/150**.

### Compiler

- **Multi-file false-positive cleanup** — `build_amc.sh` step 1
  invokes `amc` on ten compiler sources at once, and the
  typechecker was emitting ~93 spurious errors on that path
  (build still went through, but the noise drowned out anything
  real). Three small fixes in `CheckProgram` / `CheckReturn`:
  the per-program filename is now read from `prog.Str2` rather
  than the constructor argument; `ExprType*`, `LocalNames`/
  `Types` and `ScopeStarts` reset between programs; and bare
  `return` (parsed as `Return(_unknown_, …)`) is recognised as
  such instead of being treated as a value-return.
  `build_amc.sh` is now silent.

### Tooling

- **`amc --lint`** — new flag that runs a static-analysis pass on
  top of the parsed AST and emits non-fatal warnings. MVP catches
  unreachable code after `return` / `throw` / `break` / `continue`,
  including inside nested `if` / `while` / `for-in` / `try` bodies.
  The skeleton (`src/linter.am`) is set up to grow more checks
  (unused locals, shadowed names, …) without touching anywhere
  else.

### CI

- **Drop stale v0.3.0 artefact, stop tracking `dist/`** — a
  `dist/amc-0.3.0-linux-x86_64.tar.gz` had been committed during
  v0.3.0 and was being uploaded on top of every later release by
  the workflow's `dist/*.tar.gz` glob. Removed, and `dist/` is now
  in `.gitignore` so future stagings don't sneak back in.

### Tests / infra

- Suite is now **150/150** (88 core + 50 stdlib + 12 fmt) under
  `./amc`. New samples: `tests/samples/lint_test.am`. New helper:
  `run_lint_check` in `run_tests.sh`.

---

## [v0.3.2] — 2026-05-08

Closes every open compiler bug tracked in v0.3.1's `SKIP_SELFHOST`
list, restores `try` / `catch` / `throw` / `finally` end-to-end in
the self-hosted parser, and ships the first round of generic type
inference for `List<T>` and `Map<K,V>`. The full test suite is
**149/149 green with zero SKIPs**.

### Language

- **`try` / `catch` / `throw` / `finally`** restored end-to-end in the
  self-hosted compiler (the Vala bootstrap had it; the rewrite hadn't
  grown a `ParseTry` yet). Lowers via `setjmp`/`longjmp` using the
  existing `_am_throw` runtime helpers.
- **`Type.Variant` patterns in `match`** — `Direction.North => …` is
  now recognised as a pattern. `ParseMatchPattern` reads an optional
  `.IDENT` suffix.
- **Generic type inference (locals + params + returns)**:
  `let xs = new List<int>()`, `xs: List<int>`, `xs = MakeList()`,
  and `Foo(List<int> xs)` all carry the elem type, so `xs.Get(i)`
  lowers to `(int)AmalgameList_get(xs, i)` without a manual cast.
  Same for `Map<K,V>.Get(k)` (which wasn't even dispatched before).
- **`obj.Method()` instance syntax for strings** — `s.Length()`,
  `"foo".Trim()`, `s.Replace(a, b)`, … route through `String_*`.

### Compiler

- **Multi-file type checking** — TypeChecker now walks every parsed
  program (was hard-coded to `programs[0]`).
- **Null-safety** — three independent bugs fixed: NodeKey collision
  in the typechecker (Kind is now part of the key), bare `?` was
  dropped by the lexer (added `OP_QMARK`), and `TypeToC("T?")` no
  longer doubles up the C pointer.

### Formatter

- **Same-line comments** preserved on round-trip: `let x = 1  // foo`
  no longer sees its comment bumped to the next line.
- **`import` directives** preserved: the parser used to discard them.
- `try` / `catch` / `throw` round-trip lossless.

### Tests / infra

- Suite is now **149/149** (87 core + 50 stdlib + 12 fmt) under
  `./amc`. **`SKIP_SELFHOST` is empty.**
- `run_multifile_test` shell helper fixed (was passing `-o foo.c`,
  amc dutifully expanded it to `foo.c.c` and never produced an
  executable).
- `tests/run_all_tests.sh` completes end-to-end for the first time
  (its `set -e` no longer trips on a half-failing suite).

[v0.3.6]: https://github.com/BastienMOUGET/Amalgame/releases/tag/v0.3.6
[v0.3.5]: https://github.com/BastienMOUGET/Amalgame/releases/tag/v0.3.5
[v0.3.4]: https://github.com/BastienMOUGET/Amalgame/releases/tag/v0.3.4
[v0.3.3]: https://github.com/BastienMOUGET/Amalgame/releases/tag/v0.3.3
[v0.3.2]: https://github.com/BastienMOUGET/Amalgame/releases/tag/v0.3.2
