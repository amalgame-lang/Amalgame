# Roadmap CODE

---

## Phase 1 : Transpileur MVP  [En cours]

| Étape              | Statut | Fichiers                          |
|--------------------|--------|-----------------------------------|
| Token + Lexer      | ✅     | `src/core/lexer/`                 |
| AST                | ✅     | `src/core/parser/ast.vala`        |
| Parser             | 🔜     | `src/core/parser/parser.vala`     |
| Resolver           | 🔜     | `src/core/analyzer/resolver.vala` |
| Type Checker       | 🔜     | `src/core/analyzer/typechecker.vala` |
| Générateur C       | 🔜     | `src/transpiler/generator/`       |
| Hello World !      | 🔜     | Premier programme qui tourne      |

## Phase 2 : Écosystème  [Planifié]

| Étape              | Statut | Description                       |
|--------------------|--------|-----------------------------------|
| Runtime C          | 🔜     | GC, ARC, Arena en C               |
| Stdlib Core        | 🔜     | Types, Collections, Math          |
| Stdlib IO          | 🔜     | File, Console, Stream             |
| Stdlib Net         | 🔜     | Http, WebSocket, Tcp              |
| LSP Server         | 🔜     | Autocomplétion, erreurs           |
| DAP Server         | 🔜     | Debug, breakpoints                |
| Extension VSCode   | 🔜     | Support complet VSCode            |

## Phase 3 : Maturité  [Futur]

| Étape              | Statut | Description                       |
|--------------------|--------|-----------------------------------|
| Package Manager    | 🔜     | `codec pkg`                       |
| Bootstrap          | 🔜     | CODE écrit en CODE                |
| Plugin WASM        | 🔜     | Transpilation vers WebAssembly    |
| Plugin IL          | 🔜     | Transpilation vers IL             |
| Plugin LLVM        | 🔜     | Transpilation vers LLVM IR        |
