# Amalgame — Roadmap

> Updated 2026-05-08 · `amc 0.3.5` · self-hosted · 180/180 tests · multi-OS CI · GitHub Releases automation

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
- **VS Code syntax highlighting** (`editors/vscode/`)

### Stdlib
Console, File, Path, Math, String, List/Map/Set, Http, TcpServer/TcpConn,
TcpClient, UdpSocket, Args, Exit, Process (v0.3.4: Run + RunCapture).
Documented in [docs/guide/04-stdlib.md](docs/guide/04-stdlib.md).

### Compiler quality
- Rustc-style diagnostics with source snippet + caret (Resolver + TypeChecker)
- Multi-OS CI (Linux + macOS + Windows MSYS2). Linux runs on the
  self-hosted amc directly — Vala is no longer in the CI dependency
  graph.
- Tag-driven Release workflow (Linux .tar.gz + macOS .tar.gz + Windows
  .zip with bundled MinGW DLLs)
- 170/170 tests in CI under `./amc` (108 core + 50 stdlib + 12 fmt),
  with no SKIPs — every sample compiles under the self-hosted compiler.

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
- [ ] **Lambda v2.5 — args + non-int sigs** — make
      `xs.Select(x => x.Name)` and `xs.Filter(x => x > 0)` work.
      Needs (1) the TypeChecker to infer the lambda's expected
      signature from the formal parameter at the call site, (2)
      CGen to emit non-int signatures (real C types instead of
      `i64` everywhere) once the signature is known, (3) stdlib
      methods on List/Map/Set typed to take a closure with a
      known signature.
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

1. **Lambda v2.5 — args + non-int sigs** — `xs.Select(x => x.Name)`
   and `xs.Filter(x => x > 0)` need (a) the TypeChecker to infer
   the lambda's expected signature from the formal param at the
   call site, (b) CGen to emit non-int signatures (real C types
   instead of `i64` everywhere) once the signature is known, (c)
   stdlib methods on List/Map/Set typed to take a closure with a
   known signature. Unlocks the bulk of real-world lambda usage.
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
4. **`amc test` polish** — `--runtime <path>` flag (don't assume
   cwd has `runtime/`), per-file timeouts, parallel execution.
5. **Process v2** — split stderr from stdout via real pipes,
   add timeouts, async streaming output for long-running children.
6. **Spread operator** — `f(...args)` and `[...a, ...b]`. Needs
   list literal syntax `[...]` first.
