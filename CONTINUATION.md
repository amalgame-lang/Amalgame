# Amalgame — Continuation Guide
## Pour reprendre le travail dans une nouvelle conversation

---

## État actuel du projet (mai 2026)

### Repository
- **GitHub** : https://github.com/BastienMOUGET/Amalgame
- **Branche active** : `feature/bootstrap`
- **Version** : v0.9.5
- **Tests** : 126/126 PASS (76 core + 50 stdlib)

### Ce qui existe

#### Compilateur Vala (archive/vala-bootstrap/src/core/ + src/transpiler/)
Le compilateur `amc` (Amalgame → C → binaire) est complet et fonctionnel.
Pipeline : source.am → Lexer → Parser → Resolver → TypeChecker → CGenerator → GCC
Build : `./compile.sh` (meson + ninja)

#### Bootstrap Amalgame (src/)
Le compilateur écrit en Amalgame lui-même — objectif final du bootstrap.

```
src/
├── lexer/
│   ├── token.am      — TokenType enum + Token class ✅ → C sans warnings ✅
│   ├── lexer.am      — Lexer complet ✅
│   └── lexer_test.am — Test
├── parser/
│   ├── ast.am        — AstNode flat + Ast factory ✅ → C sans warnings ✅
│   ├── parser.am     — Parser récursif descendant ✅
│   └── parser_test_real.am — Test
├── resolver/
│   ├── symbol.am     — SymbolTable + Resolver ✅
│   └── resolver_test.am — Test cross-fichiers ✅
├── generator/
│   ├── c_gen.am      — Générateur C ✅ (v0.9.5)
│   └── gen_test.am   — Test : génère token.am + ast.am → C → GCC OK ✅
├── diagnostics.am    — DiagnosticFormatter ✅
└── main.am           — Point d'entrée compilateur bootstrap ✅
```

#### Pipeline bootstrap fonctionnel (v0.9.5)
```
token.am → CGen → token.am_bootstrap.c → GCC sans warnings ✅
ast.am   → CGen → ast.am_bootstrap.c   → GCC sans warnings ✅
lexer.am → CGen → lexer.am_bootstrap.c → GCC ← PROCHAINE ÉTAPE (v0.9.6)
```

---

## Ce qui a été fait en v0.9.5 (session mai 2026)

### Changements dans c_gen.am
1. **Type inference** : `let n = new AstNode(...)` → `Amalgame_Compiler_AstNode* n` (plus de `void*`)
2. **`new List<T>()`** : parser bootstrap strip les génériques → `node.Name = "List"` → géré dans `TypeToC` et `EmitExprStr`
3. **Constructeurs avec params** : `_new()` prend les params du constructeur Amalgame et exécute le body
4. **Enums sans pointeur** : `TypeToC("NodeKind")` → `Amalgame_Compiler_NodeKind` (pas de `*`)
   - `EnumNames` list alimentée dans `EmitForwardDecl`
   - `IsEnum()` consulté dans `TypeToC` et `InferTypeFromExpr`
5. **`== / !=` type-aware** : string → `code_string_equals()`, enum/int/bool → `==`/`!=` C direct
6. **Inférence de champs** : `let v = this.Type` → inféré comme `Amalgame_Compiler_TokenType` via `FieldTypeGet`
7. **`(void)self; (void)param;`** : suppression des `-Wunused-parameter`
8. **`public` vs `private`** : méthodes `public` émises sans `static` en C
9. **Args dans `_new()`** : `new AstNode(kind, line, col)` → `AstNode_new(kind, line, col)` avec args

### Changements dans _runtime.h
- `static AmalgameException _am_ex = { {{0}}, ... }` → `static AmalgameException _am_ex;`
  (zero-init garanti par C99 §6.7.8/10 pour les variables statiques — plus de `-Wmissing-braces`)

---

## Prochaines étapes

### v0.9.6 — lexer.am → C sans erreurs/warnings

Problèmes identifiés à corriger dans c_gen.am :

