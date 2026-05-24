# Amalgame — Developer Guide

> Guide pour contribuer au compilateur Amalgame self-hosted.

---

## 1. Prérequis

```bash
# Dépendances build
sudo apt install libgc-dev libcurl4-openssl-dev gcc

# Cold-start bootstrap (rebuilds snapshot/amc from tracked snapshot/amc_lib.c)
gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc

# Build the self-hosted compiler
./build_amc.sh

# Vérification
./amc --version
```

---

## 2. Structure du projet

```
src/          ← Code source du compilateur en Amalgame
  lexer/
    token.am           ← Définition des 132 TokenTypes (enum)
    lexer.am           ← Tokenizer : ReadString, ReadNumber, ReadSymbol
  parser/
    ast.am             ← NodeKind (65 kinds) + AstNode + Ast.* factory methods
    parser.am          ← Parser récursif descendant (Pratt)
  generator/
    c_gen.am           ← CGen principal (~2100 lignes)
    gen_test.am        ← Script génération amc_lib.c
    gen_bootstrap.am   ← Script génération bootstrap bundles
  resolver/
    symbol.am          ← SymbolTable + Symbol (avec HasSymbol/GetTypeName)
    resolver.am        ← FullResolver : 2 passes, MemberTable, scope 2-niveaux
  diagnostics.am       ← DiagnosticFormatter (errors, warnings, phase reporting)
  typechecker.am       ← TypeChecker : inférence de types, validation
  main.am              ← AmalgameCompiler.Run() + int main() entry point

runtime/   ← Headers C du runtime
  _runtime.h             ← AmalgameList, types de base, File_StreamLine
  Amalgame_String.h      ← String_* functions
  Amalgame_Collections.h ← List/Map/Set helpers
  Amalgame_IO.h          ← File_ReadAll, File_WriteAll, File_WriteLines
  Amalgame_Math.h        ← Math_* functions
  Amalgame_Net.h         ← Http_Get, TcpServer_*, curl wrapper
  Amalgame_Console.h     ← Console_WriteLine, Console_WriteError

tests/
  fmt/fmt_test.am               ← bundle AM: formatter idempotence + semantics
  amc_new/amc_new_test.am       ← bundle AM: scaffolder smoke tests
  stdlib_bundle/stdlib_test.am  ← bundle AM: 196 stdlib assertions + 2 PM e2e
  core_bundle/
    core_test.am                ← bundle AM: 325 cas core (lang, LSP, DAP, LLM)
    fixtures/lsp_*.bin          ← séquences JSON-RPC pré-calculées pour les LSP tests
  samples/                      ← programmes .am compilés par les bundles
  fixtures/                     ← pruné par `amc test` (LSP workspace, test_runner self-test, …)
  run_all_tests.sh              ← wrapper d'une ligne autour de `amc test ./tests/`
```

---

## 3. Pipeline de compilation

```
.am source files
     │
     ▼ ./amc (or ./snapshot/amc on cold start)
gen_test.c
     │
     ▼ gcc -O2
gen_test binary
     │
     ▼ ./gen_test
     └── src/amc_lib.c        (compilateur complet bundlé)
          │
          ▼ gcc
          amc
```

**Pipeline de compilation d'un programme Amalgame :**
```
hello.am
  │
  ▼ amc (Lexer → Parser → Resolver → TypeChecker → CGen)
hello.c
  │
  ▼ gcc -Iruntime hello.c -lgc -lm
hello (binaire)
```

---

## 4. Workflow de développement

### Modifier le compilateur

```bash
# 1. Éditer un .am dans src/
# 2. Rebuild + test :
./build_amc.sh
./amc test ./tests/core_bundle/ 2>&1 | tail -3   # ou ./tests/run_all_tests.sh pour la suite complète
```

`build_amc.sh` runs the three stages (compile gen_test, generate
`src/amc_lib.c`, gcc the final `amc` binary) in ~5 seconds.

### Ajouter un nouveau fichier .am au compilateur

1. Créer `src/monmodule.am`
2. L'ajouter dans la commande build (liste des `.am`)
3. L'ajouter dans `gen_test.am` :
   - Parse block (copier le pattern `resPath/resSrc/resLex/...`)
   - `gen6.AddFilePass1(monProg)` + `gen6.AddFilePass2(monProg)`
4. Rebuild complet

### Ajouter un token

