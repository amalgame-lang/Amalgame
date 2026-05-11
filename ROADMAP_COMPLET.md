# Amalgame — Roadmap

> Updated 2026-05-11 · `amc 0.5.4` · self-hosted · 491/491 tests · multi-OS CI · GitHub Releases automation · package manager + ecosystem (incl. DuckDB) + C++ pipeline + precompile-on-install + auto-learning calibration ETA

This document is the canonical "what's done, what's next" board.
For architecture and contribution guidance see
[docs/guide/](docs/guide/README.md).

---

## ✅ Shipped

### Self-hosted compiler
The compiler is written in Amalgame in [src/](src/) and compiles
itself in ~5 seconds (`./build_amc.sh`). A 2-rung bootstrap chain
keeps recovery easy:

1. **`./amc`** — current self-hosted compiler.
2. **`./snapshot/amc`** — last known-good amc, captured by
   `tools/save-snapshot.sh` after a green test run. The portable
   `snapshot/amc_lib.c` is committed; the binary regenerates with one
   `gcc`. Used as fallback whenever `./amc` is broken mid-development.
   From a clean clone, this is also the cold-start entry point.

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
- try / catch / throw — implemented in self-host (PR landed);
  `finally` clause still TBD.

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
- Multi-OS CI (Linux + macOS + Windows MSYS2). All three platforms
  bootstrap from the tracked `snapshot/amc_lib.c` and run the
  self-hosted `amc` for testing.
- Tag-driven Release workflow (Linux .tar.gz + macOS .tar.gz + Windows
  .zip with bundled MinGW DLLs)
- **470/470 tests** in CI under `./amc` (205 core + 219 stdlib +
  12 fmt + 34 amc-new), zero SKIP. Suite grew with the v0.4.0
  LLM features (hermetic — no real LLM calls), the v0.4.x stdlib
  pushes (Path / Logging / Service / Crypto / DateTime), and the
  v0.5 package-manager work (lockfile / manifest / cache tests).
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

These samples trigger bugs in the self-hosted compiler. They're
marked SKIP in `tests/run_tests.sh` (`SKIP_SELFHOST`) so the suite
stays green; each one needs its own fix.

- [x] **`Type.Variant` patterns in match** — `ParseMatchPattern`
      now reads an optional `.IDENT` suffix and emits an `Ast.Member`,
      which the existing CGen MEMBER branch lowers to `Type_Variant`.
      Unblocks `enums.am`.
