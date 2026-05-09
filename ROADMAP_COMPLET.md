# Amalgame — Roadmap

> Updated 2026-05-09 · `amc 0.4.3` · self-hosted · 307/307 tests · multi-OS CI · GitHub Releases automation

This document is the canonical "what's done, what's next" board.
For architecture and contribution guidance see
[docs/guide/](docs/guide/README.md).

---

## ✅ Shipped

### Self-hosted compiler
The compiler is written in Amalgame in [src/](src/) and compiles
itself in ~5 seconds (`./build_amc.sh`). A 3-rung bootstrap chain
keeps recovery easy:

1. **`./amc`** — current self-hosted compiler.
2. **`./snapshot/amc`** — last known-good amc, captured by
   `tools/save-snapshot.sh` after a green test run. The portable
   `snapshot/amc_lib.c` is committed; the binary regenerates with one
   `gcc`. Used as fallback whenever `./amc` is broken mid-development.
3. **`./build/amc`** — Vala bootstrap in `archive/vala-bootstrap/`,
   no longer exercised by CI but kept locally for cold-start recovery
   (`./compile.sh`).

| Component        | File                                |
|------------------|-------------------------------------|
| Lexer            | `src/lexer/{token,lexer}.am`        |
| Parser           | `src/parser/{ast,parser}.am`        |
| Resolver         | `src/resolver/{symbol,resolver}.am` |
| TypeChecker      | `src/typechecker.am`                |
| CGen             | `src/generator/c_gen.am`            |
| Formatter        | `src/formatter/formatter.am`        |
| Diagnostics      | `src/diagnostics.am`                |
| CLI entry        | `src/main.am`                       |
| Generated bundle | `src/amc_lib.c` (~9 300 lines)      |
| Runtime (C)      | `runtime/*.h`                       |

### Language features
- Variables: `let` / `var` with optional type annotation
- Primitives: int / float / double / bool / string / void
- Classes, inheritance (single), interfaces (basic)
- Data classes, records (params auto-assign to fields since v0.3.1)
- Enums (simple) + algebraic enums (tagged unions) with destructuring
- Generics (erased to `void*` at C level)
- Null-safety: `T?` types, `??` coalescing, `?.` safe access (field + method)
- Tuples + destructuring `let (a, b) = f()`
- String interpolation `"hello {x}"` (with method calls inside)
- Triple-quoted multiline strings `"""…"""`
- `\xHH` and `\uHHHH` escape sequences
- Bitwise ops, compound assigns, pipeline `|>`, range `0..n`
- Pattern matching with **arm guards** (`n if n > 0 => …`), ranges, binders
- **List comprehensions** `[x*2 for x in xs if x > 0]` (v0.3.0)
- **`match` as expression** `let x = match y { … }` (v0.3.0)
- Match arm bodies can be statements (`return`, `break`, `continue`, blocks)
- `for x in <list>` (collection iteration, v0.3.0)
- Guard clauses: `guard cond else { return }`
- Decorators: `@inline`, `@deprecated` → C attributes
- Named arguments (documentation-only at call site)
- **Capturing closures** (since v0.3.4) — single-param expression-bodied
  lambdas snapshot enclosing locals into a heap-allocated env struct;
  callable as values, dispatched through `AmalgameClosure_call1`.
- try / catch / throw / finally — Vala bootstrap only;
  not yet implemented in self-hosted parser

### Tooling
- **`amc fmt`** — formatter that re-emits the AST canonically with
  comment preservation. Idempotent on every compiler source.