1. `token.am` : ajouter `OP_MON_TOKEN` dans l'enum `TokenType`
2. `lexer.am` : ajouter la reconnaissance dans `ReadSymbol()`
3. `parser.am` : ajouter la gestion dans le niveau de précédence approprié
4. `c_gen.am` : ajouter l'émission C si nécessaire

### Ajouter un nœud AST

1. `ast.am` : ajouter `MON_NODE` dans `NodeKind` + factory method `Ast.MonNode(...)`
2. `parser.am` : créer le nœud au bon endroit
3. `c_gen.am` : ajouter `if (k == NodeKind.MON_NODE) { ... }` dans `EmitStmt` ou `EmitExprStr`
4. `typechecker.am` : ajouter la vérification de types si nécessaire

---

## 5. Le CGen (c_gen.am)

### Architecture

```
CGen
├── Emitter (Out)        ← Buffer de sortie (lignes C)
│   ├── Lines: List<string>
│   ├── Streaming: bool  ← mode streaming → écrit directement en FILE*
│   └── Indent: int
├── Pass 1 (AddFilePass1)
│   ├── EmitHeader()     ← #include, typedefs
│   ├── EmitForwardDecl() ← déclarations anticipées
│   └── EmitClassPass1() ← structs C + méthodes forward
└── Pass 2 (AddFilePass2)
    ├── EmitClass()      ← corps des méthodes
    ├── EmitStmt()       ← instructions
    └── EmitExprStr()    ← expressions → string C
```

### Règles importantes

**Accès chaînés** — Le Resolver ne suit pas les types au-delà d'un appel de méthode.
Au lieu de `obj.Field.Method()`, utiliser des variables intermédiaires :
```kotlin
// ❌ Ne compile pas correctement
let count = this.SomeList.Children.Count()

// ✅ Correct
let children = this.SomeList.Children   // type inféré de SomeList
let count = children.Count()
```

**Variables mutables** — Toujours `var` pour les variables réassignées :
```kotlin
var x = 0        // mutable
let y = "hello"  // immutable (let)
```

**Boucles bornées** — Jamais `while (ptr != null)` sur des objets GC.
Toujours une boucle `for i in 0..MAX` avec break :
```kotlin
// ❌ Peut boucler infiniment (GC ne met pas NULL)
while (scope != null) { scope = scope.Parent }

// ✅ Borné
for i in 0..64 {
    if (scope == null) { break }
    scope = scope.Parent  // var typé, pas de réassignation de type
}
```

**Corps de méthodes vides** — Pas de `{ }` vide, toujours un no-op :
```kotlin
// ❌ Fait boucler le parser
public void Noop() { }

// ✅
public void Noop() { this.X = this.X }
```

**Continuations de ligne** — Pas de `+` en fin de ligne :
```kotlin
// ❌ Parser interprète comme fin de statement
let s = "hello" +
        "world"

// ✅
let prefix = "hello"
let s = prefix + "world"
```

**Strings vides** — Ne pas retourner `""` directement, utiliser `" "` si nécessaire
(bug parser sur `return ""`).

---

## 6. Le Runtime C

### Conventions de nommage
- `code_string` = `char*` (sera renommé `amc_string` en v1.0)
- `code_bool` = `bool` (sera renommé `amc_bool` en v1.0)
- `AmalgameList*` = liste générique GC-managée
- `AmalgameMap*` = map clé-valeur GC-managée

### AmalgameList
```c
AmalgameList* l = AmalgameList_new();       // capacité initiale 64
AmalgameList_add(l, (void*)item);           // O(1) amorti
void* item = AmalgameList_get(l, i);        // O(1)
int n = AmalgameList_count(l);              // O(1)
AmalgameList_clear(l);                      // vide la liste
File_WriteLines("out.c", l);               // écrit toutes les lignes
```

### Streaming (performance)
Pour générer de gros fichiers sans GC pressure :
```c
File_OpenWrite("output.c");    // ouvre le fichier
File_StreamLine("int main() {");  // écrit une ligne directement
File_CloseWrite();             // ferme
```

### GC Notes
- Toute allocation via `GC_MALLOC` — jamais `malloc` directement
- Les strings sont GC-managées : ne pas `free()` manuellement
- Éviter les listes de milliers d'éléments (GC scan coûteux)
- `AmalgameList_reserve(l, n)` pour pré-allouer (évite les reallocations)

