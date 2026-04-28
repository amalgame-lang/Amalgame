# Changelog Amalgame

Format: [Keep a Changelog](https://keepachangelog.com)
Versions: [Semantic Versioning](https://semver.org)

---

## [0.3.0] - 2026-XX-XX

### 🎉 Project renamed: CODE → Amalgame

The project is now named **Amalgame** — reflecting its core philosophy:
the amalgam of the best features from every programming language.

### Changed
- Project name: `CODE` → `Amalgame`
- File extension: `.code` → `.am`
- Executable: `codec` → `amc`
- Debug env var: `CODE_DEBUG` → `AMC_DEBUG`
- All documentation updated to English

### ✅ Added

#### Test suite (`tests/`)
- `run_tests.sh` — automated test runner with color output
- `hello.am` — Hello World baseline
- `variables.am` — primitives, let/var, string interpolation
- `control_flow.am` — if/else if/else, while, for
- `classes.am` — constructor, fields, methods, this, new
- `match.am` — literal and range pattern matching
- `math.am` — static methods, recursion, return types
- `record.am` — record types, field access

#### Generator fixes
- Static vs instance call detection (uppercase/lowercase heuristic)
- `_localCTypes` map: tracks C types during generation for correct
  string interpolation wrapping (`code_int_to_string`, `code_float_to_string`)
- `_LookupFieldCType`: resolves field types from class declarations
  for correct interpolation of `{this.Name}`, `{obj.Field}`
- `_LookupMethodReturnType`: infers return type of static method calls
  for correct `InferCType` on `let x = ClassName.Method(...)`
- Constructor detection: `MethodDeclNode` with name == class name
  → emits `ClassName_new(params)` instead of regular method
- No duplicate constructor: skips `EmitDefaultConstructor` when
  an explicit constructor exists
- `VisitNewExpr`: always uses `_new(args)` — removes `_create` variant
- `VisitFor`: defaults to `i64` instead of `void*` for loop counter

#### Parser fixes
- `ParseMethodDecl`: properly captures `ReturnType` from AST instead
  of discarding it — fixes TypeChecker seeing all methods as void
- `ParseFieldOrProperty`: now parses `Name: Type` style (Amalgame/Kotlin)
  instead of `Type Name` (Java/C#)

---

## [0.2.0] - 2026-XX-XX

### ✅ Added

#### Analyzer (src/core/analyzer/)
- `symbol.vala` — SymbolKind, Symbol, Scope, SymbolTable, Levenshtein
- `resolver.vala` — Two-pass name resolver, ResolveError/Result
- `typechecker.vala` — Type inference & validation, TypeError/Result

#### Parser fix
- Renamed `with` → `withExpr` (Vala keyword conflict)

---

## [0.1.0] - 2024-01-XX

### 🎉 MILESTONE: Hello World works!

### ✅ Added
- Lexer, AST, Parser, C Generator, Runtime
- Boehm GC integration
- Meson build system
- GitHub Actions CI
