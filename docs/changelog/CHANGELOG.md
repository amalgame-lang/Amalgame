# Changelog Amalgame

Format: [Keep a Changelog](https://keepachangelog.com)
Versions: [Semantic Versioning](https://semver.org)

---

## [0.4.0] - 2026-04-30

### ✅ Added

#### Enums

Full enum support — simple and rich (tagged union) variants:

```amalgame
// Simple enum
public enum Direction { North, South, East, West }

// Rich enum (associated types)
public enum Shape {
    Circle(float),
    Rectangle(float, float)
}
```

- **Simple enums** → `typedef enum { Tests_Direction_North, ... } Tests_Direction`
- **Rich enums** → tagged union C struct with `static inline` constructors
- **`match` on enum** → `subject == Tests_Direction_North`
- **`EnumName.Member`** access → emits `Tests_Direction_North` directly
- **Enum methods** → methods declared inside enum body
- **Namespace prefixing** → `namespace Tests` + `Direction` → `Tests_Direction`

**Parser fixes for enums:**
- `ParseEnumDecl` — members on same line with comma separator now work:
  `North, South, East, West` parsed correctly
- `CheckMethodStart()` no longer triggered for enum members — method
  detection now requires explicit access modifier or type keyword
- `ParseMatchPattern` — `Direction.North` parsed as single pattern token
  (`BindName = "Direction.North"`)

**TypeChecker:**
- `_ResolveMemberType` handles `EnumDeclNode` — `Direction.North` resolves
  to type `"Direction"` instead of failing with "has no member"

**Generator:**
- `EmitForwardDecls` — two-pass: enum typedefs emitted first (pass 1),
  then class typedefs + static method fwd decls (pass 2). Fixes
  `unknown type name 'Tests_Direction'` in forward declarations.
- `_IsEnumType` / `_IsRichEnum` helpers for correct code generation
- `VisitMemberAccess` detects enum types and emits `Tests_Role_Tank`
  instead of `Role->Tank`

**Resolver:**
- `VisitEnumDecl` registers each member globally as `Direction_North`
  for cross-scope access

#### Interfaces — vtable dispatch

Full interface support with C vtable pattern:

```amalgame
public interface IDescribable {
    Describe() -> string
}

public class Circle implements IDescribable, IScalable {
    public string Describe() { return "Circle(r={this.Radius})" }
}

// Pass as interface — auto-converted
Program.PrintDesc(c)   // → Tests_Circle_as_IDescribable(c)
```

**Generator:**
- `VisitInterfaceDecl` — emits vtable struct + fat pointer struct:
  ```c
  typedef struct { code_string (*Describe)(void*); } Tests_IDescribable_vtable;
  typedef struct { Tests_IDescribable_vtable* vtable; void* self; } Tests_IDescribable;
  ```
- `EmitInterfaceImpl` — for each `implements`, emits static vtable instance
  and `as_Interface()` converter function
- `EmitForwardDecls` — interface typedefs emitted in pass 1a (before enums
  and classes) to avoid forward reference issues
- `EmitMethodCall` — detects interface fat pointer targets and emits vtable
  dispatch: `obj.vtable->Describe(obj.self)`
- **Auto-conversion** — `PrintDesc(c)` where `c: Circle*` and param expects
  `IDescribable` → automatically wraps as `Tests_Circle_as_IDescribable(c)`
- `TypeNameToC` — interface types returned as value types (no `*`) since
  the fat pointer struct already contains the pointer

#### Multi-file compilation (2026-04-30)

Multiple `.am` files compiled into a single executable via AST merge:

```bash
amc models.am utils.am main.am        # → out.c → ./out
amc models.am utils.am main.am -o app # → app.c → ./app
amc src/*.am -o mygame                # glob support
```

**Strategy — AST merge:**
- Each file is lexed + parsed independently
- `_MergePrograms()` merges all `ProgramNode`s:
  - Imports: union, deduplicated
  - Declarations: concatenated in order (entry point last)
  - Namespace: entry point's namespace wins
- Single Resolver → TypeChecker → CGenerator pass on the merged AST
- Single `.c` output compiled by GCC

**`main.vala` changes:**
- Accepts N `.am` files as arguments
- `-o output` — explicit output name (no `.c` extension needed)
- `AMC_RUNTIME` env var support for runtime header location
- Runtime path resolution: `AMC_RUNTIME` → relative to source → fallback
- `--version` now reports `v0.4.0` with `Multi-file: OK`

#### Test suite (2026-04-30)
- `tests/samples/multifile/models.am` — `Player`, `Enemy` classes
- `tests/samples/multifile/utils.am` — `Logger`, `MathUtils` classes
- `tests/samples/multifile/main.am` — entry point using both
- `run_multifile_test` helper in `run_tests.sh` for multi-arg invocation
- 4 multi-file tests: player status, logger, cross-file clamp, enemy


**`Amalgame.IO`** (`import Amalgame.IO`)
- `File_ReadAll`, `File_WriteAll`, `File_AppendAll`, `File_Exists`, `File_Delete`, `File_Size`
- `Path_Combine`, `Path_GetExtension`, `Path_GetFilename`, `Path_GetDirectory`
- `Environment_GetVar`, `Environment_GetVarOr`, `Environment_HasVar`
- `Console_WriteError`, `Console_Clear`

**`Amalgame.Math`** (`import Amalgame.Math`)
- Basic: `Math_Abs`, `Math_Sqrt`, `Math_Cbrt`, `Math_Pow`, `Math_Exp`, `Math_Log/2/10`
- Rounding: `Math_Floor`, `Math_Ceil`, `Math_Round`, `Math_Trunc`
- Min/Max/Clamp: `Math_MaxI/F`, `Math_MinI/F`, `Math_ClampI/F`
- Trig: `Math_Sin/Cos/Tan/Asin/Acos/Atan/Atan2/Sinh/Cosh/Tanh`
- Conversion: `Math_ToRadians`, `Math_ToDegrees`
- Integer: `Math_AbsI`, `Math_PowI`, `Math_Gcd`, `Math_Lcm`, `Math_IsPrime`
- Checks: `Math_IsNaN`, `Math_IsInf`, `Math_IsFinite`, `Math_ApproxEq`
- Random: `Math_SeedRandom`, `Math_Random`, `Math_RandomInt`
- Constants: `Amalgame_Math_PI`, `E`, `TAU`, `SQRT2`, `LN2`, `INF`

**`Amalgame.String`** (`import Amalgame.String`)
- Info: `String_Length`, `String_IsEmpty`, `String_IsWhitespace`
- Search: `String_Contains`, `String_StartsWith`, `String_EndsWith`, `String_IndexOf`, `String_LastIndexOf`
- Substrings: `String_Substring`, `String_From`, `String_Until`
- Case: `String_ToUpper`, `String_ToLower`
- Trim: `String_Trim`, `String_TrimStart`, `String_TrimEnd`
- Modify: `String_Replace`, `String_Split`, `String_Join`, `String_Repeat`, `String_PadLeft`, `String_PadRight`
- Convert: `String_ToInt`, `String_ToFloat`, `String_ToBool`, `String_FromInt`, `String_FromFloat`, `String_FromBool`
- Chars: `String_CharAt`, `String_IsDigit`, `String_IsAlpha`, `String_IsAlnum`

#### Resolver — stdlib symbol injection
- `VisitImport` now calls `_RegisterStdlibSymbols()` which pre-injects all
  stdlib function names into the global SymbolTable — prevents resolver from
  reporting stdlib functions as unknown identifiers

#### Generator — stdlib type inference
- `_StdlibReturnType()` — lookup table mapping stdlib function names to their
  C return types (`code_bool`, `i64`, `f64`, `code_string`)
- `InferCType` for `CallExprNode` with `IdentifierNode` callee now consults
  `_StdlibReturnType` → correct C type for `let ok = File_WriteAll(...)`
- `_LookupMethodReturnType` falls back to `_StdlibReturnType` for functions
  not found in user-defined classes

#### Test suite
- `tests/run_stdlib_tests.sh` — dedicated stdlib test runner (33 tests)
- `tests/run_all_tests.sh` — full suite: core (42) + stdlib (33) = 75 tests
- `tests/samples/stdlib_io.am` — IO module tests
- `tests/samples/stdlib_math.am` — Math module tests
- `tests/samples/stdlib_string.am` — String module tests

### ✅ Fixed

#### Runtime headers
- `_runtime.h` — removed `Math_Abs`, `Math_Sqrt`, `Math_Pow` macros that
  caused `static declaration follows non-static` conflicts when combined with
  `Amalgame_Math.h` (which declares proper `static inline` functions)
- `Amalgame_Math.h` — `<math.h>` now included before `_runtime.h` to ensure
  non-static declarations precede any static ones

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
