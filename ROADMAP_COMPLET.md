# Amalgame — Roadmap

> **Upcoming priority sequence (2026-05-14 decision):**
> `v0.7.4 = G (inline-C blocks)` ✅ shipped → `v0.7.5 = F (libamalgame pre-compile)` ✅ shipped → `v0.7.6 = stdlib purity + per-package facade pipeline + Math.h/Math_Vec.h migration` ✅ shipped → `v0.7.7 = framework split (5 new external facades + cgen 2-pass external forward decl)` ✅ shipped → `v0.7.8 = bundled-runtime trim (3 binding packages: regex/compress/net-websocket)` ✅ shipped → `v0.7.9 = amc build / run / watch` ✅ shipped → `v0.7.10 = LSP signatureHelp + full-signature hover` ✅ shipped → `v0.8.0 = DAP proxy (lldb-dap) + amc build --debug + cgen #line directives + VS Code extension v0.3.0 (amc debug type + DebugAdapterDescriptorFactory)` ✅ shipped → `v0.8.1 = amc new --vscode + tasks.json preLaunchTask + /proc/self/exe canonical path + scaffolds wrap amc build + XDG install layout (install.sh + amalgame.iss + release.yml all aligned on bin/ + share/amalgame/{runtime,lib,docs})` ✅ shipped → `v0.8.2 = batteries-included onboarding (gcc-bundle restored on Windows + VS Code .vsix auto-install on all 3 OS + Neovim/Helix LSP wiring + ~/Amalgame/samples/MyFirstApp scaffold) + cgen fix: Main(List<string> args) now receives a proper AmalgameList instead of the (code_string*)argv cast that left args as garbage in the debug pane` ✅ shipped → `v0.8.3 = peacock logo wired across README banner, Windows installer SetupIconFile, VS Code extension Marketplace icon` ✅ shipped → `v0.8.4 = hotfix dynamic ldd-based MinGW DLL resolution (v0.8.3 setup.exe missed libngtcp2_crypto_ossl-0.dll after MSYS2 rename — hardcoded list replaced with recursive closure walk)` ✅ shipped → `v0.8.5 = hotfix Windows .iss bypassed cmd.exe wrap for amc new scaffold call` ✅ shipped → `v0.8.6 = real fix for the same symptom: amc new itself shelled out to mkdir -p which Windows cmd treats as a literal -p dir name + ShellEscape single-quotes pollute the path; replaced with cross-platform File.Mkdir runtime helper `'MyFirstApp'` + stray `-p` directories — replaced cmd wrap with direct Inno Setup Exec + WorkingDir)` ✅ shipped → `v0.8.6+` gdb --dap fallback (Linux/Windows-MSYS2) + msgpack extraction (gated on cgen facade ABI fix) + LSP package discovery code action + macOS canonical path via _NSGetExecutablePath + Approche A migration (pretty-print AmalgameList*, filter runtime frames).
>
> **DAP strategy — decision 2026-05-13 (hybride, two-step):**
> 1. **v0.8.0 — Approche C (proxy mince + path translation)** : `amc dap` spawn `gdb --dap` (Linux + Windows-MSYS2, gdb ≥ 14) ou `lldb-dap` (macOS, Xcode CLT 14+) et forward DAP messages bidirectionnel. Les `#line N "foo.am"` directives émises par la cgen (à ajouter, prérequis bloquant) permettent à gdb/lldb de mapper natively `.c` ↔ `.am` via DWARF. **Effort : ~3-4h DAP + ~1-2h #line directives**. Pas de translation custom des types Amalgame — proxy minimal.
> 2. **v0.9.0+ — Migration vers Approche A (bridge MI custom)** : quand un use case réel demande du sucre Amalgame-spécifique (pretty-print `AmalgameList*`/`AmalgameMap*`, filtrer les frames runtime `Amalgame_*`/`_runtime.h`, prettifier les closures), on ajoute un mode bridge MI complet. `amc dap` parle DAP au client, gdb-MI au serveur, traduit les deux. **Effort : ~6-10h**. Le proxy C reste un fallback. **À NE PAS OUBLIER** : la dette « migrer vers A » est explicite, pas un nice-to-have ; à reprendre dès qu'un debug user-facing devient pénible avec le proxy.
> 3. **Rejetées** : (B) passthrough complet sans `amc dap` — laisse l'utilisateur exposé aux paths `.c` et complique launch.json ; (A direct) — duplique gratuitement le travail upstream gdb/lldb.
> No new `runtime/Amalgame_*.h` after v0.7.3 — every new stdlib module lands as a `.am` file
> using `@c { ... }` blocks for low-level glue. Migration cost of existing `.h` files stays
> bounded because we stopped adding to that pile in v0.7.3. See "Open design questions"
> for F details and "Runtime → AM migrations" for the rétro candidates.

> Updated 2026-05-14 · `amc 0.8.6` · self-hosted · 451/451 tests (amc) + 85/85 ecosystem · multi-OS CI · GitHub Releases automation · package manager + **13-package ecosystem** (math, math-vec, random, encoding, crypto, datetime, logging, service, io-filewatcher, yaml, regex, compress, net-websocket) · framework split (`libamalgame.a` 215 KB → 91 KB; 10 facade modules + 3 binding modules in external packages) · `amc build / run / watch` first-class compile verbs · `amc dap` DAP proxy (lldb-dap, gdb --dap pending v0.8.2) · `amc build --debug` (-O0 -g) + cgen `#line` directives → native `.am` breakpoints via DWARF · VS Code extension v0.3.0 (`amc` debug type + DebugAdapterDescriptorFactory + `amc new --vscode` opt-in scaffold) · `/proc/self/exe` canonical-path resolution → `amc build` works via PATH install · `build_amc.sh --install` opt-in user-bin layout · LSP signatureHelp + full-signature hover · C++ pipeline + precompile-on-install + calibration ETA + `search`/`versions`/`info`/`outdated`/`notice`/`check`/`suggest --json` with compat status + index cache TTL + auto-resolve add-without-tag + semver operators (^/~/>=/>/<=/</=) + `--version` with baked git rev + build date + ArgParser fluent framework + `--verbose` phase profiling + inline-C blocks (`@c { ... }`, `@c_include`, `@c_link`) + file-scope `@c { ... }` + per-package facade pipeline (`[stdlib].facade`)

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

- [ ] **Facade-package ABI bug — same-class static dispatch
      vs PkgClassMangledPrefix** (v0.7.8 known issue, blocks
      msgpack extraction in v0.8.0). When a package's `facade.am`
      calls its own static methods via `ClassName.X()` syntax
      (e.g. `MsgPack.DecodeOne(c)` inside `MsgPack.DecodeJson`):
      - `AddFilePass1` registers `MsgPack` in `LocalClasses`
      - `PackageRegistry.Load()` also adds it to `PkgClasses` (the
        running amc has its own package registered in
        `amalgame.lock` during the precompile-on-add step)
      - `EmitCalleeStr` checks `PkgClassMangledPrefix` **before**
        the `SymName` (LocalClass) fallback, so the call lowers to
        `Amalgame_Formats_MsgPack_DecodeOne` (namespace only,
        SQLite-style)
      - …but the function is **defined** as
        `Amalgame_Formats_MsgPack_MsgPack_DecodeOne` (namespace +
        class, facade-style)
      - gcc accepts the implicit-int declaration, sign-extends the
        return into a pointer, runtime segfault on first deref.
      - **Fix**: check `IsLocalClass(tname)` **before**
        `PkgClassMangledPrefix` in `EmitCalleeStr` (line 3515).
        Same precedence change needed in `TypeToC` (line 3793)
        for return-type / let-annotation paths if they hit the
        same conflict.
      - **Workaround until fixed**: facades use `this.X()` or
        private inline helpers; never `ClassName.X()` within the
        same class. The 5 facades shipped in v0.7.7 follow this
        accidentally, which is why they work.

