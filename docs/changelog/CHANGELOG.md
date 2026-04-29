# Changelog Amalgame

Format: [Keep a Changelog](https://keepachangelog.com)
Versions: [Semantic Versioning](https://semver.org)

---

## [0.4.0] - 2026-XX-XX  ← next

### 🔜 Planned
- Standard library: `Amalgame.IO`, `Amalgame.Math`, `Amalgame.Collections`
- Enum generation in C
- Interface vtable dispatch
- Multi-file compilation

---

## [0.3.1] - 2026-XX-XX

### ✅ Fixed

#### Generator
- **Namespace prefixing** — all C symbols now prefixed with namespace:
  `namespace MyApp` → `MyApp_Player`, `MyApp_Player_new()`, `MyApp_Player_TakeDamage()`
  `namespace MyApp.Models` → `MyApp_Models_Player` (dots replaced by underscores)
- **Library mode** — files without `class Program { static Main() }` are
  automatically compiled as libraries (no `int main()` emitted)
- **`--lib` flag** — forces library mode regardless of file content
- **Generator output** now reports mode: `Generator OK : file.c [Executable|Library]`
- **Forward declarations** — now emit full parameter types to avoid C type
  conflicts (`void(code_bool)` vs `void(_Bool)`)
- **`string[] args` forward decl** — correctly emits `int, char**` to match
  `EmitParamList` special case
- **`TypeNameToC`** — user-defined types now get namespace prefix applied:
  `Animal*` → `Tests_Animal*`
- **`_StripNsPrefix`** — all lookup helpers (`_LookupFieldCType`,
  `_LookupMethodInClass`, `_FindInheritedMemberPrefix`) strip namespace
  prefix before comparing bare class names
- **Chained method call** — `this.Home.Format()` now correctly resolves
  `Tests_Address_Format(self->Home)` via new `MemberAccessNode` branch
  in `EmitMethodCall`
- **`InferCType`** — now handles `MemberAccessNode`, `ThisNode`, `f32`
  literals, and comparison operators (returns `code_bool`)
- **String concatenation** — `string + string` → `code_string_concat(a, b)`
  instead of invalid C `a + b`
- **`EmitParamList`** — registers each parameter in `_localCTypes` for
  correct type inference during generation
- **`VisitFor`** — registers loop variable in `_localCTypes`

#### Parser
- **`ParseMatchArm`** — arm body now handles `return`, `break`, `continue`
  as statements (via `ParseStatement()`) instead of treating them as
  expressions — fixes `match n { 0 => return "zero" }`
- **Match arm semicolon** — `ReturnNode`, `BreakNode`, `ContinueNode` no
  longer get a spurious `;` appended after emission

#### Argument parsing (`main.vala`)
- **Flexible argument order** — `amc --lib file.am` and `amc file.am --lib`
  both work; file can appear in any position among flags

### ✅ Added

#### Test suite — Namespace section
- `namespace.am` — sub-namespace `MyApp.Models`, multi-class, runtime check
- C-level checks: `MyApp_Models_Player`, `struct _MyApp_Models_Player`,
  `MyApp_Models_Player_Info`, `MyApp_Program_Main`

#### Test suite — Library mode section
- `library.am` — auto-detected library (no `Program.Main`)
- `forced_lib.am` — `--lib` flag forces library mode on executable
- Checks: `Library — no entry point`, `Amalgame_Utils_StringHelper`

#### Test suite — Extended coverage (20 new tests)
- `operators.am` — arithmetic, comparison, logical, compound assignment
- `strings.am` — concatenation `+`, string interpolation with expressions
- `loops.am` — `break`, `continue`, nested loops, multi-condition while
- `null_safety.am` — bool logic, coalesce pattern, static helpers
- `static_class.am` — default constructor, expression-body methods `=>`
- `pattern_advanced.am` — `match` with `return` in arms, ranges, wildcard
- `multi_class.am` — class composition, chained field access, method calls
- `recursion.am` — Fibonacci, GCD, power, digit sum (mutual recursion)
- `type_explicit.am` — explicit type annotations, negative numbers, floats

#### Distribution
- `install/install.sh` — universal Linux/macOS installer (`curl | bash`)
- `install/windows/install.ps1` — Windows PowerShell installer (`irm | iex`)
- `install/windows/amalgame.iss` — Inno Setup `.exe` installer script
- `install/homebrew/amalgame.rb` — Homebrew formula
- `install/PUBLISHING.md` — step-by-step guide for all package managers
- `.github/workflows/release.yml` — CI: Linux x86_64 + ARM64, macOS x86_64 + ARM64
- `.github/workflows/release-windows.yml` — CI: Windows x86_64 + Inno Setup

#### Documentation
- `docs/DEVELOPER_GUIDE.md` — complete 1600-line developer reference
  covering all 28 language features with examples
- `docs/transpiler/typechecker.md` — TypeChecker pass documentation

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
- 7 core test files covering hello world through records

#### Generator fixes
- Static vs instance call detection
- `_localCTypes` map for string interpolation type wrapping
- `_LookupFieldCType`, `_LookupMethodReturnType` for InferCType
- Constructor detection (MethodDeclNode name == class name)
- No duplicate constructor emission
- `VisitNewExpr`: always uses `_new(args)`
- `VisitFor`: defaults to `i64` for loop counter

#### Parser fixes
- `ParseMethodDecl`: properly captures `ReturnType`
- `ParseFieldOrProperty`: parses `Name: Type` (Amalgame/Kotlin style)

---

## [0.2.0] - 2026-XX-XX

### ✅ Added

#### Analyzer (`src/core/analyzer/`)
- `symbol.vala` — SymbolKind, Symbol, Scope, SymbolTable, Levenshtein
- `resolver.vala` — two-pass name resolver, ResolveError/Result
- `typechecker.vala` — type inference & validation, TypeError/Result

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