- **`amc lsp`** — workspace-aware LSP server (since v0.3.6 / PR #146):
  scans every `.am` file under the detected workspace root so
  cross-file types resolve in any open file. Diagnostics + hover +
  global completion.
- **`amc migrate <file|dir>`** — LLM-driven source-to-Amalgame
  migration (v0.4.0). 21 source extensions auto-detected. Provider
  abstraction: `claude` (CLI), `claude-api` (Anthropic HTTP),
  `chatgpt` (OpenAI), `gemini` (Google), `custom` (script). Auto-
  selection by env var. Directory recursion. Result cache by
  SHA-256(source + system prompt). Cost estimation in `--dry-run`.
- **`amc generate "<prompt>"`** — LLM-driven prose-to-Amalgame
  (v0.4.0). Same provider stack, plus `--stream` via the claude
  CLI for direct stdout passthrough.
- **`amc explain <file.am>`** — LLM-driven Amalgame-to-prose
  (v0.4.0). `--lang` flag for non-English explanations.
- **VS Code syntax highlighting + LSP client** (`editors/vscode/`)

### Stdlib
Console, File, Path, Math, String, List/Map/Set, Http (with
`PostWithHeaders` return-type tracked since v0.4.0), TcpServer/TcpConn,
TcpClient, UdpSocket, Args, Exit, Process (Run + RunCapture),
Env (Get + Has, since v0.4.0).
Documented in [docs/guide/04-stdlib.md](docs/guide/04-stdlib.md).

### Compiler quality
- Rustc-style diagnostics with source snippet + caret (Resolver + TypeChecker)
- Multi-OS CI (Linux + macOS + Windows MSYS2). Linux runs on the
  self-hosted amc directly — Vala is no longer in the CI dependency
  graph.
- Tag-driven Release workflow (Linux .tar.gz + macOS .tar.gz + Windows
  .zip with bundled MinGW DLLs)
- **263/263 tests** in CI under `./amc` (201 core + 50 stdlib + 12
  fmt), zero SKIP. Suite grew significantly with the v0.4.0 LLM
  features (most additions are hermetic — no real LLM calls).
- **Lambda v2.5** (PR #142) — non-int signatures unlocked.
  `xs.Map(x => x.Name)` over `List<Class>` works.
- **Multi-line method chains** (PR #154) — fluent / LINQ-style code
  parses cleanly across newlines.
- **CGen auto-qualify** (PR #155) — `Id = id` in a class method
  lowers to `self->Id = id`. Lets users from C# / TS / Kotlin skip
  the explicit `this.` qualifier.
- **TS-style param syntax** (PR #152) — `Foo(id: int)` accepted as
  alias for `Foo(int id)`. Also fixed an infinite-loop regression
  on the same syntax.

---

## 🟡 Language — backlog

In rough order of usefulness × effort:

- [x] **Capturing closures** (v0.3.4) — single-param expression-bodied
      lambdas (`x => expr`) now capture enclosing locals by value at
      creation time. Resolver computes the free identifiers below
      `LambdaBoundary`; CGen emits a `LamEnv_N` struct + `lam_N_fn`
      top-level fn per lambda; runtime ships `AmalgameClosure { fn,
      env }` with `Closure_new` / `Closure_call1`. v1 is arity-1
      `(i64) -> i64`. Multi-param, block bodies, lambdas in argument
      position, and non-int signatures are tracked for v2.
- [x] **`obj.Method()` instance syntax for strings** — `s.Length()`,
      `"foo".Trim()`, `s.Replace(a, b)` etc. now lower to
      `String_Method(receiver, args)`. `EmitCalleeStr` maps
      `code_string`-typed receivers to the `String_` runtime prefix
      (the bare type is the legacy C typedef from when the language
      was called "code"); the CALL emit branch treats both
      `IDENTIFIER` and `LITERAL_STRING` receivers as `isSelfCall` so
      the receiver is passed as the first argument.
- [x] **Generic type inference** (locals + params + returns) —
      `ParseNew` captures the type-arg list on `NEW_EXPR.Str2`. CGen
      threads the elem (List<T>) / value (Map<K,V>) type through three
      sources at every `let`: (1) the explicit annotation on the var,
      (2) the NEW_EXPR RHS, and (3) the raw return type of the callee
      (looked up via `MethodRetRawSet`/`Get`). Method params do the
      same on entry, so `xs.Get(i)` / `m.Get(k)` in any reasonable
      shape lowers to `(T)AmalgameList_get(...)` / `(V)AmalgameMap_get(...)`
      with the correct cast. The TypeChecker's `IsAssignable` now
      treats `List` ↔ `List<int>` (and similar generic erasures) as
      compatible so a `return xs` from a `List<int>`-returning method
      type-checks even when the local is recorded as the bare `List`.
- [x] **Generic interfaces** (v0.3.5) — `interface IComparable<T>`
      now type-parameterise. `class Box implements IComparable<int>`
      goes through a new `CheckImplementsContract` pass that
      substitutes the interface's generic params by the concrete
      args (recursing into nested generics, preserving `?` markers)
      and asserts every method exists on the class with the
      substituted signature. Static contract check only — no
      vtable / dynamic dispatch.
- [x] **Lambda v2 — multi-param + block body** (v0.3.5) —
      `(x, y) => x + y`, `x => { let d = x*2; return d+1 }`,
      `(a, b, c) => a + b + c`. Routes through new `Closure_call2`
      / `_call3` runtime helpers; block bodies emit through
      `EmitBlock` with an `InLambdaBody` flag that boxes returns.
      Lambdas in argument position with non-int signatures
      (e.g. `xs.Filter(x => x > 0)`) need a lambda-typing layer
      in the TypeChecker — that lands in v2.5.
- [x] **Lambda v2.5 (partial) — higher-order List methods** —
      `xs.Filter / .Map / .Reduce / .ForEach / .Any / .All /
      .CountIf` all take a lambda. Lowered to runtime helpers
      that dispatch through `AmalgameClosure_callN`, so captured
      env follows. CGen learned an `EmitClosureArg` helper that
      emits a GCC compound-statement-expression at the call site
      (env alloc + capture copies + `Closure_new(...)`).
      InferTypeFromExpr returns `AmalgameList*` for Filter/Map
      so `let big = xs.Filter(...)` typechecks without an
      explicit annotation, and the receiver's element type is
      propagated to the result (so `big.Get(0)` lowers with the
      right cast).
- [ ] **Lambda v2.5 — non-int signatures (still pending)** —
      `xs.Map(x => x.Name)` over a `List<Class>` doesn't yet
      work because the lambda is still `(i64) → i64` at the C
      level. Needs (1) the TypeChecker to infer the lambda's
      expected signature from the formal parameter at the call
      site, (2) CGen to emit non-int `lam_N_fn` signatures
      based on the inferred shape, (3) string interpolation
      `"x: {coll.Count()}"` to propagate the inferred
      `AmalgameList*` to the embedded call (current workaround:
      stage in named locals).
- [ ] **Spread operator** `f(...args)` and `[...a, ...b]`. Needs list
      literal syntax `[...]` first and a clear semantics for variadic
      calls. Larger than it looks.
- [ ] **`async` / `await`** — coroutines via ucontext or setjmp.
      Substantial: runtime + AST + CGen. Defer until there's a concrete
      use case.

---

## 🟠 Compiler — open bugs (samples currently SKIPped)

These samples pass under the Vala bootstrap but trigger bugs in the
self-hosted compiler. They're marked SKIP in `tests/run_tests.sh`
(`SKIP_SELFHOST`) so the suite stays green; each one needs its own
fix.

- [x] **`Type.Variant` patterns in match** — `ParseMatchPattern`
      now reads an optional `.IDENT` suffix and emits an `Ast.Member`,
      which the existing CGen MEMBER branch lowers to `Type_Variant`.
      Unblocks `enums.am`.
- [x] **try / catch / throw** — `ParseTry` and `ParseThrow` build
      `TRY_STMT { Body=try, Else=catch, Cond=finally?, Name=binder }`
      and `THROW_STMT { Left=expr }`. Resolver/typechecker open a
      `catch` scope and declare the binder as `void*`. CGen emits the
      same `setjmp`/`longjmp` pattern the Vala bootstrap used (saves
      `_am_ex.env` into a `_am_prev_env_<line>` slot, restores it
      after the handler) and lowers `throw new T(args)` to
      `_am_throw((void*)(T_new(args)), "T", first_string_arg)`. The
      formatter learned `EmitTry` / `EmitThrow` so round-trip stays
      lossless. Drops `try_catch.am` from `SKIP_SELFHOST`.
- [x] **null safety / null-safe member typing** — three independent
      bugs fixed: (1) `NodeKey` in the typechecker hashed nodes by
      `line:col:name:str`, so two unrelated nodes with the same
      coordinates shared a type slot — `Kind` is now part of the key;
      (2) the lexer dropped a bare `?` token (only `??` and `?.` were
      recognised), so `Box?` parsed as `Box`; an `OP_QMARK` token was
      added; (3) `TypeToC` lowered `T?` to `Type**`, so any
      `Body: AstNode?` field generated incompatible `**`-pointer
      shapes — `T?` now lowers to `TypeToC(T)`, leaving the C pointer
      unchanged. Drops `null_safety.am` and `null_safe_member.am`
      from `SKIP_SELFHOST`. **`SKIP_SELFHOST` is now empty.**

### Compiler — polish

- [x] **Multi-file type checking** — `main.am` now loops over every
      parsed program and runs `tc.Check` on each, sharing a single
      TypeChecker instance so cross-file uses get type-checked too.
      Also fixed the `run_multifile_test` shell helper, which passed
      `-o foo.c` (which amc dutifully expanded to `foo.c.c`) and
      expected an executable amc never produces — now follows the
      single-file convention (`-o base` + manual gcc), so the four
      multifile tests pass.
- [ ] **Better error recovery** — the parser is okay but produces
      `_unknown_` placeholder ASTs that cascade into noisy resolver
      errors. Skip them more aggressively.
- [x] **Comments-on-same-line in `amc fmt`** — `Sync` and
      `FlushTrailingComments` now drain pending comments whose source
      line == `LastLine` by appending them to the previously emitted
      output line, so `let x = 1  // foo` and `if (x) {  // foo`
      round-trip on their original line. `EmitBlockStmts`/`EmitInline`
      drain once more before bumping `LastLine` to the closing brace,
      catching trailing comments on the *last* stmt of a block.
- [x] **Imports preserved by `amc fmt`** — parser stores each
      `import X.Y` as an `Ast.Ident` on `prog.Args` (a slot the
      Program node didn't otherwise use), and `EmitProgram` re-emits
      them between `namespace` and the first top-level decl.
- [ ] **`while(ptr != null)` GC issue** — existing workaround uses
      `for i in 0..N`. Investigate whether it's a real GC bug or
      just a CGen mis-detection.

### Compiler — internal refactoring & optimization

These are quality-of-life improvements *to the compiler source
itself*, not to the code it emits. The compiler grew quickly
during v0.3 → v0.4 and now has technical debt worth paying down
before the next big language addition.

- [ ] **`if`/`else if` chains → `match` expressions** — the
      compiler dispatches on `NodeKind` / token values via long
      chains of `if (k == NodeKind.X) { ... } else if (k == NodeKind.Y)`
      almost everywhere (`EmitStmt`, `EmitExprStr`, `ResolveExpr`,
      `ResolveStmt`, `ParseDecl`, `ParseStmt`, `InferTypeFromExpr`).
      Match expressions would be ~30% shorter, easier to read, and
      catch missing cases at typecheck time. Blocked on:
        - Algebraic-enum patterns in expression position (currently
          partial — see `### Compiler — open bugs` above).
        - Match-arm guards on enum patterns (also partial).
      So the language work is partly already on the roadmap;
      this refactor is the prize once it lands.
- [ ] **Extract repeated CGen helpers** — `EmitInterpolatedString`,
      `EmitMatchExpr`, `EmitOneLambdaBody`, `EmitClosureArg`, and
      `TryEmitListCall` all duplicate variants of the
      `(void*)(intptr_t)X` boxing dance and the symmetric unbox.
      Extract a typed `BoxScalar(expr, ctype)` / `UnboxScalar(expr,
      ctype)` pair so the call-sites read as intent rather than
      ceremony.
- [ ] **Reduce `void*` erasure across method-call boundaries** —
      every other contributor PR hits this and works around it with
      `let x: T = chain.Get(i)` or by extracting a typed helper.
      Either fix the inference (CGen knows the return type, just
      forgets to propagate it across the boundary) or codify the
      workaround patterns in `docs/guide/07-internals.md` so it's
      not a paper cut every time.
- [ ] **CGen: chained `obj.Field.Method()` / `obj.Method().Method()`
      lowers as a name-mash.** Repro:
      `o.Field.Get()` emits `o->Field_Get()` (should be
      `Inner_Get(o->Field)`); `o.GetInner().Get()` emits
      `App_Outer_GetInner(o)_Get()` (should be
      `Inner_Get(App_Outer_GetInner(o))`). Root cause:
      `EmitCalleeStr` (`src/generator/c_gen.am:3003-3004`)
      fallback returns `target + "_" + mname` where `target` is
      already a complete C expression — concatenating a `_method`
      suffix gives an invalid identifier. Fix is non-trivial:
      callers (mostly `EmitCallExpr`) assume the callee is a
      bare function name they can call as `<name>(args)` —
      changing the contract to "if the receiver is a chain,
      route through `Type_Method(<expr>, args)`" needs a typed
      lookup of `callee.Left`'s return type and a small refactor
      of every call site that consumes `EmitCalleeStr`.
      Workaround: extract intermediate locals
      (`let mid: T = obj.Field; mid.Method()`). Already applied
      in `src/lsp.am`, `src/migrate.am`, and the JSON test
      sample; comments inline cite this item.
- [ ] **Parser: `expr >> N` inside a `let` drops the shift.**
      Surfaced while writing `Amalgame.Random` (2026-05-09). Repro:
      `let x: int = r >> 8` lowers to `i64 x = r;` — the shift
      operator is silently dropped. The mask form of the same
      expression (`expr & N`) lowers correctly. Likely a precedence
      or look-ahead bug where `>>` collides with the closing
      `>` of a generic parameter (`List<List<int>>` style).
      Workarounds: replace with division by a power of two
      (`let x: int = r / 256`) — used by `Random.Bytes` and
      `Random.Float`. Found cases dropping `& 255` *outside* a
      surrounding `(expr >> N) & 255`, lowering it as a stray
      statement-level `_unknown_ & 255;`. Same call extracted to
      named locals fixes both. Next step: build a minimal repro
      file and grep the parser/cgen for the lowering of
      `BinaryExpr(>>, …)` inside `LetStmt` initializers.
- [ ] **Parser: top-level free functions in a stdlib namespace
      don't emit a definition.** `public List<int> SystemBytes(n)`
      at file scope (no enclosing class) parses without error but
      cgen emits a bare `SystemBytes(...)` call site without ever
      defining the function. Workaround: hang the function on a
      class as a `public static` method (matches the existing
      facade pattern in `Amalgame.Json`). Worth supporting because
      free helpers are a natural fit for utility modules.
- [ ] **Snapshot size** — `snapshot/amc_lib.c` is ~12 500 lines,
      tracked in git for the bootstrap chain. Each compiler PR
      regenerates it and the diff dominates the review noise. Two
      mitigations:
      (a) shrink the C output (the per-method `__attribute__((unused))`
          dance + redundant `(void)` casts add ~20%);
      (b) `.gitattributes` `merge=ours` on `snapshot/amc_lib.c` so
          merge conflicts auto-resolve (we always rebuild the
          snapshot post-merge anyway — see PR #146 / #149 / #155
          conflict resolutions).
- [ ] **Profile compile time** — `./build_amc.sh` is ~5 s end-to-end,
      and the largest cost is gen_test re-parsing every source on
      every invocation. A serializable AST cache (file mtime →
      pickled AST under `.amc-cache/`) could probably halve that.
      Modest win but unblocks faster CI loops.
- [ ] **Linter coverage** — `amc --lint` flags unreachable code,
      unused locals, shadowed names. Easy adds the framework already
      supports: catch-binder unused, suspicious match (missing default
      arm + non-exhaustive enum), implicit fallthrough, dead `import`,
      `let` declared but never assigned past initialization.
- [ ] **Reduce duplication in arg parsing across subcommands** —
      `migrate.am`, `generate.am`, `explain.am`, and
      `main.am::RunFmt`/`RunTest` each reimplement an args loop.
      A shared `ArgParser` class with a fluent registration API
      would cut ~150 lines and centralize the `--help` rendering.
- [x] **Promote ad-hoc JSON to a real `Amalgame.Json` module**
      (resolved). Phase 1 (module + 24 tests, v0.4.2). Phase 2
      (swap `lsp.am` request dispatcher to `Json.Parse`, swap
      Anthropic / ChatGPT / Gemini response extractors in
      `migrate.am`, swap all `EscapeJsonStr` / `JsonEscape` to
      `Json.EscapeString`). Phase 3 (delete the six dead helpers
      in `lsp.am` and `migrate.am`). The earlier-noted
      "Json.Parse hangs on 16 KB bodies" turned out to be a
      benchmark artefact: a bash probe that used `${#body}` to
      compute Content-Length under-counted UTF-8 multibyte chars
      and shipped a truncated frame. Real client traffic uses
      byte-accurate counts and the parser handles a typical 16 KB
      didOpen body in ~37 ms.
- [x] **Tighten the parser's error-recovery path** — audit done
      after PR #152. `ParseClassBody`, `ParseBlock`, `ParseCallArgs`,
      `ParseEnumBody`, `ParseMethod`-params, `ParseInterface`-params
      all have the `lastPos / Pos == lastPos → Advance` safety belt.
      The remaining loops (logical / arithmetic operator climbing,
      `ParseTypeName` chain, `ParseQualifiedName`) only call helpers
      that always consume at least one token, so they're not at risk.
      Pattern documented in `docs/guide/07-internals.md`'s "Adding a
      new statement" recipe — any new while-loop that calls a sub-
      parser must include a position-watchdog.
- [x] **LSP false positives on enum types** (resolved) — verified
      empirically on 2026-05-09: opening `src/lexer/lexer.am`
      through `amc lsp` now produces 0 "Unknown symbol"
      diagnostics (was ~70+). `FullResolver.CollectDecl`
      (`src/resolver/resolver.am:516-525`) handles `ENUM_DECL` the
      same way as `CLASS_DECL`: declares the enum name as a global,
      then walks members via `CollectEnumMembers` to register
      qualified names (`TokenType_KW_LET`) too. Cross-file enum
      references resolve through the workspace scan (PR #146).
      Note: a separate typechecker bug — spurious "Return type
      mismatch: expected 'TokenType', got '?'" on enum-member
      returns inside `if (word == "lit") { return TokenType.X }`
      bodies (37 cases on `lexer.am` via the LSP) — is tracked
      as an open compiler internal item below.

- [ ] **Typechecker: spurious return-type mismatch in IF-body
      RETURN of enum members.** Investigation (2026-05-09): the
      reported `got` type is variable across the 37 `lexer.am`
      sites — sometimes `void`, sometimes `string`, sometimes
      `int`. For
      `if (word == "if") { return TokenType.KW_IF }` the LSP
      reports `got 'string'`, which matches the type of the
      `"if"` LITERAL_STRING in the surrounding condition. That
      strongly suggests `CheckReturn` is reading the wrong
      `stmt.Left` when the RETURN is nested inside an IF body —
      either a parse-tree shape mismatch on single-stmt
      `{ return ... }` blocks, or a NodeKey collision in the
      `ExprType` map (the SetType/GetType pair desyncs Keys vs
      Vals on update — see typechecker.am:162). Doesn't
      reproduce on minimal cross-file repros (a 2-file
      `enum_def.am` + `enum_use.am` passes both `--check` and
      the LSP probe with 0 diagnostics). Next step: instrument
      `CheckReturn` to log `(stmt.Line, stmt.Left.Kind,
      stmt.Left.Name, GetType result)` for each return on
      `lexer.am`, compare to expected.

---

## 🟢 Ecosystem — outillage et docs

- [x] **`amc fmt`** — formatter (v0.2.0). Idempotent on every
      compiler source. Re-emits comments by source line.
- [x] **`amc test [<dir>]`** (v0.3.4) — discovers `*_test.am`,
      compiles + runs each via `Process.RunCapture`, aggregates
      `[PASS]`/`[FAIL]`/`[SKIP]` lines from stdout. Crash with
      no tags surfaces as `[FAIL] <crash> exit=N`. Convention is
      framework-free for v1; a richer Assert module + `test_*`
      auto-discovery is a possible v2.
- [x] **`amc --lint`** (v0.3.3 unreachable, v0.3.4 unused/shadow)
      — `src/linter.am` walks the AST and flags:
      unreachable code after `return` / `throw` / `break` /
      `continue` (incl. nested blocks); unused locals (`let x = …`
      never read; `_` prefix silences); shadowed names. Method/
      lambda params participate in shadow detection but never
      get warned-on as unused. Still TBD: suspicious patterns,
      catch-binder unused detection (parser puts them at a node
      we don't yet walk).
- [ ] **`amc new <name> [--template <kind>]`** — scaffolding command,
      à la `cargo new` / `dotnet new`. Creates a `<name>/` directory
      with the right starter files for the chosen template:
        - `exe` (default) — `src/main.am` with a `Program.Main` skeleton
          + a `tests/hello_test.am`. Minimal "compiles and runs"
          starting point.
        - `lib` — `src/<name>.am` with a `public class` skeleton, a
          README pointing at the `--lib` flag, no `Program.Main`.
        - `test` — pure test bundle (`tests/<name>_test.am` with a
          PASS/FAIL example), useful when starting from an existing
          codebase to add a test layer.
        - Future: `cli` (with arg parsing skeleton), `web` (HTTP
          server skeleton tied to the `Http` stdlib), `fmt-plugin`,
          etc.
      All templates ship a `.gitignore`, a `README.md` stub, and a
      `build.sh` calling `amc` directly. Implementation: a small set
      of file templates in `src/templates/` (or hard-coded strings
      in `src/main.am` to stay self-contained), a CLI dispatcher
      branch in `main.am`, and a sample roundtrip test that
      scaffolds + compiles a fresh project under `/tmp`.
- [ ] **`amc doc`** — extract doc-comments and emit Markdown / HTML.
- [ ] **`amc add <pkg>`** — package manager (re-export of the legacy
      Vala one in `archive/vala-bootstrap/src/pkg/`).
- [x] **`amc lsp` (diagnostics)** (v0.3.4) — minimal LSP 3.x server
      over stdio JSON-RPC. Implements lifecycle (`initialize` /
      `shutdown` / `exit`), document state (didOpen / didChange /
      didClose, Full sync), and `publishDiagnostics` push on every
      did{Open,Change}. Diagnostics merge resolver + typechecker
      errors, range covers the whole token. Hover and completion
      land in v0.3.5; goto-def remains out of scope.
- [x] **VS Code LSP client** (v0.3.4) — `editors/vscode/extension.js`
      spawns `amc lsp` via `vscode-languageclient`. Configurable
      via `amalgame.serverPath` and `amalgame.enableLsp`.
- [x] **`amc lsp` hover + global completion** (v0.3.5) — follow-up on top of v0.3.4
      diagnostics. Needs pos→symbol lookup on the AST.
- [ ] **Editor integration on install** — when a user installs
      Amalgame (`install.sh`, future `amc-up` package script,
      Homebrew formula, `.deb`/`.rpm`), automatically wire the
      LSP into the editors present on the host:
        - **VS Code / VS Code Insiders / VSCodium**: detect via
          `code --list-extensions`; if missing, install
          `editors/vscode/` from the local `.vsix` bundled in
          the release tarball, or publish to the Marketplace and
          install by ID. Set `amalgame.serverPath` to the resolved
          `amc` binary so the extension doesn't depend on `$PATH`.
        - **Neovim**: drop a `lspconfig` snippet into
          `~/.config/nvim/lua/amalgame_lsp.lua` and print the
          one-line `require("amalgame_lsp")` users add to
          `init.lua` (don't edit `init.lua` itself — too many
          competing setups).
        - **Helix**: append an `[[language]]` block to
          `~/.config/helix/languages.toml` (idempotent — skip if
          already present).
        - **Zed / Sublime / Emacs**: emit a one-page setup hint
          in the install summary pointing at `docs/guide/`.
      Implementation: a post-install step (`install/setup-editors.sh`)
      that's opt-out via `--no-editors` and prints a per-editor
      summary at the end ("VS Code: ✓ extension installed",
      "Neovim: snippet at <path>, source it from your init.lua").
      Bundle the `.vsix` in release tarballs (already bundling docs
      since v0.4.0) so this works air-gapped. Open question:
      auto-detect editors vs. interactive prompt.
- [ ] **DAP** — debug adapter using DWARF (`-g3` already emitted).
- [ ] **Inlay hints + code actions** — once hover/completion is in.

### Stdlib — backlog

- [ ] **Core stdlib expansion** — fill in the gaps that everyday
      Amalgame code currently has to fake or shell out for:
      - `Amalgame.DateTime` — wall-clock, monotonic, parsing,
        formatting, durations.
      - `Amalgame.Json` — parse + serialize, schemaless
        `JsonValue` tree first, typed binding later.
      - `Amalgame.Regex` — PCRE-style or RE2 binding, capture
        groups exposed as `Match` records.
      - `Amalgame.Random` — seeded PRNG + crypto-grade source
        for tokens / IDs.
      - `Amalgame.Encoding` — Base64, hex, URL encode/decode.
      - `Amalgame.Compress` — gzip, deflate (zip later).
      - `Amalgame.Crypto` — SHA-256, HMAC, constant-time compare.
      - `Amalgame.Threading` — at minimum a thread pool +
        Mutex/Channel; needs runtime-side care around libgc.
      Each is a small project on its own; ship as separate PRs
      and add docs/guide entries in lockstep. Tied to the open
      "Stdlib delivery model" design question below.
- [ ] **GUI / Forms toolkit** — bindings SDL2 dans la stdlib
      (`Amalgame.UI` ou similaire) avec une couche "Forms" au-dessus
      pour les widgets courants (Window, Button, TextField, Layout).
      Permettrait d'écrire des apps graphiques en Amalgame sans
      descendre au C. Choix de design ouverts : retained vs immediate
      mode, theming, accessibilité, packaging du runtime SDL2 dans
      les releases.

### Distribution
- [x] GitHub Actions CI (Linux/macOS/Windows)
- [x] GitHub Releases automation (tag-triggered)
- [ ] Homebrew tap (formula draft in `install/homebrew/amalgame.rb`)
- [ ] Homebrew core (after public adoption)
- [ ] AUR / `.deb` / `.rpm` / Nix flake / winget / Scoop
- [ ] `install.sh` universal one-liner
- [ ] Windows packaged installer (.msi or .exe with bundled MinGW
      gcc + libgc + libcurl, so end users don't need MSYS2). Sketched
      in conversation; no script yet.
- [ ] **URL sweep** — 25 occurrences of the old
      `BastienMOUGET/Amalgame` URL still live in the tree post-org-
      transfer (PR #187). Touches `runtime/Amalgame_*.h` headers,
      `install/homebrew/amalgame.rb`, `install/windows/install.ps1`
      + `amalgame.iss`, `editors/vscode/package.json`,
      `archive/vala-bootstrap/**`. CHANGELOG mention of the transfer
      itself stays as historical record. One trivial sed pass +
      smoke-test the homebrew formula locally — defer until after
      the next stdlib batch lands.

### Documentation
- [x] User guide (`docs/guide/`)
- [x] README that doesn't lie about features
- [ ] Static site (docs.amalgame-lang.org)
- [ ] Tour interactif à la go.dev/tour
- [ ] EBNF grammar (file exists at `docs/language/grammar.ebnf` — to
      be re-validated against the current self-hosted parser)
- [ ] Cookbook of idiomatic snippets

---

## Open design questions

- **`?.` chaining cost** — current emit double-evaluates the receiver.
  Switch to a GCC compound statement expression for safety against
  side effects.
- **Generic erasure vs. monomorphisation** — the current C-erased
  approach is simple but precludes useful overload sets.
  Monomorphisation would change the ABI substantially.
- **Module/import system** — imports are informational today. A real
  module system needs interface files (`.ami`?) and a resolver that
  uses them rather than scanning the global stdlib.
- **Stdlib delivery model** — runtime is currently header-only
  (`runtime/*.h`, all `static inline`, `-Iruntime` at link).
  Simple and bootstrap-friendly, but compile time grows with the
  stdlib and every external dep (`-lcurl`, `-lpcre`, `-lcrypto`…)
  is passed by hand at link time. Alternatives to weigh:
    - **A. Status quo — header-only inline.** Simplest; linker
      dead-code-elims; deps stay explicit per program.
    - **B. Static `libamalgame.a`.** Pre-compiled; users get
      faster compiles. Cost: ship per OS/arch, harder bootstrap.
    - **C. Shared `libamalgame.so/.dylib/.dll`.** Runtime
      upgradable. Cost: rpath/loader fragility, ABI versioning,
      messy distribution.
    - **D. Per-module dead-code stripping in `amc`.** Keep
      header-only but emit only runtime symbols actually used
      (transitively). Smallest binaries; narrow deps. Cost:
      symbol catalogue + a symbol-graph pass in the compiler.
    - **E. Hybrid — primitives inline, modules in Amalgame.**
      Keep `String_Length` & co. header-only. Write Json /
      Regex / DateTime in `.am` under `stdlib/`, imported via
      a real (not informational) module system. Lands well
      with the "Module/import system" question above.
  Bootstrap-curiosity → A is fine. Daily driver → D or E
  (likely E once imports are physical).
- **Error vs. exception model** — `try/catch/throw` works under Vala
  but is missing in self-host. Worth replacing with a Rust-like
  `Result<T, E>` plus `?` operator for short-circuiting before
  re-implementing the setjmp version.
- **Single-threaded** — bdwgc is configured for the main thread. If
  Amalgame ever wants concurrency, the GC config and runtime helpers
  need a pass.

---

## How to pick the next thing

Top of the list, ordered by *unlocked-value* per *days-of-work*:

1. **Lambda v2.5 — non-int signatures** — i64-shaped Filter /
   Map / Reduce / ForEach are in (List), but `xs.Map(x => x.Name)`
   over a `List<Class>` still needs (a) TypeChecker to infer the
   lambda's expected signature from the formal param at the call
   site, (b) CGen to emit non-int `lam_N_fn` signatures based on
   the inferred shape. Unlocks the rest of real-world lambda usage.
2. **Stdlib expansion** — pick one or two modules from the stdlib
   backlog (`DateTime`, `Json`, `Regex` are the most missed).
   Independent of compiler/tooling work, so can run in parallel
   with the language items. Each is a 200-400 LoC PR. Tied to the
   open "Stdlib delivery model" question — the early modules will
   be done header-only, but reaching ~10 modules is when options
   D/E start paying off.
3. **LSP member completion** — `obj.<cursor>` narrowed to the
   methods/fields of `obj`'s type. Needs a position→receiver→type
   →members chain on top of v0.3.5's global completion. Probably
   ~150 LoC once the receiver-resolution helper is in place.
4. **`amc new <name> [--template …]`** — scaffolding command à la
   `cargo new` / `dotnet new`. File templates (exe/lib/test) +
   a dispatcher branch in `main.am`. ~200-400 LoC. Big onboarding
   win — turns "clone the repo and stare at src/" into "amc new
   hello && cd hello && ./build.sh".
5. **`amc test` polish** — `--runtime <path>` flag (don't assume
   cwd has `runtime/`), per-file timeouts, parallel execution.
6. **Process v2** — split stderr from stdout via real pipes,
   add timeouts, async streaming output for long-running children.
7. **Spread operator** — `f(...args)` and `[...a, ...b]`. Needs
   list literal syntax `[...]` first.
