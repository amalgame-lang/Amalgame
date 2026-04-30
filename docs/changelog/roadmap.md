# Roadmap Amalgame

---

## Phase 1 : Transpiler MVP  [✅ Complete]

| Step                    | Status | Files                                        |
|-------------------------|--------|----------------------------------------------|
| Token + Lexer           | ✅     | `src/core/lexer/`                            |
| AST                     | ✅     | `src/core/parser/ast.vala`                   |
| Parser                  | ✅     | `src/core/parser/parser.vala`                |
| Resolver                | ✅     | `src/core/analyzer/resolver.vala`            |
| Type Checker            | ✅     | `src/core/analyzer/typechecker.vala`         |
| C Generator             | ✅     | `src/transpiler/generator/c_generator.vala`  |
| Hello World             | ✅     | First program running                        |
| Test suite (42/42)      | ✅     | `tests/`                                     |
| Namespace prefixing     | ✅     | `namespace MyApp` → `MyApp_Symbol`           |
| Library mode            | ✅     | `amc --lib` / auto-detection                 |
| Distribution scripts    | ✅     | `install/`, `.github/workflows/`             |
| Developer Guide         | ✅     | `docs/DEVELOPER_GUIDE.md`                    |

## Phase 2 : Ecosystem  [In progress]

| Step                    | Status | Description                                  |
|-------------------------|--------|----------------------------------------------|
| C Runtime               | ✅     | GC, ARC, Arena in C                          |
| Stdlib IO               | ✅     | Console, File, Path, Environment             |
| Stdlib Math             | ✅     | Full math library with constants             |
| Stdlib String           | ✅     | Complete string manipulation                 |
| Stdlib Collections      | ✅     | List, Map, Set — full implementation          |
| Stdlib Net              | 🔜     | Http, WebSocket, Tcp                         |
| Enum generation         | ✅     | Simple + rich enums, match, namespace prefix  |
| Interface vtable        | ✅     | vtable dispatch, fat pointer, auto-conversion |
| Multi-file compilation  | ✅     | AST merge, N files → 1 executable            |
| LSP Server              | 🔜     | Autocompletion, real-time errors             |
| DAP Server              | 🔜     | Debug, breakpoints                           |
| VSCode Extension        | 🔜     | Full VSCode support                          |

## Phase 3 : Maturity  [Future]

| Step                    | Status | Description                                  |
|-------------------------|--------|----------------------------------------------|
| Generics (full)         | 🔜     | Hindley-Milner type unification              |
| Package Manager         | 🔜     | `amc pkg`                                    |
| Bootstrap               | 🔜     | Amalgame written in Amalgame                 |
| WASM Plugin             | 🔜     | Transpilation to WebAssembly                 |
| IL Plugin               | 🔜     | Transpilation to IL (.NET)                   |
| LLVM Plugin             | 🔜     | Transpilation to LLVM IR                     |
