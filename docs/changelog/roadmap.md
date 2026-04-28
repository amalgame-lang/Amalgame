# Roadmap Amalgame

---

## Phase 1 : Transpiler MVP  [In progress]

| Step               | Status | Files                                        |
|--------------------|--------|----------------------------------------------|
| Token + Lexer      | ✅     | `src/core/lexer/`                            |
| AST                | ✅     | `src/core/parser/ast.vala`                   |
| Parser             | ✅     | `src/core/parser/parser.vala`                |
| Resolver           | ✅     | `src/core/analyzer/resolver.vala`            |
| Type Checker       | ✅     | `src/core/analyzer/typechecker.vala`         |
| C Generator        | ✅     | `src/transpiler/generator/c_generator.vala`  |
| Hello World !      | ✅     | First program running                        |
| Test suite         | ✅     | `tests/` — 7/7 passing                       |

## Phase 2 : Ecosystem  [Planned]

| Step               | Status | Description                       |
|--------------------|--------|-----------------------------------|
| C Runtime          | ✅     | GC, ARC, Arena in C               |
| Stdlib Core        | 🔜     | Types, Collections, Math          |
| Stdlib IO          | 🔜     | File, Console, Stream             |
| Stdlib Net         | 🔜     | Http, WebSocket, Tcp              |
| LSP Server         | 🔜     | Autocompletion, real-time errors  |
| DAP Server         | 🔜     | Debug, breakpoints                |
| VSCode Extension   | 🔜     | Full VSCode support               |

## Phase 3 : Maturity  [Future]

| Step               | Status | Description                       |
|--------------------|--------|-----------------------------------|
| Package Manager    | 🔜     | `amc pkg`                         |
| Bootstrap          | 🔜     | Amalgame written in Amalgame      |
| WASM Plugin        | 🔜     | Transpilation to WebAssembly      |
| IL Plugin          | 🔜     | Transpilation to IL               |
| LLVM Plugin        | 🔜     | Transpilation to LLVM IR          |
