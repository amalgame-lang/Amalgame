# Resolver

The Resolver is the first semantic pass of the CODE transpiler.
It runs after the Parser and before the TypeChecker.

---

## Role

The Resolver answers one question for every identifier in the source:
**"Does this name exist, and where was it declared?"**

It does **not** infer or validate types — that is the TypeChecker's job.
It does **not** generate code — that is the Generator's job.

---

## Pipeline position

```
.code source
    │
    ▼
[ Lexer ]       → tokens
    │
    ▼
[ Parser ]      → AST
    │
    ▼
[ Resolver ]    → AST + SymbolTable       ← here
    │
    ▼
[ TypeChecker ] → AST annotated with types
    │
    ▼
[ C Generator ] → .c file
    │
    ▼
[ GCC ]         → native executable
```

---

## Files

| File                              | Description                              |
|-----------------------------------|------------------------------------------|
| `src/core/analyzer/symbol.vala`   | SymbolKind, Symbol, Scope, SymbolTable   |
| `src/core/analyzer/resolver.vala` | Two-pass resolver, ResolveError/Result   |

---

## Two-pass design

### Pass 1 — CollectTopLevel

Before visiting any code, the resolver walks the top-level declarations
of the program and registers every type name into the global scope:

- `class Foo`       → `Symbol("Foo", CLASS)`
- `interface IBar`  → `Symbol("IBar", INTERFACE)`
- `enum Role`       → `Symbol("Role", ENUM)`
- `record Point`    → `Symbol("Point", RECORD)`
- `data class Item` → `Symbol("Item", DATA_CLASS)`

This ensures **forward references** work: class `A` can reference
class `B` even when `B` is declared after `A` in the source file.

### Pass 2 — Full AST walk

The resolver extends `BaseAstVisitor` and overrides every `Visit*`
method that opens a scope or uses a name.

**Scope lifecycle**

| Construct              | Scope label            |
|------------------------|------------------------|
| Class body             | `class:ClassName`      |
| Interface body         | `interface:IfaceName`  |
| Enum body              | `enum:EnumName`        |
| Record body            | `record:RecordName`    |
| Data class body        | `data:ClassName`       |
| Method body            | `method:MethodName`    |
| Constructor body       | `constructor:ClassName`|
| Lambda body            | `lambda`               |
| Block `{ … }`          | `block`                |
| `for` init+body        | `for`                  |
| `foreach` body         | `foreach`              |
| `match` arm            | `match-arm`            |
| `try/catch` handler    | `catch`                |
| List comprehension     | `list-comprehension`   |

**Class member pre-collection**

When entering a class, the resolver runs a mini-pass that declares
all fields, properties, and methods into the class scope *before*
visiting their bodies. This allows a method to reference a field
declared below it.

---

## Symbol table

### SymbolKind

```
CLASS, INTERFACE, ENUM, ENUM_MEMBER, RECORD, DATA_CLASS, TRAIT
METHOD, CONSTRUCTOR, LAMBDA
FIELD, PROPERTY, LOCAL_VAR, PARAMETER
NAMESPACE, IMPORT
```

### Built-in symbols (pre-loaded)

Primitives: `int`, `float`, `double`, `string`, `bool`, `void`,
`i8`…`i64`, `u8`…`u64`, `f32`, `f64`, `char`, `byte`, `object`

Generic stubs: `List`, `Map`, `Set`, `Option`, `Result`,
`Task`, `Func`, `Action`

Built-in classes: `Console`, `Math`

### Scope chain

```
global  (builtins + top-level types)
  └─ namespace scope
       └─ class scope   (fields, methods, this)
            └─ method scope  (parameters)
                 └─ block scope  (local vars)
                      └─ nested block …
```

Lookup walks upward until a match is found or the global scope
is exhausted.

---

## Error reporting

All errors follow the same box-drawing format as `ParseError`:

```
┌── [resolver] Player.code:12:5
│
│  Unknown symbol 'playr' — did you mean 'player'?
│
└──
```

### "Did you mean?" suggestions

When a name is not found, the resolver computes the Levenshtein
distance between the unknown name and every visible symbol.
If a candidate is within distance 3, it is suggested.

### Checks performed

| Check                                        | Error message                                   |
|----------------------------------------------|-------------------------------------------------|
| Unknown identifier                           | `Unknown symbol 'x'`                            |
| Unknown type reference                       | `Unknown type 'Foo'`                            |
| Duplicate declaration in same scope          | `Variable 'x' already declared in this scope`   |
| Duplicate top-level type                     | `Duplicate top-level declaration 'Foo'`         |
| Assignment to `let` binding                  | `Cannot assign to immutable binding 'x'`        |
| `break` outside loop                         | `'break' used outside of a loop`                |
| `continue` outside loop                      | `'continue' used outside of a loop`             |
| `await` outside `async` method               | `'await' used outside of an 'async' method`     |
| `this` outside class                         | `'this' used outside of a class`                |
| Duplicate parameter                          | `Duplicate parameter 'x'`                       |
| Duplicate enum member                        | `Duplicate enum member 'X'`                     |
| Duplicate record field                       | `Duplicate record field 'x'`                    |
| Duplicate import                             | `[warning] Duplicate import 'Code.IO'`          |

---

## Usage

```vala
var resolver = new Resolver(filename);
var result   = resolver.Resolve(ast);

if (!result.Success) {
    foreach (var err in result.Errors)
        stderr.printf(err.ToString());
} else {
    // result.Symbols contains the full SymbolTable
    // Pass result.Program to the TypeChecker next
}
```

---

## What the Resolver does NOT do

- It does **not** resolve member accesses (`player.Name`) — the
  target object is resolved, but the member itself is left to the
  TypeChecker once the type of the target is known.
- It does **not** validate generic type argument constraints.
- It does **not** infer types for `var` / untyped `let` declarations.

These are intentionally deferred to the TypeChecker pass.