**1. `else if` chaîné**
- Parser bootstrap : `else if` stocké comme `node.Else = IF_STMT`
- CGen actuel : `stmt.Else != null` → `EmitBlock(stmt.Else)` → émet IF comme statement dans un bloc else
- Fix : détecter `stmt.Else.Kind == IF_STMT` → émettre `} else if (...) {` récursivement

**2. `for i in 0..count` → émet juste un commentaire**
- CGen émet `/* for i in count */` — non compilable
- Fix : `for (i64 i = START; i < END; i++) { ... }`
- Parser : `FOR_IN_STMT` avec `stmt.Name = "i"`, `stmt.Left = début`, `stmt.Right = fin` (à vérifier)

**3. `this.Field.Method()` en statement**
- `this.Tokens.Add(tok)` → CALL avec callee = MEMBER(MEMBER(THIS,"Tokens"),"Add")
- `TryEmitListCall` Case 2 doit matcher ce pattern — à valider

**4. Méthodes retournant `List<Token>`**
- `TypeToC` gère `List<` → `AmalgameList*` ✅
- Mais le parser bootstrap stocke le return type tel quel : `"List<Token>"` → vérifier

### v0.9.7 — parser.am + symbol.am → C compilable

### v0.9.8 — Linker tout + amc_bootstrap binaire

---

## Comment reprendre dans une nouvelle conversation

Dis à Claude :

```
Je travaille sur le langage Amalgame (transpiler Amalgame → C).
Branche feature/bootstrap, version v0.9.5, 126/126 tests.

Context :
- Compilateur Vala fonctionnel (amc) : ./build/amc (archive/vala-bootstrap/src/core/ + src/transpiler/)
- Bootstrap en cours : src/ (lexer, parser, resolver, generator)
- token.am et ast.am → C sans warnings GCC ✅
- Prochaine étape : v0.9.6 — lexer.am → C sans erreurs

Lire CONTINUATION.md pour le détail.
Commencer par : ./tests/run_all_tests.sh puis coller le résultat.
```

---

## Architecture technique

### Pipeline Vala (actuel, stable)
```
*.am → Lexer.vala → Parser.vala → Resolver.vala → TypeChecker.vala
     → CGenerator.vala → GCC → binaire
```

### Pipeline Bootstrap (objectif)
```
*.am → amc_bootstrap (écrit en Amalgame) → C → GCC → binaire
```

---

## Commandes utiles

```bash
# Build compilateur Vala
./compile.sh

# Tests complets
./tests/run_all_tests.sh

# Pipeline bootstrap (compile + génère les .c)
./build/amc src/lexer/token.am \
            src/lexer/lexer.am \
            src/parser/ast.am \
            src/parser/parser.am \
            src/generator/c_gen.am \
            src/generator/gen_test.am \
            -o gen_test && ./gen_test

# Vérifier le C généré
gcc -Wall -Wextra -Iruntime \
    src/lexer/token.am_bootstrap.c -lgc -c 2>&1
gcc -Wall -Wextra -Iruntime \
    src/parser/ast.am_bootstrap.c -lgc -c 2>&1
gcc -Wall -Wextra -Iruntime \
    src/lexer/lexer.am_bootstrap.c -lgc -c 2>&1
```

---

## Limitations Amalgame documentées (pour le bootstrap)

1. Pas de `char` literals → utiliser strings à 1 char
2. `while (cond)` → parenthèses requises
3. `&&`/`||` multilignes → seulement dans `(...)`
4. `AstNode?` comme return type → utiliser sentinel/null check manuel
5. `super()` → non supporté (héritage limité)
6. `return null` dans méthode non-nullable → erreur TypeChecker
7. `;` non supporté pour multiple stmts sur une ligne
8. `for x in 0..n` → syntaxe standard Amalgame
9. String interpolation `{expr}` → non générée par CGen bootstrap (émise telle quelle)
10. Parser bootstrap strip les génériques : `new List<T>()` → `node.Name = "List"` (sans `<T>`)

---

## Ce qui manque encore dans CGen pour compiler main.am

- `args.Length()` → méthode sur `string[]`
- `args.Get(i)` → accès indexé sur tableau
- `Console.WriteError()` → stderr output
- Variables globales / static fields
- String interpolation `{expr}` → `code_string_format(...)`
