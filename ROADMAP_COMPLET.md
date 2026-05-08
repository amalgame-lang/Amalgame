# Amalgame — Roadmap

> Updated 2026-05-08 · `amc 0.3.1` · self-hosted · 147/147 tests · multi-OS CI · GitHub Releases automation

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
- Lambdas (non-capturing)
- try / catch / throw / finally — Vala bootstrap only;
  not yet implemented in self-hosted parser

### Tooling
- **`amc fmt`** — formatter that re-emits the AST canonically with
  comment preservation. Idempotent on every compiler source.
- **VS Code syntax highlighting** (`editors/vscode/`)

### Stdlib
Console, File, Path, Math, String, List/Map/Set, Http, TcpServer/TcpConn,
TcpClient, UdpSocket, Args, Exit. Documented in [docs/guide/04-stdlib.md](docs/guide/04-stdlib.md).

### Compiler quality
- Rustc-style diagnostics with source snippet + caret (Resolver + TypeChecker)
- Multi-OS CI (Linux + macOS + Windows MSYS2). Linux runs on the
  self-hosted amc directly — Vala is no longer in the CI dependency
  graph.
- Tag-driven Release workflow (Linux .tar.gz + macOS .tar.gz + Windows
  .zip with bundled MinGW DLLs)
- 147/147 tests in CI under `./amc` (85 core + 50 stdlib + 12 fmt),
  with no SKIPs — every sample compiles under the self-hosted compiler.

---

## 🟡 Language — backlog

In rough order of usefulness × effort:

- [ ] **Capturing closures** — `let counter = make_counter()`. Requires
      capture analysis at parse time + heap-allocated env structs.
      Touches Parser + CGen; medium-large.
- [x] **`obj.Method()` instance syntax for strings** — `s.Length()`,
      `"foo".Trim()`, `s.Replace(a, b)` etc. now lower to
      `String_Method(receiver, args)`. `EmitCalleeStr` maps
      `code_string`-typed receivers to the `String_` runtime prefix
      (the bare type is the legacy C typedef from when the language
      was called "code"); the CALL emit branch treats both
      `IDENTIFIER` and `LITERAL_STRING` receivers as `isSelfCall` so
      the receiver is passed as the first argument.
- [~] **Generic type inference** — partial. `ParseNew` captures
      the type-arg list on `NEW_EXPR.Str2`. CGen records:
      `let xs = new List<T>()` via `ListElemSet("__local__", xs, T)`
      → `xs.Get(i)` lowers to `(T)AmalgameList_get(xs, i)`.
      `let m = new Map<K,V>()` via `ListElemSet("__local_map__", m, V)`
      → `m.Get(k)` lowers to `(V)AmalgameMap_get(m, k)`. `Map.Get` is
      now wired in the dispatch (it wasn't before).
      Method params `List<T>` / `Map<K,V>` also seed the elem-type
      table on entry, so `xs.Get(i)` inside the body of a function
      taking `List<int>` returns `int`.
      Still missing: propagation through return values
      (`let xs = MakeList()` doesn't yet know `xs`'s elem type), and
      TypeChecker awareness of element types (the inference happens
      at codegen via the cast-extraction heuristic in VAR_DECL).
- [ ] **Generic interfaces** (`IComparable<T>`) — follows from generic
      inference. Modest extra work once that's in.
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
- [ ] **`amc test`** — discover `*_test.am`, compile, run, aggregate.
      Replace `tests/run_*.sh` with a self-hosted runner.
- [ ] **`amc lint`** — basic linter (unused vars, dead code,
      suspicious patterns). Cheap now that `amc fmt` is in.
- [ ] **`amc doc`** — extract doc-comments and emit Markdown / HTML.
- [ ] **`amc add <pkg>`** — package manager (re-export of the legacy
      Vala one in `archive/vala-bootstrap/src/pkg/`).
- [ ] **LSP** — `amc lsp` mode: stdio JSON-RPC over the existing
      Lexer/Parser/Resolver/TypeChecker. Wire to VS Code, Neovim,
      Emacs.
- [ ] **DAP** — debug adapter using DWARF (`-g3` already emitted).
- [ ] **Inlay hints + code actions** — once LSP is in.

### Stdlib — backlog

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

1. **Solder the open bugs** above (especially try/catch and
   `Type.Variant` patterns) — drops the SKIP list, restores feature
   parity with the Vala bootstrap, and makes future LSP/lint work
   easier because the compiler doesn't lie about what's accepted.
2. **Minimal LSP** — re-uses existing passes; the smallest LSP that
   does completion + hover is a few hundred lines.
3. **Capturing closures** — bigger but expected by anyone reading
   "modern".
4. **Generic inference** — biggest of the bunch; do it after LSP is
   in so the diagnostic story is solid first.