### Cgen/typechecker bugs surfaced by amalgame-ui-forms (v0.0.3 → v0.0.5)

Six bugs hit while shipping ui-forms Widget/Form/Application/
Layout/CheckBox/RadioButton/TextBox/Panel. Each one has a local
workaround documented in-context (commit messages + facade.am
comments), so the package CI is green — but the cgen/typechecker
still needs the proper fix. Fix order can be opportunistic; none
of these block a release on its own.

- [ ] **Forward-decl ordering across class boundaries** —
      a method on class A that calls a method on class B,
      where B is declared **after** A in the same source file,
      emits `B_Foo(...)` without a forward declaration. gcc
      then treats it as `int B_Foo()`-implicit and bails on
      type conflict when the real definition is emitted later.
      Hit while wiring `Layout.Apply(Form)` calling
      `form.ChildCount()`. **Workaround**: drop the inter-
      class reference (Layout.Apply takes `List<Widget>`
      instead of `Form`). **Fix**: emit method forward decls
      in a single pre-class block at the top of the .c
      instead of grouping them per-class, or do a 2-pass
      emit (collect signatures, then write definitions).

- [x] **Chained method calls on cross-package types** —
      **fixed 2026-05-15**. Two-part fix in c_gen.am:
      `RegisterExternalProg` now registers each external method's
      return type via `MethodRetSet` (was emitting forward decls
      only, leaving `MethodRet*` tables empty for external classes).
      `InferTypeFromExpr`'s `ClassName.Method()` static-call path
      tries `ExternalClassMangled` before falling back to the
      consumer's `SymName` — `Page.New()` on an external `Page`
      now resolves to `Amalgame_UI_Web_Page*` instead of
      `App_Page*` (which had no MethodRet entries).
      Plus: `EmitExprStr` for inline `__lambda__` arguments
      no longer returns a `__lambda_…__` placeholder string;
      it emits a real `AmalgameClosure_new((void*)lam_N_fn, env)`
      compound statement expression. The previous placeholder
      survived only when wrapped by a Map/Filter dispatch that
      called `EmitClosureArg` first — every other call site
      (Bind, Element.OnClick, etc.) got garbage C tokens.
      Discovered while bootstrapping amalgame-ui-web v0.0.3's
      fluent builder (`Page.New().SetTitle(...).SetBody(...)`).

- [ ] **Field name == type name shadowing** —
      `public Layout: Layout` (field `Layout` of type
      `Layout`) generates correct struct emission, but
      assignments (`self->Layout = x`) and reads
      (`self->Layout`) silently no-op or hit the typedef
      rather than the struct member. Symptom in ui-forms
      v0.0.4: `SetLayout` ran but `ApplyLayout` saw a null
      LayoutRef, so children stayed at construction bounds.
      **Workaround**: rename the field
      (`Layout` → `LayoutRef`). **Fix**: cgen `EmitFieldRef`
      should mangle by struct-relative offset, not by name
      lookup that prefers the typedef.

