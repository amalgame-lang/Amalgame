# TypeChecker

The TypeChecker is the second semantic pass of the CODE transpiler.
It runs after the Resolver and before the C Generator.

---

## Role

The TypeChecker answers two questions for every expression in the source:
**"What type does this expression have?"**
**"Are the types at this use site compatible?"**

It does **not** resolve names — that is the Resolver's job.
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
[ Resolver ]    → AST + SymbolTable
    │
    ▼
[ TypeChecker ] → AST annotated with types      ← here
    │
    ▼
[ C Generator ] → .c file
    │
    ▼
[ GCC ]         → native executable
```

---

## File

| File                                  | Description                              |
|---------------------------------------|------------------------------------------|
| `src/core/analyzer/typechecker.vala`  | Type inference, annotation, validation   |

---

## Type inference

Every expression node is visited and assigned a **TypeKey** — a string
that identifies its type. TypeKeys are stored in an internal map
`_exprTypes : HashMap<AstNode, string>` and retrieved by parent nodes
after visiting their children.

### TypeKey format

| Expression                  | TypeKey              |
|-----------------------------|----------------------|
| `42`                        | `int`                |
| `3.14`                      | `float`              |
| `"hello"`                   | `string`             |
| `true`                      | `bool`               |
| `"Hello {name}!"`           | `string`             |
| `null`                      | `null`               |
| `new Player("Arthus")`      | `Player`             |
| `new List<Player>()`        | `List<Player>`       |
| `[1, 2, 3]`                 | `List<int>`          |
| `{"a": 1}`                  | `Map<string, int>`   |
| `player.Health`             | *(field type)*       |
| `player.TakeDamage(10)`     | *(return type)*      |
| `await Http.GetAsync(url)`  | *(unwrapped Task<T>)*|
| `p => p.Level > 10`         | `Func<Player, bool>` |

---

## Type compatibility rules

The `_IsAssignable(expected, actual)` method implements the compatibility
rules used at every assignment, argument, and return site.

| Rule                              | Example                          |
|-----------------------------------|----------------------------------|
| Exact match                       | `int` ← `int` ✅                 |
| Unknown type (`?`) is permissive  | `int` ← `?` ✅                   |
| `object` is the top type          | `object` ← anything ✅           |
| Nullable base match               | `string?` ← `string` ✅          |
| `null` → nullable                 | `Player?` ← `null` ✅            |
| Numeric widening                  | `float` ← `int` ✅               |
| Numeric widening                  | `double` ← `float` ✅            |
| Incompatible types                | `int` ← `string` ❌              |
| Non-nullable ← null               | `Player` ← `null` ❌             |

### Numeric widening chain

```
i8 → i16 → i32 → i64
              ↘
               float (f32) → double (f64)
```

---

## Operator type rules

### Binary operators

| Operator      | Operand requirement    | Result type           |
|---------------|------------------------|-----------------------|
| `+`           | numeric or string      | wider numeric / string|
| `-` `*` `/` `%` `^` | numeric          | wider numeric         |
| `==` `!=` `<` `>` `<=` `>=` | any   | `bool`                |
| `&&` `\|\|`   | `bool`                 | `bool`                |
| `??`          | nullable left          | base type (strip `?`) |
| `\|>`         | any                    | type of right         |
| `>>`          | func                   | type of right         |
| `..` `...`    | any                    | `range`               |

### Unary operators

| Operator | Operand requirement | Result type |
|----------|---------------------|-------------|
| `!`      | `bool`              | `bool`      |
| `-`      | numeric             | same type   |

---

## Validation checks

| Check                                        | Error message                                          |
|----------------------------------------------|--------------------------------------------------------|
| Field initialiser type mismatch              | `Field 'x' declared as 'T' but initialised with 'U'`  |
| Property initialiser type mismatch           | `Property 'x' declared as 'T' but initialised with 'U'`|
| Method body type mismatch (expression body)  | `Method 'f' returns 'T' but body has type 'U'`         |
| Return value mismatch                        | `Return type mismatch: expected 'T', got 'U'`          |
| Return value in void method                  | `Cannot return a value from a void method`             |
| Empty return in non-void method              | `Empty return in method expecting 'T'`                 |
| Assignment type mismatch                     | `Cannot assign 'U' to 'T'`                             |
| Variable initialiser mismatch                | `Cannot assign 'U' to variable 'x' of type 'T'`        |
| Parameter default value mismatch             | `Parameter 'x' is 'T' but default is 'U'`              |
| `if` / `else if` condition not bool          | `'if condition' must be bool, got 'T'`                 |
| `while` condition not bool                   | `'while condition' must be bool, got 'T'`              |
| `for` condition not bool                     | `'for condition' must be bool, got 'T'`                |
| `guard` condition not bool                   | `'guard condition' must be bool, got 'T'`              |
| Match guard not bool                         | `'match guard' must be bool, got 'T'`                  |
| `&&` / `\|\|` operand not bool               | `Left/Right operand of '&&' must be bool, got 'T'`     |
| `+` on non-numeric, non-string               | `Operator '+' not applicable to 'T'`                   |
| Arithmetic on non-numeric                    | `Operator '-' requires numeric operands, got 'T'`      |
| `??` on non-nullable left                    | `Left operand of '??' must be nullable, got 'T'`       |
| `!` on non-bool                              | `Operator '!' requires bool, got 'T'`                  |
| Unary `-` on non-numeric                     | `Unary '-' requires numeric, got 'T'`                  |
| Unknown member access                        | `'Player' has no member 'Healt'`                       |
| Null-safe `?.` on non-nullable (warning)     | `[warning] Null-safe '?.' on non-nullable type 'T'`    |
| Wrong argument count                         | `Method 'f' expects 1-2 arguments, got 3`              |
| Wrong constructor argument count             | `Constructor expects 1-2 arguments, got 0`             |
| `with` field type mismatch                   | `'with' field 'x': cannot assign 'U' to 'T'`           |

---

## Async support

Async methods wrap their declared return type in `Task<T>`:

```code
public async Task<string> FetchName() { … }
// return type stored as "Task<string>"
```

`await` unwraps `Task<T>` back to `T`:

```code
let name = await FetchName()
// name : string
```

The TypeChecker enforces that `await` is only used inside `async` methods
(this check is shared with the Resolver).

---

## Collection type inference

| Expression            | Inferred element type |
|-----------------------|-----------------------|
| `List<Player>`        | `Player`              |
| `Set<int>`            | `int`                 |
| `Map<string, float>`  | `float` (value type)  |
| `int[]`               | `int`                 |
| `[1, 2, 3]`           | `int` (first element) |

---

## Symbol annotation

After the TypeChecker visits a declaration, it writes the resolved
TypeKey back into the corresponding `Symbol.TypeKey` in the
SymbolTable. This allows the C Generator and future passes to read
the type of any symbol without re-inferring it.

---

## What the TypeChecker does NOT do (deferred)

- Full generic type unification (Hindley-Milner)
- Variance checking (covariance / contravariance)
- Flow-sensitive type narrowing (smart casts after `is` checks)
- Interface implementation verification
- Abstract method override checking

These are left for a future dedicated type inference pass.

---

## Usage

```vala
// resolver must have run first
var tc      = new TypeChecker(resolveResult.Symbols, filename);
var result  = tc.Check(ast);

if (!result.Success) {
    foreach (var err in result.Errors)
        stderr.printf(err.ToString());
}
// result.Symbols now has TypeKey populated on every Symbol
```

---

## Error format

```
┌── [typechecker] Player.code:12:5
│
│  Return type mismatch: expected 'int', got 'string'
│
└──
```