---

## 7. Tests

### Lancer tous les tests
```bash
./tests/run_all_tests.sh                # full suite (578 PASS / 0 FAIL / 5 SKIP, ~42s)
./amc test ./tests/                     # équivalent direct (le wrapper appelle ça)
./amc test ./tests/core_bundle/         # une suite seule (325 PASS, ~29s)
./amc test ./tests/stdlib_bundle/       # stdlib (196 PASS / 5 SKIP, ~8s)
./amc test ./tests/fmt/                 # formatter (12 PASS, ~3s)
./amc test ./tests/amc_new/             # scaffolder (38 PASS, ~1s)
```

Les bundles AM (`tests/<suite>/*_test.am`) sont la **seule** voie de
test depuis 2026-05-24 — les 4 bash runners legacy (`run_tests.sh`,
`run_stdlib_tests.sh`, `run_fmt_tests.sh`, `run_amc_new_tests.sh`)
ont été supprimés après la période de parité safety-net. `amc test`
prune `fixtures/` pour ne pas auto-exécuter les fichiers fixture
quand on crawle depuis `./tests/`.

### Ajouter un test

Pour un test de comportement runtime (compile + run + grep) :
1. Créer le sample : `tests/samples/montest.am`
2. Ajouter une assertion au bundle approprié (par ex. `core_bundle/core_test.am`) :
```amalgame
let g_montest: List<CoreCase> = new List<CoreCase>()
g_montest.Add(new CoreCase("ma feature", "sortie attendue"))
Program.RunGroup("./tests/samples/montest.am", "montest", g_montest)
```
3. Relancer : `./amc test ./tests/core_bundle/`

Pour un test de tooling (LSP, lint, --check), utiliser un helper
spécialisé (`RunLspCheck`, `RunLintCheck`, `RunCheckFail`, etc.) déjà
défini dans `core_test.am`.

### Tester un sample manuellement
```bash
./amc tests/samples/hello.am -o /tmp/hello
gcc -Iruntime /tmp/hello.c -lgc -lm -o /tmp/hello_bin
/tmp/hello_bin
```

---

## 8. Git workflow

```bash
# Nouvelle feature
git checkout -b feat/ma-feature

# Après développement + tests verts
git add src/ tests/
git commit -m "feat: description courte

- détail 1
- détail 2"

git checkout main
git merge feat/ma-feature --no-ff
git branch -d feat/ma-feature
git push origin main
```

### Fichiers générés à committer
- `src/amc_lib.c` ✅ (représente l'état du compilateur)
- `src/amc_bootstrap_lib.c` ✅
- `src/lexer/*_bootstrap.c` ✅
- `src/parser/*_bootstrap.c` ✅
- `src/generator/cgen_bundle_bootstrap.c` ✅

### Fichiers à ne PAS committer
- `gen_test`, `gen_test.c`, `gen_bootstrap`, `gen_bootstrap.c`
- `amc`, `amc_bootstrap` (binaires)
- `tests/samples/*` (binaires compilés)
- Scripts de debug temporaires (`fix_*.py`, `apply_fix.py`)

---

## 9. Bugs connus et contournements

| Bug | Contournement |
|-----|---------------|
| `return ""` fait boucler le parser | Utiliser `return " "` |
| `{ }` corps vide fait boucler le parser | `{ this.X = this.X }` |
| `obj.Field.Method()` : type inference cassé | Variable intermédiaire |
| `while (ptr != null)` boucle infinie GC | `for i in 0..MAX { break }` |
| `\x1b` dans les strings : bootstrap circulaire | Pas de `\x` dans les sources compilateur |
| gen_test lent (~1min) | Recompiler avec `-O2` ; fix streaming en cours |

---

## 10. Contribuer

1. **Fork** + branche `feat/ma-feature`
2. Modifier les sources `.am`
3. Rebuild (`build_amc.sh` ou commandes manuelles)
4. 76/76 tests verts + 0 warnings gcc
5. Self-hosting check : `./amc` compile ses propres sources, output identique
6. PR avec description des changements

### Checklist PR
- [ ] 76/76 tests passent
- [ ] 0 warnings gcc sur amc_lib.c
- [ ] amc_lib.c committée (état généré)
- [ ] Pas de `while(ptr)` ni de `{ }` vide
- [ ] Pas de `.am` files avec `\x1b`
