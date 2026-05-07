# Amalgame — Roadmap

> Updated 2026-05-07 · `amc` self-hosted · 127/127 tests · multi-OS CI · GitHub Releases automation

This document is the canonical "what's done, what's next" board.
For architecture and contribution guidance see
[docs/guide/](docs/guide/README.md).

---

## ✅ Shipped

### Self-hosted compiler
The compiler is written in Amalgame in [src/](src/) and compiles
itself in ~5 seconds (`./build_amc.sh`). The Vala bootstrap remains
in `archive/vala-bootstrap/` as a recovery path; `./compile.sh`
rebuilds it on demand.

| Component        | File                              |
|------------------|-----------------------------------|
| Lexer            | `src/lexer/{token,lexer}.am`      |
| Parser           | `src/parser/{ast,parser}.am`     |
| Resolver         | `src/resolver/{symbol,resolver}.am`|
| TypeChecker      | `src/typechecker.am`             |
| CGen             | `src/generator/c_gen.am`         |
| Diagnostics      | `src/diagnostics.am`             |
| CLI entry        | `src/main.am`                    |
| Generated bundle | `src/amc_lib.c` (~7 000 lines)   |
| Runtime (C)      | `runtime/*.h`                    |

### Language features
- Variables: `let` / `var` with optional type annotation
- Primitives: int / float / double / bool / string / void
- Classes, inheritance (single), interfaces (basic)
- Data classes, records
- Enums (simple) + algebraic enums (tagged unions) with destructuring
- Generics (erased to `void*` at C level)
- Null-safety: `T?` types, `??` coalescing, `?.` safe access (field + method)
- Tuples + destructuring `let (a, b) = f()`
- String interpolation `"hello {x}"` (with method calls inside)
- Triple-quoted multiline strings `"""…"""`
- `\xHH` and `\uHHHH` escape sequences
- Bitwise ops, compound assigns, pipeline `|>`, range `0..n`
- Pattern matching with **arm guards** (`n if n > 0 => …`), ranges, binders
- Guard clauses: `guard cond else { return }`
- Decorators: `@inline`, `@deprecated` → C attributes
- Named arguments (documentation-only at call site)
- Lambdas (non-capturing)
- try / catch / throw / finally (setjmp-based, non-stack-unwinding)

### Stdlib
Console, File, Path, Math, String, List/Map/Set, Http, TcpServer/TcpConn,
Args, Exit. Documented in [docs/guide/04-stdlib.md](docs/guide/04-stdlib.md).

### Compiler quality
- Rustc-style diagnostics with source snippet + caret (Resolver + TypeChecker)
- Multi-OS CI (Linux + macOS + Windows MSYS2)
- Tag-driven Release workflow (Linux .tar.gz + macOS .tar.gz + Windows .zip with bundled MinGW DLLs)
- VS Code syntax highlighting extension (`editors/vscode/`)
- 127/127 tests in CI

### Recent commits resolving named priorities
| Old code | What landed |
|----------|-------------|
| P1       | Streaming gen_test brought the build down from ~2m30 to ~5s |
| P2       | TypeChecker member resolution via the resolver's MemberTable |
| P3       | `--lib` mode is type-checked and tested end-to-end (`tests/samples/lib_e2e.am`) |
| P4       | Vala sources moved to `archive/vala-bootstrap/` (recovery path) |
| P5       | Resolver gained a real scope stack (push/pop/RemoveAt) |
| P6       | Diagnostics enriched with source snippets + caret |
| P7       | `\x` and `\u` escapes parsed properly; ANSI colors restored |

---

## 🟡 Language — backlog

In rough order of usefulness × effort:

- [ ] **List comprehensions** `[x*2 for x in xs if x > 0]`
      — desugar via GCC compound statement expression. Small, high impact.
- [ ] **Capturing closures** — `let counter = make_counter()`. Requires
      capture analysis at parse time + heap-allocated env structs.
      Touches Parser + CGen; medium-large.
- [ ] **`obj.Method()` instance syntax for strings** —
      sugar for `String.Method(obj)`. Tracked since stdlib was made
      explicit. Small CGen extension.