- [x] **try / catch / throw** — `ParseTry` and `ParseThrow` build
      `TRY_STMT { Body=try, Else=catch, Cond=finally?, Name=binder }`
      and `THROW_STMT { Left=expr }`. Resolver/typechecker open a
      `catch` scope and declare the binder as `void*`. CGen emits a
      `setjmp`/`longjmp` pattern (saves `_am_ex.env` into a
      `_am_prev_env_<line>` slot, restores it after the handler) and
      lowers `throw new T(args)` to
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
- [ ] **CGen: constructor forward-decls don't precede call
      sites.** Surfaced while writing `Amalgame.DateTime`
      (2026-05-09). If class A's constructor calls `new B(...)`
      and B is declared later in the same file, the bootstrap
      cgen emits A's constructor body before B's `_new`
      function, triggering an implicit declaration warning that
      then conflicts with the real signature ("conflicting types
      for B_new"). gcc still tolerates it as a warning so the
      build limps through, but the produced binary may be wrong
      if the compiler picks `int()` semantics for the implicit
      decl. Workaround applied in datetime.am: `InstantResult`
      takes the initial Instant as a constructor parameter
      instead of building one inline. Real fix is to forward-
      declare every `<Class>_new` signature at the top of pass2
      output, before any class body emits.
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
          `service` (long-running background process — cross-platform
          install/start/stop hooks: systemd unit on Linux, launchd
          plist on macOS, Windows Service via `sc create`/SCM stubs;
          template ships a `Service.Run()` with signal-aware shutdown
          and a sample `journalctl`/`Console`-friendly logger), and
          `forms` (cross-platform GUI app — SDL2-or-equivalent
          binding under the hood; template ships a Window + Button +
          TextField sample with the platform DLL/dylib/so resolution
          handled in the build script). `service` and `forms` both
          need extra runtime headers (signal/SCM glue and an SDL
          binding respectively), so their templates land alongside
          `Amalgame.Service` / `Amalgame.UI` stdlib modules — see
          "Stdlib gaps" below for the matching entries.
      All templates ship a `.gitignore`, a `README.md` stub, and a
      `build.sh` calling `amc` directly. Implementation: a small set
      of file templates in `src/templates/` (or hard-coded strings
      in `src/main.am` to stay self-contained), a CLI dispatcher
      branch in `main.am`, and a sample roundtrip test that
      scaffolds + compiles a fresh project under `/tmp`.
- [ ] **`amc doc`** — extract doc-comments and emit Markdown / HTML.
- [x] **`amc package <action>`** (v0.5.0 → v0.5.2) — full package
      manager. `add <git-url>@<tag>` clones + validates + records,
      `remove` / `list` / `search` / `update` / `cache` round out
      the CLI (PR #303 grouped them under `amc package`, alias
      `amc pkg`). Storage at `~/.amalgame/packages/<host>/<owner>/
      <repo>/<tag>_<sha>/`. `amalgame.toml` (deps) +
      `amalgame.lock` (resolved SHAs) live in the project root.
      `amc test` is package-aware: auto-installs missing deps
      (v0.5.1) and links each package's `[stdlib].sources` `.c`
      files into every test binary (v0.5.2).
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
- [x] **`amc lsp` foldingRange** (slice 5) — token-driven brace-pair
      matching for class / method / block bodies; `kind:"comment"`
      runs for consecutive `//` lines; `kind:"imports"` runs for
      consecutive `import …` statements. Filters one- and two-line
      blocks so the gutter stays uncluttered. Capability advertised
      as `foldingRangeProvider: true`.
- [ ] **`amc lsp` navigation (v2)** — the next-tier features users
      hit fastest after diagnostics + completion are working:
        - **`textDocument/definition`** — jump to where a symbol is
          declared. Reuses the resolver's symbol table; pos→token →
          symbol → `(file, line, col)`. Probably the highest-value
          single feature here.
        - **`textDocument/declaration`** — separate endpoint per LSP
          spec; for languages without forward declarations it
          aliases definition. Trivial passthrough.
        - **`textDocument/typeDefinition`** — for `let x: T = …`
          jumps to T's definition. Needs the typechecker to expose
          the inferred type at a given position.
        - **`textDocument/references`** + **`Find all references`** —
          enumerate every use of a symbol across the workspace. Needs
          a reverse index; cheap to build during resolve since we
          already walk every reference site.
        - **`textDocument/documentHighlight`** — highlight other
          occurrences of the symbol under the cursor in the current
          file. Subset of references, scope-limited.
        - **`textDocument/documentSymbol`** — outline view (classes,
          methods, top-level decls). Walks the AST top-level + class
          children, emits SymbolKind.{Class,Method,Field,Enum}.
        - **`workspace/symbol`** — fuzzy-search every declared symbol
          in the workspace. Needs the same reverse index as references.
        - **`textDocument/prepareCallHierarchy`** +
          **`callHierarchy/{incoming,outgoing}Calls`** — jump-to-callers
          / jump-to-callees views. Builds on references for incoming;
          outgoing is a method-body walk for CALL nodes.
        - **Hover preview ("Aperçu")** — peek-style inline preview
          panel. We already serve hover; "Peek" is the editor's
          rendering of the same data, so usually no extra server
          work. VS Code wires it automatically.
        - **`textDocument/rename`** — rename a symbol across the
          workspace. Builds on references; emit a `WorkspaceEdit`
          with one TextEdit per use site.
        - **`textDocument/inlayHint`** — inferred-type hints at
          `let x = …` positions. Needs the inferred-type lookup
          from typeDefinition above.
        - **`textDocument/codeAction`** — quick fixes (e.g. "wrap
          this top-level fn in a class as `public static`" —
          recently emitted as a parse diagnostic). Per-diagnostic
          dispatcher that returns a `WorkspaceEdit`.
        - **`textDocument/foldingRange`** — **DONE in slice 5**
          (see the v0.4.17 `[x]` entry above). Token-driven
          brace-pair matching for class / method / block bodies;
          `kind:"comment"` for `//` runs; `kind:"imports"` for
          consecutive `import` statements. Multi-line `if`/`while`/
          `for` blocks are covered by the brace-pair logic;
          multi-line `match` arms remain a v2 polish.
- [ ] **`amc lsp` performance — workspace resolver caching.**
      Every hover / completion / definition call rebuilds the
      whole workspace resolver: parse the open file, walk every
      sibling `.am`, parse each, collect+resolve. On a 30-file
      workspace that's ~2.5–3s per request — fast enough for
      diagnostics-on-save but noticeably slow for Cmd+Click
      definition (VS Code shows the spinner). Cache the resolver
      across requests, invalidate on `didChange` /
      `didCreate` / `didDelete`. ~1 day of work; biggest LSP
      UX win after the v2 navigation features themselves.
- [ ] **VS Code extension robustness on `serverPath`.** The
      extension currently does `child_process.spawn(serverPath)`
      with the user-set value verbatim. Two real-world traps:
      a leading whitespace in the JSON setting silently makes
      the path look like ` /home/.../amc` (ENOENT), and a `~/`
      prefix isn't expanded by `spawn`. Fix in
      `editors/vscode/extension.js`: `serverPath.trim()` then
      `serverPath.replace(/^~/, os.homedir())` before spawning.
      ~5 LoC, unblocks anyone who configures `amalgame.serverPath`
      with the natural shell-style value.
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
      - [x] `Amalgame.Json` — parse + serialize, schemaless
        `JsonValue` tree first, typed binding later. (PR #182, #183, #193)
      - [x] `Amalgame.Random` — seeded PRNG + crypto-grade source
        for tokens / IDs. (PR #200)
      - [x] `Amalgame.Encoding` — Base64, hex, URL encode/decode.
        (PR #201)
      - [x] `Amalgame.DateTime` — Instant + Duration + Stopwatch,
        UTC-only, RFC 3339 strict (PR #202). Local time and
        timezone follow-up tracked below.
      - [x] `Amalgame.Crypto` — SHA-256 + HMAC-SHA-256 (FIPS 180-4
        + RFC 2104), pure-C runtime header, no external dep
        (PR #213). Constant-time compare deferred — caller's
        responsibility documented in the guide.
      - [ ] `Amalgame.Regex` — PCRE-style or RE2 binding, capture
        groups exposed as `Match` records.
      - [ ] `Amalgame.Compress` — gzip, deflate (zip later).
      - [ ] `Amalgame.Threading` — at minimum a thread pool +
        Mutex/Channel; needs runtime-side care around libgc.
      Each is a small project on its own; ship as separate PRs
      and add docs/guide entries in lockstep. Tied to the open
      "Stdlib delivery model" design question below.

### Stdlib gaps — second tier

Beyond the core "fill obvious holes" list above, here's the inventory
of things a reasonably complete stdlib usually has and Amalgame
currently doesn't. Ordered by rough user-facing value, not by
implementation effort.

- [ ] **`Amalgame.Audio`** — playback + capture + basic synthesis.
      Recommended path: a runtime header binding [miniaudio](https://miniaud.io/)
      (single-file C library, MIT, cross-platform via WASAPI on
      Windows / CoreAudio on macOS / ALSA-or-PulseAudio on Linux).
      Public surface: `AudioBuffer.Load(path)` for WAV/MP3/Ogg
      decode, `AudioStream.Play(buf)` / `.Pause()` / `.Stop()`,
      `AudioMixer` for multi-source playback, `AudioRecorder` for
      mic capture. Linker flags: `-ldl -lpthread -lm` on Linux,
      `-framework AudioToolbox -framework CoreAudio` on macOS,
      `-lole32 -lwinmm` on Windows. ~400 LoC stdlib + ~200 LoC
      runtime (mostly miniaudio passthrough). Stretch: MIDI in/out.
- [x] **`Amalgame.Database.SQLite` v1** (PR #266, v0.4.15) —
      SQLite 3 binding via the vendored amalgamation
      (`runtime/Amalgame_Database/sqlite/`). Public-domain
      upstream, no `libsqlite3-dev` package needed on any OS.
      Surface: `SQLite.Open(path) → AmalgameSQLite*`, `Close`,
      `IsOpen`, `Exec(sql)`, `QueryAll(sql) → List<List<string>>`,
      `LastInsertId`, `Changes`, `LastError`. Parameter binding
      via `?` placeholders is the v2 ask.
- [x] **`Amalgame.Database.NoSQL.Redis` v1** (v0.4.17) —
      pure-protocol RESP2 client over raw TCP, no vendored client
      lib. Speaks the wire format Redis / KeyDB / Dragonfly /
      Valkey share. Runtime header
      (`runtime/Amalgame_Database_Redis.h`, ~340 LoC) reuses the
      cross-platform socket layer from `Amalgame_Net.h`. Surface:
      `Redis.Open(host, port) → AmalgameRedis*`, `Close`,
      `IsOpen`, `LastError`, `Ping`, `Set`, `Get`, `Del`,
      `Exists`, `Incr`, `Decr`, `Expire`. Test fixture gates on a
      TCP reachability probe — every case SKIPs cleanly if no
      server is up, runs the round-trip otherwise. AUTH / SELECT,
      pipelining, pub/sub, MULTI/EXEC, SCAN, TLS, binary values
      with embedded NULs are v2.
- [x] **`Amalgame.Messaging.MQTT` v1** (v0.4.18) —
      pure-protocol MQTT 3.1.1 client over raw TCP, no vendored
      lib (no `libmosquitto` / `libpaho` at link time). Runtime
      header (`runtime/Amalgame_Messaging_MQTT.h`, ~450 LoC)
      implements CONNECT/CONNACK, PUBLISH (QoS 0), SUBSCRIBE/
      SUBACK, PINGREQ/PINGRESP, DISCONNECT, and the variable-
      length encoding for "remaining length". Surface:
      `MQTT.Open(host, port, clientId) → AmalgameMQTT*`,
      `Close`, `IsOpen`, `LastError`, `Ping`, `Publish(topic,
      payload)`, `Subscribe(topic)`, `WaitMessage(timeout_ms)
      → bool`, `LastTopic`, `LastPayload`. Lands the
      `Amalgame.Messaging.*` namespace as the parallel to
      `Amalgame.Database.*`. QoS 1/2, retain, will, auto-
      keepalive, multi-topic SUBSCRIBE, MQTT 5, TLS, username/
      password auth, broker-side wildcards (`+`/`#`) exercised
      in tests are v2.
- [x] **Stdlib package manager — extract optional backends as
      packages** (v0.5.0 → v0.5.2, PRs #284–#304). All three
      inaugural backends extracted into stand-alone repos
      under `amalgame-lang`:
      [amalgame-database-sqlite](https://github.com/amalgame-lang/amalgame-database-sqlite),
      [amalgame-database-nosql-redis](https://github.com/amalgame-lang/amalgame-database-nosql-redis),
      [amalgame-messaging-mqtt](https://github.com/amalgame-lang/amalgame-messaging-mqtt).
      `amc package add` / `remove` / `list` / `search` / `update`
      / `cache` round out the CLI (alias `amc pkg`). `amc test`
      auto-installs missing deps and links each package's
      `[stdlib].sources` vendored C files into every test binary,
      so SQLite "just works" with no manual gcc step. Original
      design space (kept below for archaeology):

      **Release batching policy** — to avoid burning CI + release-
      flow cycles on every backend addition between now and the
      package manager landing, we batch features-headed-for-
      extraction directly into the v0.5 release rather than
      cutting intermediate patch releases. Concretely: MQTT is
      shipped on `develop` but no v0.4.18 release will be cut for
      it; it rolls into v0.5 alongside the package manager + the
      SQLite/Redis/MQTT extraction. Same rule for any further
      backend that lands before v0.5 (DuckDB, Postgres, etc.).
      Once the package manager is live, each external package
      cuts its own releases on its own cadence — totally
      decoupled from the compiler's version.

      Design space:
        - **Manifest format** — `package.am` or `amalgame.toml`
          listing name, version, runtime headers, link flags,
          deps. Has to be parseable by the package manager
          without bootstrapping the full Amalgame compiler.
        - **CLI** — `amc add <repo-url>` git-clones the package
          into `~/.amalgame/packages/<name>/` (and pins a
          revision in a project-local lock file). `amc remove`,
          `amc update`, `amc list` round out the surface.
        - **Resolver path** — `import Amalgame.Foo.Bar` walks a
          package search path; package manifest tells resolver
          which globals to declare instead of today's hardcoded
          list in `src/resolver/resolver.am`.
        - **CGen plugin** — package manifest declares isStdlib
          class names + return-type table per stdlib function,
          replacing today's hardcoded blocks in
          `src/generator/c_gen.am`.
        - **Linker** — manifest declares native deps
          (`-lcurl`, `-lz`, vendored amalgamation .c sources
          to compile alongside the user binary).
        - **Inaugural packages** — extract
          `Amalgame.Database.SQLite`,
          `Amalgame.Database.NoSQL.Redis`, and
          `Amalgame.Messaging.MQTT` to
          `amalgame-lang/amalgame-database-sqlite`,
          `amalgame-lang/amalgame-database-nosql-redis`, and
          `amalgame-lang/amalgame-messaging-mqtt`. All three already use
          the runtime-header pattern so the migration is mostly
          moving files + writing the manifest. Validates the
          design against three distinct shapes: vendored .c
          amalgamation (SQLite), binary protocol with simple
          framing (Redis RESP2), and binary protocol with
          variable-length encoding + bidirectional async
          delivery (MQTT 3.1.1).
      ~1 week of work; gates further DB / Messaging backends
      so we don't keep adding to the monolithic compiler tree.
- [ ] **ORM layer(s) on top of the DB backends** — once the
      package ecosystem has 2–3 SQL siblings (Postgres / DuckDB
      alongside SQLite) the per-backend Open/Exec/QueryAll
      surface starts wanting an ergonomic class-to-table layer.
      Open design question: **one umbrella ORM** abstracting
      every engine, or **per-family ORMs** (one for SQL, one
      for NoSQL document stores, one for KV)?
        - **Unified-ORM path** (à la Hibernate / SQLAlchemy /
          Entity Framework) — one package
          `amalgame-database-orm` providing a `[Table]`-style
          decorator surface that dispatches to whatever
          backend the user has installed (SQLite, Postgres,
          MySQL, …). High consistency, but every SQL engine's
          quirks (JSON columns, RETURNING, vendor functions)
          leak into the abstraction.
        - **Per-family path** (à la Diesel / Mongoid / RedisOM)
          — separate `amalgame-database-orm-sql`,
          `amalgame-database-orm-nosql`,
          `amalgame-database-orm-kv` packages. Each focuses on
          one paradigm's strengths. SQL ORM still dispatches
          to SQLite / Postgres / MySQL backends via a thin
          driver layer; NoSQL ORM targets Mongo / DynamoDB /
          Cosmos. KV ORM (sometimes called "key-pattern
          mapping") targets Redis / Memcached.
        - **Middle ground**: one ORM crate per *backend* with
          a shared protocol crate (`amalgame-database-orm-core`)
          they all implement. Compatible with Cargo's split
          between `diesel` core + `diesel_derives` + per-DB
          backends.
      Whatever shape lands first, it builds on top of the
      backend packages — never inside them — so backends can
      evolve their wire-level concerns independently of any
      ORM choice. Tracked here to be discussed before any
      backend's surface gets enriched in ways that lock in
      an ORM shape (e.g. typed column accessors, prepared-
      statement caches, transaction handles).
- [ ] **`Amalgame.Database.<Engine>` siblings — SQL backends** —
      extend the `Amalgame.Database.*` namespace with bindings to
      other relational engines. Each gets its own header
      (`Amalgame_Database_<Engine>.h`) and resolver-declared
      globals so users opt-in to the surface they need without
      compiling everything. Shared `Result` / `Rows` types
      consolidate into a common `Amalgame_Database.h` once a
      third engine lands and the pattern is clear.
        - [x] **DuckDB** (v0.5.3, `amalgame-database-duckdb`) —
          vendored C++ amalgamation (`duckdb.cpp` + `duckdb.h`, MIT
          licence). OLAP-flavoured workloads; columnar storage +
          vectorised execution. Surface mirrors SQLite
          (Open/Close/IsOpen/Exec/QueryAll/LastError) so callers
          swap engines without rewriting. Shipped alongside the
          v0.5.3 C++ pipeline (`[stdlib].sources` of type
          `.cpp/.cc/.cxx` compile with g++, manifest gains
          `cflags`/`cxxflags`/`libs`/`schema-version` keys).
        - **PostgreSQL** (`libpq` client) — link to system
          `libpq` (heavy to vendor — vendor only the headers,
          dynamic-link the .so/.dylib/.dll). Surface adds
          `SetUser` / `SetPass` / connection-string variants for
          network auth. PostgreSQL licence is permissive (BSD-
          style), compatible with Apache-2.0.
        - **MySQL / MariaDB** (`libmariadbclient`) — same
          dynamic-link path as Postgres. Lower priority.
          MariaDB connector under LGPL-2.1 → fine to dynamic-link
          (LGPL doesn't taint dynamically-linked consumers).
        - **Oracle Database** — only the **Oracle Instant Client**
          (free download, proprietary) ships the headers + libs
          needed (`oci.h`, `libclntsh.so` / `oci.dll`). Cannot
          vendor; must dynamic-link at runtime and document the
          one-time `Instant Client` install for the operator.
          Connection: `oraclite.<host>:<port>/<service>`. Surface
          adds session pooling (`OCISessionPool*`) since Oracle
          connections are expensive to set up.
        - **Microsoft SQL Server** — three viable client paths:
          1. **MS ODBC Driver for SQL Server** — proprietary
             distributable from Microsoft; works on Linux / macOS /
             Windows. Dynamic-link to `libodbc` + the MS driver.
             Surface: connection-string-based (`Driver={ODBC Driver
             18 for SQL Server};Server=...;…`).
          2. **FreeTDS** — open-source TDS protocol library,
             LGPL. Dynamic-link, no Microsoft distributable
             needed. Less feature-complete (no AAD auth, older
             TDS versions only).
          3. **Tiberius (Rust)** — rules out FFI-from-Amalgame
             complexity; skipped.
          Default to ODBC for parity with the .NET ecosystem;
          FreeTDS as a fallback for fully-FOSS deployments.
- [ ] **`Amalgame.Database.SQLite` v2** — parameter binding
      (`db.ExecBind(sql, params)` / `db.QueryBindAll(sql, params)`),
      typed column accessors (`row.AsInt(0)` / `row.AsBytes(2)`),
      prepared-statement reuse, transactions (`db.Begin` / `Commit`
      / `Rollback`). Same pattern applies to sibling engines.
- [ ] **`Amalgame.Database.NoSQL.<Engine>` — document / KV /
      column stores.** Different surface from the SQL family:
      no `Exec(sql)` / `QueryAll(sql)`, instead JSON-document
      reads and writes with engine-native query expressions.
      Shared namespace prefix `Amalgame.Database.NoSQL.*` to keep
      it discoverable next to the SQL siblings; types and
      patterns deliberately diverge.
      **Status**: **Redis shipped** as v0.2.0 (package
      `amalgame-database-nosql-redis`, protocol-only). Mongo,
      DynamoDB, Cassandra still pending — see sub-bullets below.
        - **MongoDB** — link to `libmongoc` + `libbson`
          (Apache-2.0). Surface: `Mongo.Connect(uri)`,
          `Mongo.InsertOne(collection, json)`,
          `Mongo.FindMany(collection, filter_json) → List<string>`
          (each entry is the document serialised as JSON; the
          caller reparses with `Amalgame.Json`). Aggregation
          pipelines, indexes, change-streams in v2.
        - **Redis** — pure-protocol client, no vendored lib
          needed — RESP3 is ~300 LoC of socket + parser code.
          Surface: `Redis.Connect(host, port)`, `Redis.Get(key)`,
          `Redis.Set(key, value)`, `Redis.Del(key)`, `Redis.Exists`,
          plus pub/sub. Fits naturally as the "cache + ephemeral
          KV" companion to SQLite's "durable structured" role.
        - **DynamoDB / Cosmos DB / Firestore** — service APIs over
          HTTP; can be built on top of `Amalgame.Net.Http` +
          `Amalgame.Json` without a vendored driver. Lower
          priority since they're cloud-vendor-specific; tracked
          for completeness.
        - **Cassandra / ScyllaDB** — CQL over the native binary
          protocol. Either dynamic-link to `libcassandra` or
          implement the binary protocol in Amalgame. Defer to v2.
      All NoSQL backends expose a `<Engine>.LastError()` and
      maintain a single connection handle per facade, same
      ergonomic shape as the SQL family.
- [ ] **`Amalgame.Messaging.<Broker>` — message brokers** — the
      missing tier between "in-process work loop" and "SQL/NoSQL
      durable storage". Covers pub/sub, work queues, event
      streams, the patterns that decouple services. Two families
      based on protocol complexity:

      **Status**: **MQTT v3.1.1 shipped** as v0.2.0 (package
      `amalgame-messaging-mqtt`, protocol-only, Connect / Publish
      / Subscribe / Loop, QoS 0). Kafka, RabbitMQ, NATS, AMQP
      still pending — see sub-bullets below.

      **Pure-Amalgame implementations** — implement the wire
      protocol in `.am`, no vendored library, no system package.
      Faster to ship, fewer install steps, lower surface area.

        - **MQTT v3.1.1 / v5** — binary protocol, ~300 LoC of
          packet encode/decode + socket reader. Built on top of
          `Amalgame.Net.TcpClient`. Surface:
          `Mqtt.Connect(host, port, clientId)`,
          `Mqtt.Publish(topic, payload, qos)`,
          `Mqtt.Subscribe(topic, qos, handler: (msg) => ...)`,
          `Mqtt.Loop()`. Targets the IoT + embedded story —
          tiny binary, no native dep, runs on every OS Amalgame
          supports.
        - **NATS Core** — text protocol over TCP, ~250 LoC. Pub/
          sub + request/reply. Closest fit for the "Redis but
          for messages" mental model. Same handle + Subscribe
          callback shape as MQTT.

      **Dynamic-link to native broker client** — for protocols
      where the upstream library does work we don't want to
      re-implement (compression, SASL, advanced ack semantics,
      partition consumer groups).

        - **Apache Kafka** (`librdkafka`, BSD-2-clause) —
          producer + consumer. Surface:
          `Kafka.Producer(brokers)`,
          `Kafka.Send(topic, key, value)`,
          `Kafka.Consumer(brokers, groupId, topics)`,
          `Kafka.Poll(timeout) → KafkaMessage?`. The librdkafka
          client handles partition assignment, offset commits,
          delivery reports. Surface deliberately thin so the
          underlying tunables (acks, compression, retries)
          stay accessible via setter methods.
        - **RabbitMQ / AMQP 0.9.1** (`librabbitmq`, MIT) —
          publish + consume on named exchanges and queues.
          Surface: `Rabbit.Connect(host, port)`,
          `Rabbit.DeclareExchange(name, kind)`,
          `Rabbit.DeclareQueue(name)`, `Rabbit.Bind(queue,
          exchange, routingKey)`, `Rabbit.Publish(exchange,
          routingKey, body)`, `Rabbit.Consume(queue, handler)`.
          Heavier surface than MQTT/NATS because AMQP itself is.

      **Out of scope for v1:** ZeroMQ (LGPL is fine but ZMQ
      itself is a library, not a broker — different mental
      model), Pulsar (smaller user base than Kafka, can be
      added later via the C++ client).

      Common ergonomics across all brokers: single connection
      handle per facade, `<Broker>.LastError()` for diagnostics,
      blocking + callback-style consumer APIs, no async/await
      requirement on the caller (the Amalgame side stays
      synchronous; concurrency comes from running the consumer
      loop in a thread spawned by user code).
- [x] **`Amalgame.Path`** (v0.4.11) — cross-platform path
      manipulation in `src/stdlib/path.am`: `Path.Combine`,
      `Path.Extension`, `Path.Filename`, `Path.Directory`,
      `Path.Stem`, `Path.IsAbsolute`, `Path.Normalize` (Go
      filepath.Clean semantics, no FS access), `Path.Sep`.
- [x] **`Amalgame.Logging`** (v0.4.12) — leveled stderr + optional
      file sink in `src/stdlib/logging.am` /
      `runtime/Amalgame_Logging.h`. `Log.SetMinLevel`,
      `Log.SetFile`, `Log.Debug/Info/Warn/Error`. Process-wide
      singleton state in the runtime. Structured logging (context
      fields, JSON-per-line) deferred to v2.
- [ ] **`Amalgame.Net.WebSocket`** — RFC 6455 client (and later
      server). The existing `TcpServer` could host it but the
      handshake + framing isn't trivial. Useful for amc lsp over
      websocket transport, bidirectional service comms, etc.
- [ ] **Filesystem watcher** — extend `Amalgame.IO` with
      `File.Watch(path, callback)` backed by `inotify` on Linux,
      `FSEvents` on macOS, `ReadDirectoryChangesW` on Windows.
      Useful for `amc test --watch`, dev-server hot reload, and
      generally anything dev-tools-shaped.
- [ ] **`Amalgame.Math` advanced** — vectors (`Vec2/3/4`), matrices
      (`Mat4`), complex numbers, bigint. Currently `Math.*` is
      scalar-only. Easy to start with `Vec3` for game/graphics
      use cases; `BigInt` is a bigger project (no GMP dep wanted).
- [ ] **Other serialization formats** — TOML (config files),
      YAML (CI configs), MessagePack (binary RPC). Json covers
      most needs but each has its niche.
      **Status**: **TOML subset shipped** in
      `src/stdlib/toml.am` (namespace `Amalgame.Formats.Toml`)
      to back `amalgame.toml` manifest parsing — TOML 1.0 minus
      dates/times, floats with exp, multiline strings, hex/oct/
      bin ints, dotted-key assignment. YAML + MessagePack
      still pending.
- [ ] **DateTime v2** — local time + named timezones. Adds a
      `LocalTime` companion class wrapping `(instant, zoneId)`
      with a `Now`, `In(zone)`, breakdown into Y/M/D/h/m/s,
      and `strftime`-ish formatter. Needs a way to ship tzdata:
      either bundle a stripped IANA dataset in the runtime, or
      delegate to the OS (POSIX `TZ` env + `/usr/share/zoneinfo`,
      Windows `GetDynamicTimeZoneInformation`). Also covers
      explicit `+HH:MM` offsets in Parse, currently rejected.
      Wait until a real consumer needs it — the v1 UTC API
      already covers most server-side use cases.
- [ ] **`Amalgame.UI` / Forms toolkit (cross-platform GUI)** —
      backs the `amc new <name> --template forms` scaffolder. SDL2
      binding under the hood (universally available on Linux/macOS/
      Windows, MIT-equivalent license, packageable via apt / brew /
      MSYS2). Public surface in two layers: a thin
      `Amalgame.UI.Window` / `Surface` / `Event` API that mirrors
      SDL's event loop, and a `Forms` layer above it
      (`Window`, `Button`, `TextField`, `Layout`, theming hooks) so
      everyday apps don't reach for raw event handling. Open design
      questions: retained vs immediate mode, accessibility surface
      (ATK / NSAccessibility / UIAutomation), how to ship the SDL
      runtime in release tarballs (link static? bundle the .so/dylib/
      DLL alongside the binary?). ~600 LoC stdlib + ~400 LoC runtime
      (mostly SDL passthrough).
- [x] **`Amalgame.Service` v1 — POSIX signals + Windows console**
      (PR #256, v0.4.13). `Service.Install` / `ShouldStop` /
      `RequestStop` / `Sleep`. POSIX `signal()` + `nanosleep()`;
      Windows `SetConsoleCtrlHandler` + chunked `Sleep`. Single
      surface across platforms; no SCM dispatch yet.
- [x] **`amc new --template service` v1 — Linux systemd + Windows NSSM**
      (PR #261, v0.4.14). Scaffolds a full daemon project:
      `src/main.am` with the canonical loop, `<name>.service`
      systemd unit + `install.sh` for Linux, `build.ps1` +
      `install.ps1` (NSSM-based) for Windows. README covers all
      three OSes; macOS install scripts are documented but not
      auto-generated (run the binary directly under launchd
      manually for now).
- [ ] **`Amalgame.Service` v2 — native Windows Service mode (SCM
      dispatcher).** Today the Windows path relies on NSSM
      (https://nssm.cc) to wrap the console binary as a service —
      operationally indistinguishable from a native service but
      requires the operator to install a small wrapper exe. v2
      ships the SCM dance inside the binary itself: at startup
      call `StartServiceCtrlDispatcher` with a static `ServiceMain`
      callback; if it returns FALSE with
      `ERROR_FAILED_SERVICE_CONTROLLER_CONNECT`, fall through to
      console mode (current behaviour). The `ServiceMain` callback
      calls `RegisterServiceCtrlHandler` for shutdown control codes
      and pumps `SetServiceStatus` so SCM sees the service as
      properly running. The user-loop side stays exactly the same
      (`while (!Service.ShouldStop()) { ... }`); the runtime hides
      the dual-mode plumbing.
      Implementation needs a worker-thread split (SCM dispatcher
      blocks the main thread; the user loop runs on a secondary
      thread, both observe the same `ShouldStop` atomic flag) plus
      an amc-side wrap of generated `main()` so the SCM bootstrap
      happens before the user's Amalgame `Main()` is invoked. The
      `--template service` scaffolder switches to native mode and
      drops the NSSM dependency; existing NSSM installs keep
      working since NSSM-managed services run the same binary
      either way.
- [ ] **`amc new --template service` v2 — macOS launchd plist.**
      The binary already runs cleanly on macOS via `./build.sh
      && ./<name>`; only the install scripting is missing. Ship
      a `launchd.plist` template + `install-macos.sh` wrapper
      that does `launchctl bootstrap gui/$(id -u) <plist>`.

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
- [x] **URL sweep** — old `BastienMOUGET/...` URLs scrubbed
      from `runtime/Amalgame_*.h`, `install/homebrew/amalgame.rb`,
      `install/windows/install.ps1` + `amalgame.iss` (post-PR #187 /
      org transfer).
      CHANGELOG mention of the transfer itself kept as historical
      record. The VS Code `publisher` field in
      `editors/vscode/package.json` is left at `BastienMOUGET`
      until the extension is actually published under the org
      identity on the marketplace.

### Marketing / discoverability
- [ ] **Submit Amalgame to GitHub Linguist** — until accepted,
      ` ```amalgame ` markdown fences render as plain text and
      `.am` files don't get a language badge on the repo. Linguist
      uses a stars/usage heuristic (~200 repos) and requires a
      TextMate grammar — we already have one in
      `editors/vscode/syntaxes/amalgame.tmLanguage.json`. Until we
      cross the threshold, README + docs/guide use ` ```kotlin `
      as a syntax-highlight fallback (closest visual match: shared
      `let` / `var` / `class` / `null` keywords, mismatches on
      `fn` and lowercase type names).

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
- **Error vs. exception model** — `try/catch/throw` works in
  self-host via setjmp/longjmp. Worth considering a Rust-like
  `Result<T, E>` plus `?` operator for short-circuiting as a
  complementary path.
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