- [x] **Parens lost on mixed `* + /`** — **fixed 2026-05-14**.
      cgen `EmitExprStr` BINARY branch now wraps any sub-BINARY
      operand in parens (belt-and-braces over-parenthesising;
      matches clang-format's pretty-print convention). Repro
      `let avail = (h - 2*pad - gap*(n-1)) / n` lowers correctly.
      421/421 tests still pass.

- [x] **`return null` rejected by typechecker for non-primitive
      return types** — **fixed 2026-05-14**. `IsAssignable`
      now accepts `null` for any target that isn't a primitive
      value type (int/float/bool/char/void). New helper
      `IsPrimitiveValue` enumerates the rejection set; everything
      else (classes, strings, List/Map/Set) takes null as the
      legitimate NULL sentinel. Drops the `@c { return NULL; }`
      workaround from facade.am files (see ui-forms cleanup pass).

- [ ] **`let` scope flattened to function level** — every
      `let` in a method body lowers as a top-of-function C
      declaration, so two `let x: T = ...` in *different*
      `if`/`while` blocks collide at the C level with
      `redefinition of x`. Hit during ui-forms v0.0.5 tests
      (RadioButton block declared r0, r2; Form.Resize block
      reused the same names). **Workaround**: rename locals
      across blocks. **Fix**: resolver should track block
      scope and let the cgen emit per-block C scopes
      (`{` … `}` around each block's locals).

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
- [x] **`while(ptr != null)` GC issue (resolved)** — minimal
      repro traversing a 3-node linked list with `while (cur !=
      null) { ...; cur = cur.Tail }` now counts correctly and
      doesn't get GC'd mid-loop. Was a cgen mis-detection rather
      than a GC bug: the typed local plus the `!=` against a
      typed `null` lowers cleanly to a `while (cur != NULL)`
      with the field deref bound to the locale's type. Existing
      `for i in 0..N` workarounds can be unwound where the
      sequence is unbounded; left in place where the bound is
      meaningful documentation.
- [ ] **Parser: `(a + b) % 256` ignores parens, parses as
      `a + (b % 256)`** — surfaced 2026-05-12 in `msgpack.am`.
      Repro: `let a = 500; let b = 65536; (a + b) % 256` returns
      `500` (= `a + (b % 256)` = `500 + 0`) instead of the
      expected `244` (= `66036 % 256`). The same expression
      assigned via an intermediate local works: `let sum = a +
      b; sum % 256` → `244`. Workaround in msgpack.am's ByteOf
      is to use the intermediate local. Likely the `%` operator
      binds tighter than `+` in `ParseExpr` and doesn't honour
      the paren grouping in the AST shape it returns. Repro fixture
      worth dropping into `tests/samples/` once we touch this.
- [x] **CGen: `<call>.Count()` on `AmalgameList*` lowered to
      `_Count` instead of `_count` (resolved v0.7.2)** — the
      chained-call dispatch in `EmitCalleeStr` (case
      `lk == NodeKind.CALL` ~ line 3337) used to emit
      `<bareR>_<mname>` verbatim. When the receiver was an
      `AmalgameList*` / `Map*` / `Set*` returned from another
      call (e.g. `jv.AsArray().Count()`), the PascalCase
      `Count` collided with the C runtime's camelCase
      `AmalgameList_count`. Fix downcases the first letter of
      `mname` when `bareR` is one of the three collection
      types. User classes still keep PascalCase methods.

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
- [x] **Extract repeated CGen helpers (resolved)** —
      `BoxAsVoid(expr)` and `UnboxScalar(ctype, expr)` live at
      the bottom of `src/generator/c_gen.am` and serve every
      site that previously open-coded the `(void*)(intptr_t)X`
      and `(T)(intptr_t)X` boxing dance. ~17 sites
      consolidated; remaining `intptr_t` mentions in the file
      are inside the helper bodies, in comments, or in the
      lambda-body emitter that already routes through the
      helpers. No code-quality regressions left here.
- [x] **Reduce `void*` erasure (v0.6.3)** — `xs.Get(i)` on a
      local `List<T>` / `Map<K,V>` now infers the element /
      value type directly from the `__local__` and
      `__local_map__` registry that `TrackGenericLocal` already
      populates at `let` / `var` declaration sites. Before:
      `let n = names.Get(0)` degraded to `void* n = ...`. After:
      `code_string n = (code_string)AmalgameList_get(names, 0)`.
      Drops the need for the `let n: string = ...` workaround at
      most call sites. Class-method return types were already
      propagating via `MethodRetTypes` since v0.5.x — the gap
      was specifically on the generic-collection accessors.
      Chained calls on user classes (`x.Foo().Bar().Baz()`)
      still propagate; verified via the new `ArgParser` fluent
      registration API.
- [x] **CGen: chained `obj.Field.Method()` /
      `obj.Method().Method()` (resolved)** — `EmitCalleeStr`
      now handles both shapes: when the receiver is a CALL, the
      inner return type is looked up via `InferTypeFromExpr` and
      the outer method lowers as `RetType_Method(<expr>, ...)`
      (c_gen.am ~3260); when the receiver is a MEMBER, the
      field's typed value flows through `TryEmitListCall` and
      the existing MEMBER → method-of-field-type path. Verified
      via repro `o.Field.Get()` (Inner_Get of o->Field) and
      `o.GetInner().Get()` (Inner_Get of App_Outer_GetInner(o)) —
      both produce valid C and the right runtime value. The
      intermediate-local workarounds in `lsp.am` / `migrate.am`
      could be inlined now but the indirection costs nothing
      and helps readability, so leaving them.
- [x] **Parser: `expr >> N` inside a `let` (resolved)** —
      `let x: int = r >> 8` now lowers correctly to a C
      `i64 x = r >> 8;`, repro from the original `Amalgame.Random`
      surfacing context passes. The shift operator is no longer
      dropped; verified via runtime repro returning the expected
      value. The `r / 256` workarounds in `Random.Bytes` /
      `Random.Float` could be reverted to `r >> 8` form now —
      cosmetic only, leaving them for a future cleanup pass.
- [x] **Parser: top-level free functions now reject with a
      clear diagnostic (resolved)** — the parser already caught
      `fn name(...)` at file scope; the TS/C-style equivalent
      `public List<int> Helper(int n) { ... }` slipped through
      and produced a call site against an undefined symbol
      (gcc `-Wimplicit-function-declaration`). `ParseDecl` now
      lookaheads up to 16 tokens for an `IDENT (` pair before
      any `{ / ; / }` punctuator and emits the same "Top-level
      functions aren't supported, hang it on a class" error as
      the `fn` form, then skips past the body. Verified via the
      original `public List<int> MakeBytes(int n) { ... }`
      repro — `amc --check` now reports "Top-level functions
      aren't supported (got 'MakeBytes' at 2:8). Wrap it inside
      a class as `public static`." instead of silently
      generating broken C. Full free-fn support stays out of
      scope (everything hangs on a class; matches the
      `Amalgame.Json` / `Amalgame.Path` facade pattern).
- [x] **CGen: constructor forward-decls (resolved)** — pass2
      now emits every `<Class>_new` signature in a forward-decl
      header block at the top of the C output, before any class
      body. Cross-class `new B(...)` calls in `A_new` therefore
      see the right signature instead of falling back to an
      implicit `int()` declaration. The `InstantResult`
      workaround in datetime.am could be reverted; left as-is
      since the explicit-parameter form is clearer regardless.
- [x] **Snapshot size — shrink C output (v0.6.4)** —
      `snapshot/amc_lib.c` shrunk from ~22 500 lines / 1.17 MB
      down to ~21 360 lines / 1.06 MB (-7%/-9%). The cgen no
      longer emits a `__attribute__((unused))` marker on every
      `VAR_DECL` (3342 occurrences gone) nor `(void)self;` /
      `(void)<param>;` boilerplate at the top of every method
      body (~2000 lines gone). The build adds
      `-Wno-unused-variable -Wno-unused-parameter
      -Wno-unused-but-set-variable` to the gcc invocations
      that ship the user binary — `amc --lint` is the
      canonical "is this variable actually used" gate now.
      The `.gitattributes merge=ours` half of this item
      shipped earlier so PR review noise is doubly cut.
- [x] **Profile compile time (v0.6.4)** — `amc --verbose` now
      prints per-phase timings on stderr at the end of a
      compile:
      ```
        parse:     154us
        resolve:   199us
        typecheck: 9us
        cgen:      411us
      ```
      Powered by a single `Stopwatch` (`Amalgame.DateTime`)
      that's `Reset()`'d at each phase boundary. Reveals the
      hot spot without external profiling. `./build_amc.sh`
      total wall time is now ~2s end-to-end (was ~5s when the
      item was written), so the proposed AST-pickling cache is
      no longer high-priority — would risk a serializer-bug
      class for marginal gain. Revisit if a single-file compile
      ever exceeds ~50ms on a representative project.
- [x] **Linter coverage (v0.6.3)** — `amc --lint` now also
      flags:
        - **catch-binder unused** — `try { ... } catch e { ... }`
          where `e` isn't read warns "unused local 'e' (prefix
          with '_' to silence)", same opt-out as other locals.
          Implemented by declaring the binder in a fresh scope
          around the catch body so the existing unused-local
          pass catches it.
        - **`var`-declared-but-never-reassigned** — flags
          declarations that should have been `let` for clarity.
          New `AssignedNames` append-only list (mirror of
          `UsedNames`) tracks bare-identifier LHS of every `=` /
          `+=` / `-=` / `*=` / `/=` / `%=` / `&=` / `|=` / `^=` /
          `<<=` / `>>=` operation; at `PopScope`, mutable locals
          with zero post-decl assignments warn.
      Still TBD as separate items (each needs typecheck
      integration or a non-trivial walk):
        - suspicious match (missing default + non-exhaustive enum)
        - implicit fallthrough
        - dead `import`
- [x] **`ArgParser` framework (v0.6.3)** — `src/argparser.am`
      ships a fluent registration class:
      ```
      let ap = new ArgParser()
      ap.Flag("-w").Flag("--write").Flag("-h").Flag("--help")
        .Option("--from")
        .Parse(argc, 2)
      if (ap.HelpRequested()) { ... }
      if (String_Length(ap.GetUnknown()) > 0) { unknown-flag err }
      let write = ap.HasFlag("-w") || ap.HasFlag("--write")
      let files = ap.GetPositionals()
      ```
      `RunFmt` migrated as the inaugural caller (~8 lines saved).
      Migration of `RunTest`, `migrate.am`, `generate.am`,
      `explain.am`, and the `add_cmd.am` per-verb loops is
      incremental — each subcommand can move to the framework
      independently. Lacks: short-flag clustering (`-vh`),
      `--key=value` form, `--` end-of-flags marker (add when a
      real subcommand needs them).
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

- [x] **Typechecker: spurious return-type mismatch in IF-body
      RETURN of enum members (resolved)** — a focused repro
      (Lexer.Classify returning `Tok.KW_IF` / `Tok.KW_LET` /
      `Tok.EOF` from three single-stmt IF bodies) now passes
      both `amc --check` and runtime. The NodeKey hash fix that
      landed alongside null-safety (`Kind` joining the
      `line:col:name:str` tuple in typechecker.am) cleared the
      slot collisions the original 37-case lexer.am report
      depended on. Investigation (2026-05-09): the spurious
      `got 'string'` for `return TokenType.KW_IF` was the
      surrounding `"if"` literal bleeding via the colliding
      NodeKey; with Kind now part of the key the leak path is
      closed.

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
- [x] **`amc build / run / watch`** (v0.7.9) — first-class
      compile verbs. `amc build [-o <out>] [-v] <entry.am>` runs
      amc + gcc-link in one step (factors the link logic out of
      `amc test`'s internal runner, plus links `lib/libamalgame.a`
      when shipped). `amc run [-o <out>] [-v] <entry.am>
      [-- args…]` chains a build with `Process.Run`; args after
      `--` pass through to the user binary's argv. `amc watch
      [-o <out>] [--run] [-v] <entry.am>` polls the entry's mtime
      every 500 ms (vendored `@c {}` block calling `stat()` /
      `_stat64`) and rebuilds on change. The bare-args form
      `amc foo.am -o foo` still works (no gcc step) for users
      who want to splice their own gcc command. Transitive-import
      watching is deferred until the `FileWatcher` package gains
      an event-based mode (post-D).
- [ ] **Unify all test runners under `amc test`** — the repo
      still ships ~1.9k lines of bash in `tests/run_tests.sh`
      (992), `tests/run_stdlib_tests.sh` (607), `tests/run_fmt_tests.sh`
      (132), `tests/run_amc_new_tests.sh` (143), and `tests/run_all_tests.sh`
      (57). Each implements its own discovery + compile + capture
      + tally loop in shell, with subtle differences (some pass
      `--lib`, some assert expected stdout, fmt runner round-trips
      via the formatter, amc-new runner shells out to scaffold + build).
      Goal: rewrite every check as `*_test.am` files emitting
      `[PASS]/[FAIL]/[SKIP]` lines so `amc test ./tests/` drives
      the whole suite, then drop the bash. Likely needs `amc test`
      additions: per-file env (`AMC_FLAGS`), expected-stdout
      assertions (or move them inside the test bodies), parallel
      execution, a `--filter <glob>` flag, and a `--ci` output
      mode matching the current bash tally. Also: the fmt and
      amc-new runners exercise tooling other than the compiler
      (formatter idempotency, project scaffolding) — those need
      either dedicated `Amalgame.Test` helpers (e.g.
      `Test.Format(file)`, `Test.Scaffold("exe", "/tmp/x")`) or a
      runner mode that shells out and captures. Big win: one
      test entry point, one runtime, runs on every platform amc
      compiles for (today the bash runners assume POSIX).
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
- [x] **`amc package <action>`** (v0.5.0 → v0.6.x) — full package
      manager. `add <git-url>@<tag>` clones + validates + records,
      `remove` / `list` / `search` / `versions` / `info` / `update`
      / `cache` round out the CLI (PR #303 grouped them under
      `amc package`, alias `amc pkg`). Storage at
      `~/.amalgame/packages/<host>/<owner>/<repo>/<tag>_<sha>/`.
      `amalgame.toml` (deps) + `amalgame.lock` (resolved SHAs)
      live in the project root. `amc test` is package-aware:
      auto-installs missing deps (v0.5.1) and links each
      package's `[stdlib].sources` `.c` files into every test
      binary (v0.5.2). Subsequent maturity work:
        - **v0.5.3** — C++ packages pipeline, vendored amalgamation
          builds, `cflags` / `cxxflags` / `libs` manifest fields.
        - **v0.5.4** — `precompile-on-install` with persistent
          `~/.amalgame/packages/.../build/<platform>/` cache, an
          auto-learning ETA derived from `~/.amalgame/calibration.toml`,
          and cross-platform `$HOME` resolution
          (`PackageRegistry.AmalgameHome()` walks
          `$AMALGAME_HOME` → `$HOME` → `$USERPROFILE`).
        - **v0.5.5** — packages-index schema v2 (flat `[[version]]`
          array), `amc package search` + `amc package versions`
          render every indexed tag with `✓` / `✗` compat status
          against the running amc, `← latest compatible` marker,
          `--refresh` flag to bust the cache, `LoadedPackage.Tag`
          displayed by `amc package list`, `@<tag>` safety suffix
          for `remove`.
        - **v0.5.6** — 30-min TTL on `~/.amalgame/cache/packages-index.toml`
          (via `date -r`, POSIX + MSYS2 + Cygwin), serves stale
          cache with a warning on network failure, downstream
          Redis + MQTT test runners ported to the SQLite/DuckDB
          symlink-trick (their CI had been silently SKIPping).
        - **v0.6.0** — auto-resolve `amc package add <pkg>` (no
          `@<tag>`) walks the index newest-last and picks the latest
          compatible tag; `required-amalgame` learns five new
          semver operators on top of `>=`: `>`, `<`, `<=`, `=`,
          `^` (caret, npm/Cargo flavour with 0.x special-case),
          `~` (tilde, locks major.minor).
        - **v0.6.1** — bundled QoL + ship-to-prod release:
          `amc package info <name>` (description, url, tier,
          license, category, maintainer, versions, install
          status); `amc package outdated` (cross-references the
          lockfile against the index, lists deps with a newer
          compatible tag); `amc package notice` (aggregates each
          installed package's `[package].license / authors /
          description` into a NOTICE-style listing on stdout,
          ready to redirect into `NOTICE_DEPS.md` for downstream
          commercial redistribution); `amc package check
          [--frozen]` (verifies amalgame.lock matches the
          installed cache — `--frozen` exits 1 on mismatch for
          CI fail-fast lanes, bare form is informational and
          always exits 0); `--no-versions` on `search` for faster
          browse; `--json` on `versions` for scripting (jq / CI
          compat probes, stable schema). Plus: `amc --version`
          now bakes the git short-SHA + UTC build timestamp via
          `-DAMC_GIT_REV=...` / `-DAMC_BUILD_DATE=...`
          (`build_amc.sh` wires both; fall back to "" cleanly
          when the defines are absent), and surfaces author /
          licence / website / repository / issues URLs in the
          banner — debugging an ambiguous binary is one
          `amc --version` away. Help-text audit pass:
          `add --help` documents the shortname auto-resolve form
          + `--no-precompile` + the full semver operator set; the
          top-level `amc --help` verb list adds `versions / info
          / outdated / notice / check`.
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
- [ ] **Editor integration on install + Windows MSI batteries-
      included (priority for v1.0 readiness)** — when a user
      installs Amalgame (`install.sh`, future `amc-up` package
      script, Homebrew formula, `.deb`/`.rpm`, **Windows
      `.msi`**), automatically wire the LSP into the editors
      present on the host AND ship the framework so the install
      is "download → write code immediately":
        - **VS Code / VS Code Insiders / VSCodium**: detect via
          `code --list-extensions`; if missing, install
          `editors/vscode/` from the local `.vsix` bundled in
          the release tarball, or publish to the Marketplace and
          install by ID. Set `amalgame.serverPath` to the resolved
          `amc` binary so the extension doesn't depend on `$PATH`.
          **The MSI on Windows is the prime carrier** — bundles
          MinGW gcc + libgc + libcurl + the VS Code `.vsix` +
          `libamalgame.a` (post-F) + the `amc.exe` itself.
          Post-install handler auto-installs the extension and
          opens VS Code on a "Hello, Amalgame" sample. Goal:
          single `.msi` download → working compiler + LSP in
          ~30 s on a fresh Windows box.
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
      since v0.4.0) so this works air-gapped. Pairs with the
      bundled-stdlib item (post-F) — both ship in the same
      installer payload. Open question: auto-detect editors vs.
      interactive prompt — preference is opt-out (auto-detect by
      default; `--no-editors` skips).
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
      - [x] `Amalgame.Regex` (v0.7.1) — POSIX extended-regex
        binding in `runtime/Amalgame_Regex.h` (no third-party
        PCRE / RE2 dep). Surface: `Regex.Test(pat, subj)`,
        `Regex.Match(pat, subj)` → `Match*` with `GetText` /
        `GetStart` / `GetEnd` / `GroupCount` / `GroupText(i)` /
        `GroupStart(i)` / `GroupEnd(i)`, `Regex.Replace(pat,
        subj, repl)`, `Regex.ReplaceAll(...)` (zero-length-
        match safe). PCRE-only features (`\d` / `\w` / look-
        arounds / non-greedy / named captures / Unicode property
        classes) are out of scope — bind libpcre2 in a future
        package when a real consumer needs them. 11 stdlib
        tests cover predicate / match / captures / replace /
        anchors / alternation.
      - [x] `Amalgame.Compress` (v0.7.2) — zlib binding. Gzip /
        Gunzip (RFC 1952 wrapper) for `.gz` files / HTTP
        `Content-Encoding: gzip`, plus Deflate / Inflate (raw
        RFC 1951) for embedded protocols. Input + output as
        `List<int>` byte buffers; GzipString / GunzipString
        helpers for UTF-8 strings. 9 stdlib tests cover empty
        input, magic-byte verification, large input, and round-
        trips. Zip archive support stays deferred (different
        scope — central directory + per-file headers).
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
      **v0.6.1** lifts the `import Amalgame.Path` requirement
      for `Combine` / `Sep` / `IsAbsolute` / `Normalize` (those
      four method names match their runtime symbol 1:1, so the
      cgen lowers `Path.X(...)` straight to `Path_X(...)`).
      `Directory` / `Filename` / `Extension` / `Stem` still need
      the import — they wrap `Path_Get*` and would mis-mangle
      without the namespace dispatch.
- [x] **`Amalgame.Logging`** (v0.4.12) — leveled stderr + optional
      file sink in `src/stdlib/logging.am` /
      `runtime/Amalgame_Logging.h`. `Log.SetMinLevel`,
      `Log.SetFile`, `Log.Debug/Info/Warn/Error`. Process-wide
      singleton state in the runtime. Structured logging (context
      fields, JSON-per-line) deferred to v2.
- [x] **`Amalgame.Net.WebSocket` — RFC 6455 client (v0.7.3)** —
      `runtime/Amalgame_WebSocket.h` ships: TCP connect, HTTP
      upgrade handshake, SHA-1 Sec-WebSocket-Accept derivation,
      Base64 encoder, frame parser with mask/unmask, auto-Pong
      reply to Ping, Close handshake. API:
      `WebSocket.Connect(host, port, path) → WebSocket?`,
      `ws.SendText(s) → bool`, `ws.ReceiveText() → string?`,
      `ws.Close()`, `ws.IsConnected()`. Helper exposed:
      `WebSocket.AcceptKey(clientKey)` for independent
      handshake verification.
      4 stdlib tests cover the RFC 6455 §1.3 canonical
      Sec-WebSocket-Accept test vector
      (`dGhlIHNhbXBsZSBub25jZQ==` →
      `s3pPLMBiTxaQ9kYGzzhZRbK+xOo=`), empty-input edge case,
      and error paths (refused TCP + DNS failure). Live-server
      integration test deferred — a Python `websockets`
      stand-in is the planned harness, opt-in for hosts that
      have the module.
      Out of scope (next iterations):
        - `wss://` TLS (TcpTls or OpenSSL binding)
        - Binary opcodes (0x2)
        - Continuation frames (multi-fragment messages)
        - per-message-deflate negotiation
        - HTTP subprotocols (`Sec-WebSocket-Protocol`)
- [x] **Filesystem watcher — v1 single-file polling (v0.7.0)** —
      `Amalgame.IO.FileWatcher` watches one file via `stat(2)`
      mtime polling. Cross-platform (POSIX + Windows via
      `_stat64`). API: `new FileWatcher(path)`, `w.Exists()`,
      `w.Changed()` (true the first call after the mtime advances
      or the file appears/disappears; false afterwards until the
      next change), `w.GetPath()`. Tests in
      `tests/samples/stdlib_file_watch.am` use the delete /
      re-create flip for deterministic results (mtime-advance is
      hard to test reliably across filesystems). Covers the
      "reload a config file" / "rebuild on source change" 80%
      use case.
- [ ] **FileWatcher v2 — events + DirectoryWatcher + inotify** —
      v1 is intentionally minimal; v2 fleshes it out:
        - **Event types** — instead of a boolean `Changed()`,
          expose `WatchEvent` records with `Kind`
          (`Created` / `Modified` / `Deleted` / `Renamed`),
          `Path`, `RenamedTo` (for Renamed only), `Timestamp`.
          Polling backend infers Kind from the mtime/size/exist
          flip; the platform-native backends below get the Kind
          straight from the kernel.
        - **DirectoryWatcher** — `new DirectoryWatcher(path,
          recursive: bool)` returns events for every file in
          the dir (or subtree). Polling backend walks the dir
          and diffs the file-list snapshot; native backends
          subscribe at the dir level.
        - **Native backends** — `inotify` on Linux, `FSEvents`
          on macOS, `ReadDirectoryChangesW` on Windows. The
          polling backend stays as a portable fallback (and
          the test default — exact, no kernel queue to drain).
          Surface stays the same `WatchEvent` shape so user
          code doesn't branch on platform.
        - **Use cases unblocked** — `amc build --watch` (post-C),
          `amc test --watch`, dev-server hot reload, config-
          file reload across a whole dir, log tail tools.
      Estimated ~1 day for the event-typed polling backend,
      another ~1.5 days for the three native backends. Worth
      splitting into two PRs (v2-events then v2-native) so
      `amc build --watch` lands as soon as the events shape is
      stable.
- [x] **`Amalgame.Math` advanced — Vec3/Vec4/Mat4 (v0.7.0)** —
      `Amalgame.Math.Vec` ships scalar (no SIMD) implementations
      of `Vec3` (Add/Sub/Scale/Dot/Cross/Length/Normalize/Equals
      + GetX/Y/Z), `Vec4` (Add/Sub/Scale/Dot + GetX/Y/Z/W), and
      `Mat4` (Identity/Translate/Scale/RotateX/Y/Z/Multiply/
      TransformVec4/Get/Set). Matrices are 4×4 column-major
      (OpenGL convention) so `glUniformMatrix4fv` works without
      transposition. All operations heap-allocate via GC_MALLOC
      so chained calls don't alias their inputs. 16 stdlib tests
      cover every method. Complex numbers + BigInt deferred —
      different concerns (no GMP dep wanted), revisit when a
      real consumer needs them.
- [x] **Other serialization formats — TOML + YAML (v0.7.1)** —
      Two of three covered:
        - **TOML** subset (v0.5.x) — `src/stdlib/toml.am`
          (`Amalgame.Formats.Toml`), backs `amalgame.toml`
          manifest parsing.
        - **YAML 1.2** subset (v0.7.1) — `src/stdlib/yaml.am`
          (`Amalgame.Formats.Yaml`). Block mappings, block
          sequences, scalars (bool/int/float/quoted/plain),
          comments. Out of scope: anchors/aliases, multi-doc
          `---`, flow style `[1,2]`, multiline scalars
          (folded/literal), tags. 19 stdlib tests cover the
          shapes a typical CI / app config exercises.
      - **MessagePack 1.0 subset** (v0.7.2) — pure-Amalgame
        codec on top of JsonValue. `MsgPack.EncodeJson(jv)` →
        `List<int>`; `MsgPack.DecodeJson(bytes)` → `JsonValue`.
        Coverage: nil, bool, fixint (positive + negative), int8/
        int16/int32, fixstr / str8 / str16, fixarray / array16,
        fixmap / map16. Out of scope (v2): int64 / float / bin /
        ext / timestamps. 14 stdlib tests cover encode + decode
        round-trips. Round-trips through Json mean any code that
        builds JsonValue trees can switch to MsgPack with a
        one-line rename.
- [x] **DateTime v2 — UTC breakdown (v0.7.1)** — Instant now
      exposes `Year()` / `Month()` / `Day()` / `Hour()` /
      `Minute()` / `Second()` accessors that decompose the
      nanosecond-since-epoch value into UTC calendar fields via
      `gmtime_r` / `gmtime_s`. Covers the "format my timestamp
      to YYYY-MM-DD HH:MM:SS in arbitrary order" use case
      without an external library. 13 stdlib tests cover the
      epoch, a known 2026-05-12 timestamp, the famous
      `1234567890` Unix moment, and a Format/Parse round-trip.

      **Deferred — named timezones**: `LocalTime` companion
      class wrapping `(instant, zoneId)`, `In(zone)` method,
      `strftime`-ish formatter, `+HH:MM` offset parsing. Needs
      a tzdata shipping strategy (bundle IANA vs. OS delegate
      via POSIX `TZ` + `/usr/share/zoneinfo` / Windows
      `GetDynamicTimeZoneInformation`). Picks up when a real
      consumer needs it — server-side UTC + the breakdown above
      cover most cases.
- [x] **`Amalgame.UI` / Forms toolkit (cross-platform GUI)** —
      **shipped 2026-05-14, sunset 2026-05-15** as two external packages:

      > **⚠ Sunset 2026-05-15:** [`amalgame-ui-forms`](https://github.com/amalgame-lang/amalgame-ui-forms)
      > and the never-published `amalgame-ui-tk` exploration
      > have been superseded by
      > [`amalgame-ui-web`](https://github.com/amalgame-lang/amalgame-ui-web)
      > **v0.0.3+** (webview-based, renders HTML/CSS/JS in the
      > OS-native engine — WebView2 / WKWebView / WebKitGTK).
      > The decision matrix lives in
      > [`docs/proposals/amalgame-ui-web.md`](docs/proposals/amalgame-ui-web.md);
      > TL;DR: SDL retained-mode means re-implementing every widget
      > by hand (rounded corners, native fonts, OS theming, HiDPI, …)
      > with zero leverage from the OS — webview gets all of that
      > for free, and it's what the rest of the desktop ecosystem
      > (VS Code, Slack, Discord, Tauri) settled on years ago.
      >
      > `amalgame-ui-sdl` stays in the package index as the
      > foundation for any future `amalgame-gfx` package that
      > covers games / 3D / real-time visualization — those
      > use cases need a custom-rendered surface, not a
      > webview. Both `ui-sdl` v0.1.0 and `ui-forms` v0.1.4
      > stay listed for existing consumers; no further
      > releases planned on `ui-forms`.

      - `amalgame-ui-sdl` **v0.1.0** — thin SDL2/SDL3 binding.
        Surface: `Window` (create/close/title/resize), `Event`
        (Quit/MouseDown/MouseUp/MouseMove/KeyDown/KeyUp/
        WindowResize), `Surface` (Clear/Present/FillRect/
        DrawRect/DrawLine/DrawPixel), `Font` (LoadDefault
        cross-OS probe, DrawText, MeasureWidth), `Color`,
        `Rect`, `OSTheme.DetectOS()` (macOS `defaults` /
        Windows registry / Linux `gsettings color-scheme`).
        Backend chosen at compile time via
        `-DAMALGAME_UI_USE_SDL3` (default SDL2).

      - `amalgame-ui-forms` **v0.1.0** — retained-mode GUI
        toolkit on top of ui-sdl. Single concrete `Widget`
        class with a `Kind` tag (class-with-tag pattern
        because amc 0.8.x's typechecker rejects subclass
        upcasts), 9 kinds: Label, Button, CheckBox,
        RadioButton (+ Group exclusivity), TextBox (focus +
        printable-ASCII typing + backspace), Panel, ListBox,
        ComboBox, MenuBar. `Form` container with 4 layouts
        (StackVertical, StackHorizontal, Grid, Absolute).
        Theme.Light/Dark/FromOS palette. `Application.Run`
        blocking event loop with mouse-down hit-testing,
        keyboard focus dispatch, layout repack on resize.

      `amc new <name> --template forms` scaffolds a ready-to-
      build sample app (Form + Label + Button + StackVertical
      + `Application.Run`) wired against both packages.
      Requires amc 0.8.7+ (cross-package facade deps fix in
      `amc --lib`) and SDL2 dev headers on the build host
      (apt/brew/pacman one-liner in the scaffolded README).

      Deferred (small): accessibility (ATK / NSAccessibility /
      UIAutomation), HiDPI scaling factor probe, static-link
      SDL build variant, font fallback chain for non-Latin
      scripts.

      **GUI future scope (long-term, not on any release):**

      - **2D drawing primitives expansion** — current `Surface`
        covers FillRect / DrawRect / DrawLine / DrawPixel.
        Add: rotated/scaled blits, alpha-blended fills,
        gradients (linear/radial), polylines, polygons,
        Bezier curves, image loading (`Surface.LoadFromFile`
        for PNG/JPG via SDL_image). Path: extend
        `runtime/Amalgame_UI.h` + facade `Surface` class in
        ui-sdl; or split into `amalgame-ui-gfx2d` if API grows
        large.

      - **3D graphics binding** — separate package
        `amalgame-ui-gl` exposing OpenGL 3.3 core (Window
        becomes a GL context provider, new `GLProgram`,
        `GLBuffer`, `GLTexture`, `GLVertexArray` classes).
        Considered: Vulkan binding `amalgame-ui-vk`, but only
        once OpenGL surface stabilizes — Vulkan's verbosity
        argues for a higher-level wrapper than 1:1 binding.
        ui-sdl gains a `Window.GLAttachContext()` helper.

      - **WYSIWYG VS Code form designer** — visual canvas
        extension (TypeScript inside `amalgame-vscode`) that
        renders a Form preview from `.am` source, lets the
        user drag widgets onto a grid + edit properties in a
        side panel, then writes the changes back to the source
        as `Widget` constructor calls + Layout decisions. Round-
        trips: parse the `.am` via amc's LSP (already shipped),
        edit the AST visually, regenerate the relevant Form
        constructor body. Pattern: Visual Studio's Windows
        Forms designer or Qt Creator's `.ui` files (but
        Amalgame's source is the canonical form, no separate
        `.ui` file). Requires: stable widget set (✓ shipped),
        LSP form-node detection (TBD), and a reliable
        re-serialization story (the hard part — preserving
        user-written code around generated regions).

      These three items are tracked here so they don't drop
      out of context, but are explicitly **not** on the
      v0.8.x → v0.9.x roadmap. Pick them up after the
      remaining 4 cgen bugs from ui-forms close and a real
      external consumer asks for 2D/3D primitives.
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
- [ ] **Replace external-package CIs with `release.sh` scripts** —
      the package repos (`amalgame-database-sqlite` /
      `amalgame-database-nosql-redis` / `amalgame-database-duckdb` /
      `amalgame-messaging-mqtt`) currently run their CI via GitHub
      Actions, which authenticate against a `GH_TOKEN` that expires
      after 365 days. The token rotation is easy to forget and a
      missed renewal stalls every package release without warning.
      Plan: replace each `.github/workflows/ci.yml` with a local
      `release.sh` the maintainer runs from a checked-out clone —
      same test matrix, same artifact build, push tags + create
      GitHub Releases via `gh` CLI (which uses the maintainer's
      own auth, no shared token). Tradeoff: PRs from contributors
      lose automatic CI feedback — accept it for the small package
      repos (single-maintainer cadence), keep GH Actions on the
      main `Amalgame` compiler repo where contributor velocity
      matters.
- [ ] Homebrew tap (formula draft in `install/homebrew/amalgame.rb`)
- [ ] Homebrew core (after public adoption)
- [ ] AUR / `.deb` / `.rpm` / Nix flake / winget / Scoop
- [ ] `install.sh` universal one-liner — installs amc binary +
      bundled stdlib `libamalgame.a` (post-F) + auto-detects
      editors and wires the LSP up (post-H).
- [ ] **Windows packaged installer (.msi)** — bundled MinGW gcc +
      libgc + libcurl so end users don't need MSYS2. Also ships
      the VS Code extension `.vsix` and auto-installs it, sets
      `amalgame.serverPath` to the resolved `amc.exe`, and drops
      the bundled `libamalgame.a` (post-F) alongside. Goal: a
      single `.msi` download → working compiler + LSP-equipped
      VS Code immediately. Sketched in conversation; no script
      yet. Pairs with the editor-integration roadmap entry below.
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

## Runtime → AM migrations (project G follow-up, optional)

Project G (v0.7.4) shipped `@c { … }` + `@c_include` + `@c_link`
and migrated **1 POC**: `runtime/Amalgame_BuildInfo.h` →
`src/stdlib/amc_buildinfo.am`. The remaining `runtime/Amalgame_*.h`
headers stay in C unless someone actively rétro-migrates them.
This is a backlog of the ones that *could* migrate once project F
(`libamalgame.a` pre-compile, v0.7.5) lands and makes the
ergonomics painless for users.

Candidates ranked by migration value × low risk:

- [ ] **`Amalgame_DateTime.h`** — small, mostly wraps `clock()` /
      `time()` / `gmtime_r()`. ~120 LoC of glue → AM with 1-2 `@c {}`
      blocks per helper. Good warm-up candidate.
- [ ] **`Amalgame_Logging.h`** — log levels + optional file output.
      No third-party dep. ~80 LoC C, trivial to migrate.
- [ ] **`Amalgame_Crypto.h`** — SHA-256 + HMAC-SHA-256, pure-C
      implementation already (no OpenSSL). ~300 LoC but mechanical;
      the bit-twiddling stays inside one big `@c {}` per primitive.
- [ ] **`Amalgame_Random.h`** — PCG-32 + `getentropy` / `BCryptGenRandom`
      wrapper. The PCG step could even move to pure Amalgame (the
      `src/stdlib/random.am` facade already does most of it); only
      the OS-entropy syscall needs `@c_include` + `@c {}`.
- [ ] **`Amalgame_Service.h`** — daemon primitives (POSIX fork/setsid +
      Windows SCM stub). Touches `@c_include "<unistd.h>"` /
      `"<windows.h>"`; manageable.
- [ ] **`Amalgame_FileWatch.h`** — inotify (Linux) / FSEvents (macOS) /
      ReadDirectoryChangesW (Windows) bindings. Useful real-world test
      of `@c_include` cross-platform conditional compilation.

Non-candidates (stay in C):

- `_runtime.h` — foundation (GC roots, exception model, `code_string`).
  Bootstrap-critical, can't be migrated.
- `Amalgame_String.h` / `Amalgame_Collections.h` — perf-critical
  primitives that the compiler itself uses heavily. Migration would
  add a function-call boundary per `String_Length` / `List.Get` and
  show up in `amc` self-host timings.
- `Amalgame_Net.h` — libcurl + winsock2 bindings, ~600 LoC. Possible
  but the C body is mostly libcurl boilerplate; the migration gain is
  small relative to the churn risk.
- `Amalgame_Compress.h` — zlib bindings, ditto.
- `Amalgame_Regex.h` — POSIX regex bindings, ditto.

Each migration is its own PR (small, isolated, easy to revert) and
should include a benchmark snapshot in the description so we catch
the rare case where `@c {}` boundary costs hurt a hot path.

---

## Project F follow-up — per-package `libamalgame-pkg-<name>.a`

Project F (v0.7.5) shipped `lib/libamalgame.a` for the **integrated**
user-facing stdlib. The same `--external` mechanism could extend to
**external packages** (`amc package add sqlite/duckdb/redis/mqtt`)
that ship an AM facade alongside their C runtime.

Status today (post-v0.7.5):

- **Integrated stdlib** (random, encoding, json, …) — pre-compiled
  in `lib/libamalgame.a`, linked via `--external`. ✅
- **External packages — C runtime** — already pre-compiled at
  `amc package add` time (v0.5.4, `[stdlib].precompile = true` in
  the manifest), cached at
  `~/.amalgame/packages/<host>/<owner>/<repo>/<tag>_<sha>/build/<platform>/`. ✅
- **External packages — AM facade** — re-parsed on every `amc -o`
  in the user's project, even when the package author shipped a
  thin facade `.am` next to the runtime header. ❌

Sketch of the extension:

- [ ] **`tools/build-package-stdlib.sh`** (or extend
      `tools/build-stdlib.sh` with a `--package <slug>` flag) —
      compile the package's facade `.am` into
      `~/.amalgame/packages/<…>/build/<platform>/libamalgame-pkg-<slug>.a`
      at `amc package add` time, alongside the existing runtime-`.o`
      cache. Reuse the same `amc --lib --quiet -o … && gcc -c &&
      ar rcs` shape.
- [ ] **Manifest** — opt-in via `[stdlib].facade = "facade.am"` in
      `amalgame.toml`, mirroring the existing `[stdlib].sources` /
      `precompile` flags.
- [ ] **`amc test` / `amc -o`** — when a registered package has a
      pre-compiled facade lib, auto-pass `--external <package>/facade.am`
      and link against the per-package archive (same gcc step that
      already pulls in the runtime `.o` cache).
- [ ] **Resolver / cgen** — no change needed: the `--external`
      pipeline shipped in v0.7.5 already routes inter-namespace
      mangling correctly.

Estimated ~1 day. Becomes worth doing as soon as a community package
ships a facade that's expensive enough to re-parse to make it visible
in `amc -o` timings (today's 4 official packages — SQLite, Redis,
MQTT, DuckDB — are mostly thin C bindings, so the gain is small).

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
  is passed by hand at link time. The *current concrete pain*:
  every `amc -o foo foo.am` re-parses + re-compiles every
  stdlib `.am` (path.am, datetime.am, yaml.am, math_vec.am,
  argparser.am, …) even when foo.am uses one of them. With ~10
  stdlib modules in v0.7.1 and growing, that's a measurable
  fraction of compile time and 100% wasted work.

  Alternatives to weigh:
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
    - **E. Hybrid — primitives inline, user-facing modules
      pre-compiled.** Keep `String_Length` & co. header-only
      (they're needed during the compiler bootstrap so must
      stay parsable by gcc alone). Pre-compile every
      user-facing `.am` (path / datetime / yaml / regex /
      math_vec / argparser / json / random / encoding / crypto
      / logging / service) into a `libamalgame.a` bundled with
      the amc binary; the gcc invocation for user code links it
      automatically and skips re-parsing. Bootstrap unchanged:
      `build_amc.sh` continues to ingest the .am sources to
      build amc itself; the **bundled** stdlib lib is rebuilt
      as a post-step from the same sources.
  Bootstrap-curiosity → A is fine. Daily driver → E (chosen
  direction per user request, see action item below); D is
  complementary and can layer on top.

- [ ] **Pre-compiled user-facing stdlib (option E above)** —
      concrete action item. `tools/build-stdlib.sh` (or a
      `build_amc.sh` post-step) compiles every user-facing
      `src/stdlib/*.am` into a single `libamalgame.a` shipped
      alongside the `amc` binary. `amc -o foo foo.am` discovers
      the lib via `$AMALGAME_HOME/lib/libamalgame.a` (or
      `<install_prefix>/lib/`) and links against it instead of
      re-parsing the `.am` sources. Per-platform builds tracked
      by the release workflow. `import Amalgame.Path` becomes
      a *real* directive — it tells the resolver which symbols
      to pull from the lib's symbol catalogue. The compiler-
      internal `.am` files (lexer / parser / cgen / typechecker /
      …) stay in the bootstrap pipeline; only the user-facing
      facades migrate. Estimated 2–3 days: build script + the
      "imports are physical" half of the module system. Pairs
      with the F-bundled-with-installer item in Distribution
      and the H-Windows-MSI installer below.
- **Error vs. exception model** — `try/catch/throw` works in
  self-host via setjmp/longjmp. Worth considering a Rust-like
  `Result<T, E>` plus `?` operator for short-circuiting as a
  complementary path.
- **Inline-C injection blocks** — proposal raised 2026-05-12.
  ✅ **SHIPPED in v0.7.4 (project G)** — commits `776bc74`
  (phase 1+2: lexer/parser/cgen MVP + BuildInfo POC migration)
  and `0c75802` (phase 3: `@c_include` + `@c_link` directives).
  `@out = expr;` was NOT implemented — the body just uses a plain
  C `return …;` against the enclosing method's declared type, which
  turned out cleaner. Migration backlog for the remaining runtime
  headers is tracked in the "Runtime → AM migrations" section
  above. The original proposal text below is kept for context.

  Today the runtime is split between hand-written C
  (`runtime/Amalgame_*.h`, ~3 000 LoC of POSIX + libc calls)
  and Amalgame facades (`src/stdlib/*.am`). The C half is
  necessary because Amalgame has no syntax to call `fopen` /
  `curl_easy_perform` / `inotify_init1` / `regcomp` directly.
  Goal: add a balisé inline-C block to Amalgame so the C-side
  half can also be expressed in `.am` files:

  ```
  public class Path {
      public static string Combine(a: string, b: string) {
          @c {
              // Variables typed from the Amalgame signature
              // flow in transparently; the @out value flows
              // back. Same GC root rules as user code.
              size_t la = strlen(a), lb = strlen(b);
              char* r = (char*) GC_MALLOC(la + lb + 2);
              memcpy(r, a, la);
              if (la > 0 && a[la - 1] != '/') { r[la++] = '/'; }
              memcpy(r + la, b, lb);
              r[la + lb] = '\0';
              @out = r;
          }
      }
  }
  ```

  Semantics:
    - **Variables traverse the boundary both ways.** Every
      Amalgame local in scope is visible inside `@c { ... }`
      with its C representation (`code_string` / `i64` / struct
      pointer). `@out = expr;` (one assignment per block)
      provides the return value to the surrounding Amalgame
      method.
    - **Type checker treats the block as opaque** — return
      type comes from the enclosing method signature; arg
      types come from the enclosing scope. No inference inside.
    - **Header includes go via `@c_include "<header.h>"`** at
      file scope, so `<curl/curl.h>` and friends stay
      explicit.
    - **Link-time deps go via `@c_link "name"`** (passed as
      `-lname` to gcc). Same mechanism as package manifests'
      `[stdlib].libs`.
    - **Sandbox / safety** — there isn't one. Inline-C can
      crash the program, leak memory, and violate GC
      invariants. Same shape as Rust's `unsafe { … }`. Doc
      this prominently; lint-flag every `@c` block in
      production-mode builds.

  **Downstream impact** — every `Amalgame_*.h` runtime header
  becomes a `Amalgame_*.am` file with the C body inlined. The
  current `stdlib/` Amalgame-side facades collapse into the
  same `.am` file as their runtime, simplifying every
  Vec3/FileWatcher/Regex-style cross-file dance. Bigger:
  third-party packages can ship a single `.am` per binding
  instead of `runtime/header.h + stdlib/facade.am`.

  **Scope** — major feature, probably v0.9 or v1.x. Touches
  parser (tokenise inline-C as a black-box span), resolver
  (skip type-check inside `@c {}`), cgen (splice the block
  verbatim with variable-name remapping), and the package
  manager (`@c_include` / `@c_link` need to round-trip through
  manifests). ~3–5 days for an MVP; another ~1–2 weeks for
  the docs + lint + GC-rules pass.

  **Trade-off accepted** — Amalgame moves from "transpile to
  C, never mix" toward Zig's / Vala's "I can drop into C when
  I have to" model. Buys self-containment (the whole runtime
  is in `.am`), at the cost of giving users a foot-gun. Net
  positive once the language has tooling to discourage misuse
  in everyday code.
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