- [ ] **Generic type inference** — `let xs = new List<int>()` should
      let `xs.Get(i)` return `int`, not `void*`. Touches TypeChecker
      + CGen's collection method dispatch. Largest item in the lot.
- [ ] **Generic interfaces** (`IComparable<T>`) — follows from generic
      inference. Modest extra work once that's in.
- [ ] **Spread operator** `f(...args)` and `[...a, ...b]`. Needs list
      literal syntax `[...]` first and a clear semantics for variadic
      calls. Larger than it looks.
- [ ] **Match as expression** — `let x = match y { … }`. Currently arms
      are statements only. Implement via GCC compound expressions, like
      arm guards already do. Medium.
- [ ] **`async` / `await`** — coroutines via ucontext or setjmp.
      Substantial: runtime + AST + CGen. Defer until there's a concrete
      use case.

---

## 🟠 Compiler — polish

- [ ] **Ban `match` as expression at parse time** with a clear error
      pointing at the workaround (early-return arms / let-then-match).
      Currently produces broken C — confusing.
- [ ] **Multi-file type checking** — TypeChecker only walks
      `programs[0]`. Walk all programs.
- [ ] **Better error recovery** — the parser is okay but produces
      `_unknown_` placeholder ASTs that cascade into noisy resolver
      errors. Skip them more aggressively.
- [ ] **`while(ptr != null)` GC issue** — existing workaround uses
      `for i in 0..N`. Investigate whether it's a real GC bug or
      just a CGen mis-detection.

---

## 🟢 Ecosystem — outillage et docs

- [ ] **`amc fmt`** — formatter. Re-emits a parsed AST with canonical
      indentation, spacing, and trailing-comma rules. Foundational for
      LSP and contributor flow.
- [ ] **`amc test`** — discover `*_test.am`, compile, run, aggregate.
      Replace `tests/run_*.sh` with a self-hosted runner.
- [ ] **`amc lint`** — basic linter (unused vars, dead code, suspicious
      patterns). Cheap once `amc fmt` is in.
- [ ] **`amc doc`** — extract doc-comments and emit Markdown / HTML.
- [ ] **`amc add <pkg>`** — package manager (re-export of the legacy
      Vala one in `archive/vala-bootstrap/src/pkg/`).
- [ ] **LSP** — `amc lsp` mode: stdio JSON-RPC over the existing
      Lexer/Parser/Resolver/TypeChecker. Wire to VS Code, Neovim, Emacs.
- [ ] **DAP** — debug adapter using DWARF (`-g3` already emitted).
- [ ] **Inlay hints + code actions** — once LSP is in.

### Distribution
- [x] GitHub Actions CI (Linux/macOS/Windows)
- [x] GitHub Releases automation (tag-triggered)
- [ ] Homebrew tap (formula draft in `install/homebrew/amalgame.rb`)
- [ ] Homebrew core (after public adoption)
- [ ] AUR / `.deb` / `.rpm` / Nix flake / winget / Scoop
- [ ] `install.sh` universal one-liner

### Documentation
- [x] User guide (`docs/guide/`, this PR)
- [x] README that doesn't lie about features
- [ ] Static site (docs.amalgame-lang.org)
- [ ] Tour interactif à la go.dev/tour
- [ ] EBNF grammar (file exists at `docs/language/grammar.ebnf` — to be re-validated)
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
- **Error vs. exception model** — `try/catch/throw` works but isn't
  idiomatic. Consider a Rust-like `Result<T, E>` plus `?` operator
  for short-circuiting.
- **Single-threaded** — bdwgc is configured for the main thread. If
  Amalgame ever wants concurrency, the GC config and runtime helpers
  need a pass.

---

## How to pick the next thing

Top of the list, ordered by *unlocked-value* per *days-of-work*:

1. **`amc fmt`** — paves the way for everything else IDE-ish, and gives
   contributors a tool they'd use immediately.
2. **List comprehensions** — small, idiomatic, immediate user delight.
3. **Match as expression** — completes the matching story.
4. **LSP minimal** — re-uses existing passes; the smallest LSP that
   does completion + hover is a few hundred lines.
5. **Capturing closures** — bigger but expected by anyone reading
   "modern".
6. **Generic inference** — biggest of the bunch; do it after fmt/LSP
   are in so the diagnostic story is solid first.
