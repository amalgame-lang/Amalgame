# Amalgame — Continuation Guide
## Pour reprendre le travail dans une nouvelle conversation

---

## État actuel du projet (mai 2026)

### Repository
- **GitHub** : https://github.com/BastienMOUGET/Amalgame
- **Branche active** : `feature/bootstrap`
- **Version** : v0.9.4
- **Tests** : 126/126 PASS (76 core + 50 stdlib)

### Ce qui existe

#### Compilateur Vala (src/core/ + src/transpiler/)
Le compilateur `amc` (Amalgame → C → binaire) est complet et fonctionnel.
Pipeline : source.am → Lexer → Parser → Resolver → TypeChecker → CGenerator → GCC
Build : `./compile.sh` (meson + ninja)

#### Bootstrap Amalgame (src/amalgame/)
Le compiler écrit en Amalgame lui-même — objectif final du bootstrap.

```
src/amalgame/
├── lexer/
│   ├── token.am      — TokenType enum + Token class ✅
│   ├── lexer.am      — Lexer complet ✅
│   └── lexer_test.am — Test (3 sources, 6 tokens) ✅
├── parser/
│   ├── ast.am        — AstNode flat + Ast factory ✅
│   ├── parser.am     — Parser récursif descendant ✅
│   ├── parser_test_real.am — Test
│   └── test_input.txt — Source de test
├── resolver/
│   ├── symbol.am     — SymbolTable + Resolver ✅
│   └── resolver_test.am — Test cross-fichiers ✅
└── generator/
    ├── c_gen.am      — Générateur C ✅
    └── gen_test.am   — Test : génère token.am → C → GCC OK ✅
```

#### Pipeline bootstrap fonctionnel
```
token.am → Lexer.am → Parser.am → CGen.am → token_bootstrap.c → GCC ✅
ast.am   → Lexer.am → Parser.am → CGen.am → ast_bootstrap.c   → GCC ✅
```

---

## Fichiers à nettoyer (racine du projet)

Ces fichiers sont des artefacts de debug/test à supprimer :
```bash
rm debug_eq debug_eq.c
rm file_lex_test file_lex_test.c
rm gen_test gen_test.c
rm lexer_file_test lexer_file_test.c
rm lexer_simple lexer_simple.c
rm parser_real parser_real.c
rm parser_test parser_test.c parser_test2 parser_test2.c
rm resolver_test resolver_test.c
rm test_contains2 test_contains2.c
rm token_bootstrap.c token_bootstrap.o
rm src/amalgame/lexer/token.am_bootstrap.c
rm src/amalgame/parser/ast.am_bootstrap.c
```

---

## Prochaines étapes v0.9.5+

### Court terme — Compléter le CGen bootstrap

**v0.9.5** — Type inference dans CGen
- `let n = Ast.Class(...)` → inférer `AstNode*` (actuellement `void*`)
- `List<T>` → `AmalgameList*` + méthodes `Add/Count/Get`
- `new List<AstNode>()` → `AmalgameList_new()`
- Objectif : `ast.am → C → GCC sans warnings`

**v0.9.6** — Générer lexer.am en C compilable
- Gérer les méthodes avec corps complexes
- `for i in 0..count` → boucle C
- Méthodes de string : `String_Contains`, `String_Substring`, etc.
- Objectif : `lexer.am → C → GCC sans errors`

**v0.9.7** — Bootstrap complet
- Générer parser.am + resolver/symbol.am
- Linker tout ensemble
- Objectif : `amc_bootstrap` binaire qui peut compiler un "Hello World"

### Moyen terme — Optimisations et parité Vala

**Limitations connues du langage Amalgame à corriger :**
1. `AstNode?` comme type de retour de méthode non supporté
2. `super()` non supporté (héritage)  
3. `&&`/`||` multilignes sans parens explicites
4. Paramètres `out` non supportés
5. `string == ""` sur retour de méthode directement

**Features à ajouter (parité Vala) :**
- Pattern matching plus riche (guards, nested patterns)
- Generics dans les classes utilisateur (pas seulement stdlib)
- Lambdas/closures plus robustes
- Type aliases
- Modules/namespaces imbriqués
- Exceptions plus riches

### Long terme — Refactoring repository

**Question : faut-il refaire le repository ?**

**Recommandation : NON, pas de refaire from scratch**

Stratégie conseillée :
1. Merger `feature/bootstrap` → `develop` → `main` quand bootstrap est stable
2. Créer une branche `feature/pure-amalgame` où on retire graduellement Vala
3. Garder l'historique git — il a de la valeur
4. Quand le bootstrap compiler peut se compiler lui-même → archiver Vala dans `legacy/`

---

## Comment reprendre dans une nouvelle conversation

Dis à Claude :

```
Je travaille sur le langage Amalgame (transpiler Amalgame → C).
Branche feature/bootstrap, version v0.9.4, 126/126 tests.

Context :
- Compilateur Vala fonctionnel (amc) : src/core/ + src/transpiler/
- Bootstrap en cours : src/amalgame/ (lexer, parser, resolver, generator)
- token.am et ast.am génèrent du C compilable par GCC
- Prochaine étape : v0.9.5 — type inference dans CGen pour ast.am sans warnings

Lire le fichier CONTINUATION.md dans le repo pour le détail.
Commencer par : ./compile.sh && ./tests/run_all_tests.sh
```

---

## Architecture technique résumée

### Pipeline Vala (actuel)
```
*.am → Lexer.vala → Parser.vala → Resolver.vala → TypeChecker.vala 
     → CGenerator.vala → GCC → binaire
```

### Pipeline Bootstrap (en cours)
```
*.am → token.am/lexer.am → ast.am/parser.am → symbol.am 
     → c_gen.am → C → GCC → binaire
```

### Limitations Amalgame documentées (pour le bootstrap)
1. Pas de `char` literals → utiliser strings
2. `while (cond)` → parenthèses requises
3. `&&`/`||` multilignes → seulement dans `(...)`
4. `AstNode?` comme return type → utiliser sentinel
5. `super()` → non supporté (héritage limité)
6. `return null` dans méthode non-nullable → erreur TypeChecker
7. `;` non supporté pour multiple stmts sur une ligne
8. `for x in 0..n` → syntaxe standard Amalgame

---

## Commandes utiles

```bash
# Build
./compile.sh

# Tests
./tests/run_all_tests.sh
./tests/run_stdlib_tests.sh

# Bootstrap pipeline
./build/amc src/amalgame/lexer/token.am \
            src/amalgame/lexer/lexer.am \
            src/amalgame/parser/ast.am \
            src/amalgame/parser/parser.am \
            src/amalgame/generator/c_gen.am \
            src/amalgame/generator/gen_test.am \
            -o gen_test && ./gen_test

# Compiler le C généré
gcc -c -I./src/transpiler/runtime token_bootstrap.c -o token_bootstrap.o
```

---

## Fichiers ajoutés — main.am et diagnostics.am

### src/amalgame/diagnostics.am
Formatage des erreurs du compilateur (style Rust/Swift).
- `DiagnosticFormatter` class
- `FormatError()`, `FormatWarning()`, `PrintCompileOk()`, `PrintCompileError()`
- Support couleurs ANSI optionnel

### src/amalgame/main.am  
Point d'entrée du compilateur bootstrap.
- `CompilerArgs` — parsing des arguments CLI
- `AmalgameCompiler.Run()` — orchestre le pipeline complet
  1. Lex + Parse tous les fichiers
  2. Resolve (cross-fichiers)
  3. Generate C
  4. Écrire le .c
  5. Lancer GCC
- `RunBootstrap()` — subcommands save/restore/validate
- `Program.Main()` — entry point

### Structure bootstrap complète
```
src/amalgame/
├── diagnostics.am    ← NOUVEAU
├── main.am           ← NOUVEAU
├── lexer/
│   ├── token.am      ✅
│   └── lexer.am      ✅
├── parser/
│   ├── ast.am        ✅
│   └── parser.am     ✅
├── resolver/
│   └── symbol.am     ✅
└── generator/
    └── c_gen.am      ✅
```

### Commande bootstrap complète (objectif)
```bash
./build/amc src/amalgame/lexer/token.am \
            src/amalgame/lexer/lexer.am \
            src/amalgame/parser/ast.am \
            src/amalgame/parser/parser.am \
            src/amalgame/resolver/symbol.am \
            src/amalgame/generator/c_gen.am \
            src/amalgame/diagnostics.am \
            src/amalgame/main.am \
            -o amc_bootstrap && ./amc_bootstrap --help
```

### Ce qui manque encore dans CGen pour compiler main.am
- `args.Length()` → méthode sur `string[]`
- `args.Get(i)` → accès indexé sur tableau
- `Console.WriteError()` → stderr output
- Variables globales / static fields

