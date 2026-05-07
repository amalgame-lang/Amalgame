# Amalgame — Roadmap Complète

> Dernière mise à jour : mai 2026  
> État : **Bootstrap self-hosted validé** · 76/76 tests · `amc` compile ses propres sources  
> GitHub : https://github.com/BastienMOUGET/Amalgame

---

## ✅ PHASES TERMINÉES

### Phase 1 — Compilateur Vala (legacy)
- Lexer, Parser, Resolver, TypeChecker complets en Vala
- CGen (transpileur Amalgame → C) en Vala
- 126/126 tests unitaires
- Runtime C : `_runtime.h`, `Amalgame_String.h`, `Amalgame_Collections.h`,
  `Amalgame_IO.h`, `Amalgame_Math.h`, `Amalgame_Net.h`, `Amalgame_Console.h`
- Installeurs : Homebrew formula, Windows Inno Setup `.exe`, PowerShell installer, `install.sh`
- `install/PUBLISHING.md` : guide GitHub Releases, Homebrew tap, AUR, .deb, .rpm, Nix, winget

### Phase 2 — Bootstrap Amalgame-en-Amalgame ✅
**Le compilateur `amc` est écrit en Amalgame et se compile lui-même.**

Sources compilateur (`src/amalgame/`) :

| Fichier | Lignes | Rôle |
|---------|--------|------|
| `lexer/token.am` | 132 | 132 TokenTypes (+ bitwise, compound assigns) |
| `lexer/lexer.am` | 258 | Tokenizer complet |
| `parser/ast.am` | ~300 | NodeKind + AstNode + factory methods |
| `parser/parser.am` | 1211 | Parser récursif descendant (Pratt) |
| `generator/c_gen.am` | 2099 | CGen Amalgame → C |
| `generator/gen_test.am` | 209 | Génère amc_lib.c |
| `generator/gen_bootstrap.am` | 178 | Génère bootstrap bundles (rapide) |
| `resolver/symbol.am` | ~370 | SymbolTable + Symbol |
| `resolver/resolver.am` | 777 | FullResolver + MemberTable (2 passes) |
| `diagnostics.am` | 151 | DiagnosticFormatter |
| `typechecker.am` | 783 | TypeChecker complet |
| `main.am` | ~115 | AmalgameCompiler.Run() |
| `amc_lib.c` (généré) | ~7000 | Compilateur compilé |
| `amc_bootstrap_lib.c` (généré) | 4221 | Bootstrap compilé |

**Métriques :**
- 76/76 tests ✅ · 0 warnings gcc ✅ · self-hosting validé ✅

### Features du langage supportées par le CGen

**Typage & structures :**
- ✅ Variables `let`/`var`, types primitifs (int, i64, float, double, string, bool)
- ✅ Classes, héritage simple, interfaces
- ✅ Data classes, records
- ✅ Enums simples + **enums algébriques** (tagged unions)
- ✅ Génériques `T → void*`
- ✅ Null safety (`T?` types, `??` coalescing, `?.` safe access)

**Expressions :**
- ✅ String interpolation `"hello {name}"`
- ✅ Multiline strings
- ✅ Closures/lambdas (`x => x * 2` via macro C)
- ✅ Tuples : `(int, string)`, literals `(a, b)`, destructuring `let (a, b) = f()`
- ✅ **Opérateurs bitwise** : `&` `|` `^` `~` `<<` `>>`
- ✅ **Compound assigns** : `+=` `-=` `*=` `/=` `%=` `&=` `|=` `^=` `<<=` `>>=`
- ✅ Pipeline `|>`
- ✅ Précédence correcte (12 niveaux)

**Structures de contrôle :**
- ✅ `if`/`else`, one-liners `if (x) { ... }` sur une ligne
- ✅ `for x in 0..n`, `for x in collection`
- ✅ `while`
- ✅ **Match/Pattern matching** : valeurs, ranges `75..99`, variants algébriques, wildcard `_`
- ✅ `try`/`catch`/`throw`/`finally` (basique)
- ✅ `break`, `continue`

**Stdlib :**
- ✅ `String_*` (Length, Contains, StartsWith, Replace, Split, Trim...)
- ✅ `List<T>`, `Map<K,V>`, `Set<T>`
- ✅ `File.*` (ReadAll, WriteAll, WriteLines, AppendAll, OpenWrite, StreamLine)
- ✅ `Console.*` (WriteLine, WriteError, ReadLine)
- ✅ `Math.*` (Abs, Min, Max, Sqrt, Floor, Ceil...)
- ✅ `Http.*` (Get, Post via curl)
- ✅ Namespaces multi-fichiers, imports

**Samples : 36/37** compilent et tournent (`stdlib_tcp_server` = réseau non supporté)

---

## 🔴 PRIORITÉ 1 — Performance gen_test (BLOQUANT)

**Problème :** `gen_test` génère `amc_lib.c` via une `List<string>` GC.
À -O0 : ~2m30. À -O2 : ~1m. Inacceptable pour le workflow quotidien.

**Fix en cours — mode streaming :**
- `_runtime.h` : `File_OpenWrite` / `File_StreamLine` / `File_CloseWrite` ajoutés
- `c_gen.am` : `Emitter.Streaming` flag
- **Bloquant :** le Resolver ne reconnaît pas `File_StreamLine` (symbole C pur)
- **Fix à faire :** exposer un wrapper Amalgame `File.StreamLine(line: string)`
  ou passer par `Console.WriteError` mappé sur un FILE* global

**Contournement actuel :**
```bash
./build/amc [sources] -o gen_test
gcc -O2 -Isrc/transpiler/runtime gen_test.c -lgc -lm -o gen_test
./gen_test   # ~1min au lieu de 2m30
```

---

## 🟠 PRIORITÉS COMPILATEUR

### P2 — TypeChecker membre resolution
`CheckMemberExpr` retourne `"?"` pour tous les membres.
`FullResolver.Members` (MemberTable) est construit mais pas utilisé.
**Fix :** `this.Symbols.GetMemberType(baseType, memberName)` dans `typechecker.am`.

### P3 — Mode `--lib` testé
Infrastructure prête (`IsLib` flag, `amc_main.c`).
Tester : `./amc --lib mylib.am -o mylib` → `.c` compilable en `.o` sans `main()`.

### P4 — ✅ Archiver `src/core/` Vala (fait)
Sources Vala déplacées dans `archive/vala-bootstrap/src/`.
`./compile.sh` + `./build/amc` toujours rebuildables comme filet de sécurité
(meson lit désormais les sources depuis `archive/vala-bootstrap/`).
`./build_amc.sh` utilise `./amc` (self-host) avec fallback Vala automatique.

### P5 — Améliorer le Resolver (`resolver.am`)
Scope actuel = 2 niveaux plats (global + local).
Pas de nesting (boucles imbriquées, closures capturantes).
Porter le scope chaîné du Resolver Vala.

### P6 — Diagnostics enrichis
- Affichage de la ligne source avec curseur `^`
- Couleurs ANSI restaurées (Ansi.* renvoie les codes `\x1b[...]` réels)
- `DiagnosticFormatter.LoadSource()` déjà en place

### P7 — Bugs langage connus
| Bug | Impact | Fix |
|-----|--------|-----|
| ~~`return ""` fait boucler le parser~~ | ✅ | Plus reproductible, résolu lors d'un commit antérieur |
| ~~`\x` bootstrap circulaire~~ | ✅ | Lexer décode `\xHH` via `String_FromByte()` (runtime) |
| `obj.Field.Method()` type inference | Faible | Variable intermédiaire (contournement OK) |
| `while(ptr != null)` GC | Critique | Déjà contourné par `for i in 0..N` |
| Génériques — pas de vrai type checking | Moyen | TypeChecker P2 |
| Imports non résolus (ignorés) | Moyen | Module system P-longterme |

---

## 🟡 PRIORITÉS LANGAGE

### Features manquantes prioritaires
- [x] **`\x` escape sequences** dans le lexer ✅ (P7)
- [x] **`\u` unicode escape** dans le lexer ✅ (UTF-8 encoded via String_FromCodepoint)
- [ ] **`obj.Method()` syntax** pour strings : `.Length`, `.Contains`, `.Split`, `.Trim`...
- [x] **Guard clauses** : `guard x != null else { return }` ✅ desugars en if-not
- [x] **Is-guards dans match** : `x if x > 0 => ...` ✅ binder + guard via GCC compound expr
- [ ] **Closures capturantes** (actuellement lambdas simples)
- [x] **String multiline `"""`** (triple-quoted) ✅ raw newlines, no escape processing
- [ ] **Décorateurs `@`** : parsing présent, sémantique non implémentée
- [ ] **Inférence générique** : `let xs = List<int>()` → `xs.Add(42)` typé `int`
- [ ] **Interfaces avec génériques** : `IComparable<T>`
- [ ] **`async`/`await`** → coroutines ucontext ou setjmp
- [ ] **Compréhensions de liste** : `[x * 2 for x in xs if x > 0]`
- [x] **`?.` null-safe member access** ✅ field + method call, ternary `(obj ? ... : NULL)`
- [ ] **Spread operator** `...args`
- [ ] **Named arguments** `f(x: 1, y: 2)`

---

## 🟢 PRIORITÉS ÉCOSYSTÈME

### Outillage (`amc` subcommands)
- [ ] `amc fmt` — formateur de code
- [ ] `amc lint` — linter statique
- [ ] `amc doc` — génération de documentation
- [ ] `amc test` — runner de tests intégré
- [ ] `amc build` — build system intégré (remplace compile.sh)
- [ ] `amc add <pkg>` — package manager

### LSP / IDE
- [ ] **LSP** (Language Server Protocol) — complétion, hover, go-to-def, find-refs
  - Protocole JSON-RPC sur stdio
  - Basé sur le TypeChecker/Resolver existant
  - Extensions VS Code + Neovim + Emacs
- [ ] **DAP** (Debug Adapter Protocol) — debugging step-by-step
  - Basé sur debuginfo DWARF (`-g3` déjà présent)
  - Intégration VS Code debugger
- [ ] **Syntax highlighting** — fichiers `.am` 
  - VS Code grammar (TextMate)
  - Tree-sitter grammar
  - Vim/Neovim plugin
  - Emacs major mode
- [ ] **Inlay hints** — affichage des types inférés dans l'IDE
- [ ] **Code actions** — quick-fix suggestions

### Distribution
- [ ] **GitHub Actions CI** — build + tests sur push
- [ ] **GitHub Releases** — binaires Linux/macOS/Windows auto-générés
- [ ] **Homebrew tap** — `brew tap BastienMOUGET/amalgame && brew install amalgame`
  → `install/homebrew/amalgame.rb` prêt
- [ ] **Homebrew-core** — après 100+ stars
- [ ] **AUR** (Arch Linux) — `yay -S amalgame`
- [ ] **`.deb`** (Debian/Ubuntu) — `apt install amalgame`
- [ ] **`.rpm`** (Fedora/RHEL)
- [ ] **Nix/NixOS** — flake
- [ ] **Windows** — Inno Setup `.exe` → `install/windows/amalgame.iss` prêt
- [ ] **winget** — `winget install Amalgame`
- [ ] **Scoop** (Windows)
- [ ] **`install.sh`** universel — déjà prêt

### Documentation
- [ ] **Site web** — docs.amalgame-lang.org
- [ ] **Tour du langage** interactif (comme tour.golang.org)
- [ ] **Grammaire EBNF** à jour → `docs/language/grammar.ebnf` (existant, à mettre à jour)
- [ ] **Référence stdlib** complète
- [ ] **Cookbook** — exemples pratiques

---

## 🏗️ ARCHITECTURE

```
Amalgame/
├── src/
│   ├── amalgame/                  ← Compilateur en Amalgame (ACTIF)
│   │   ├── lexer/
│   │   │   ├── token.am           ← 132 TokenTypes
│   │   │   └── lexer.am
│   │   ├── parser/
│   │   │   ├── ast.am
│   │   │   └── parser.am          ← 1211 lignes
│   │   ├── generator/
│   │   │   ├── c_gen.am           ← 2099 lignes
│   │   │   ├── gen_test.am        ← génère amc_lib.c
│   │   │   └── gen_bootstrap.am   ← génère bootstrap bundles
│   │   ├── resolver/
│   │   │   ├── symbol.am
│   │   │   └── resolver.am        ← FullResolver + MemberTable
│   │   ├── diagnostics.am
│   │   ├── typechecker.am
│   │   ├── main.am
│   │   ├── amc_main.c             ← CLI C
│   │   ├── amc_lib.c              ← GÉNÉRÉ (~7000 lignes)
│   │   └── amc_bootstrap_lib.c    ← GÉNÉRÉ (4221 lignes)
│   └── transpiler/
│       └── runtime/               ← Headers C runtime
├── tests/
│   └── samples/                   ← 37 programmes .am
├── docs/
│   ├── DEVELOPER_GUIDE.md
│   ├── language/
│   │   ├── grammar.ebnf           ← Grammaire EBNF formelle
│   │   └── grammar.md
│   └── more/                      ← Docs v0.1 (référence)
├── install/
│   ├── PUBLISHING.md              ← Guide publication packages
│   ├── homebrew/amalgame.rb       ← Formula Homebrew
│   ├── windows/amalgame.iss       ← Inno Setup
│   └── install.sh                 ← Installeur universel
├── dist/                          ← Releases packagées
├── build/amc                      ← Compilateur Vala (bootstrap binaire)
├── amc                            ← Compilateur Amalgame self-hosted
└── build_amc.sh                   ← Script rebuild
```

---

## 🔧 COMMANDES DE BUILD

```bash
# Rebuild complet
./build/amc src/amalgame/lexer/token.am src/amalgame/lexer/lexer.am \
            src/amalgame/parser/ast.am src/amalgame/parser/parser.am \
            src/amalgame/generator/c_gen.am src/amalgame/resolver/symbol.am \
            src/amalgame/resolver/resolver.am \
            src/amalgame/diagnostics.am src/amalgame/typechecker.am \
            src/amalgame/main.am src/amalgame/generator/gen_test.am \
            -o gen_test

# Recompiler gen_test en -O2 (IMPORTANT — réduit le temps de génération)
gcc -O2 -Isrc/transpiler/runtime gen_test.c -lgc -lm -o gen_test
./gen_test

gcc -Isrc/transpiler/runtime src/amalgame/amc_lib.c src/amalgame/amc_main.c \
    -lgc -lm -lcurl -o amc

./tests/run_tests.sh 2>&1 | tail -3   # doit afficher PASS: 76 | FAIL: 0
```

---

## 📈 MÉTRIQUES

| Métrique | Valeur |
|----------|--------|
| Tests (run_tests.sh) | **76/76 ✅** |
| Samples compilés | **36/37 ✅** |
| Self-hosting | **✅ validé** |
| Warnings gcc | **0 ✅** |
| `amc_lib.c` généré | ~7000 lignes |
| Temps de rebuild | ~3min total (dont ~1min gen_test -O2) |
| `c_gen.am` | 2099 lignes |
| `parser.am` | 1211 lignes |

---

## 🗺️ VERSIONS

### v1.0 — Compilateur stable (en cours)
- [x] Self-hosting validé
- [x] 76/76 tests
- [x] Opérateurs bitwise + compound assigns
- [x] FullResolver + TypeChecker portés
- [ ] Fix performance gen_test (streaming)
- [ ] TypeChecker membre resolution
- [ ] Mode `--lib` testé
- [ ] Archiver Vala
- [ ] 0 bugs parser connus

### v1.1 — Langage complet
- [ ] `\x` `\u` escapes
- [ ] `obj.Method()` strings
- [ ] Guard clauses
- [ ] Closures capturantes
- [ ] Décorateurs fonctionnels
- [ ] LSP basique (hover + complétion)

### v1.2 — Outillage
- [ ] `amc fmt`, `amc lint`, `amc doc`
- [ ] VS Code extension complète (LSP + DAP + highlighting)
- [ ] Tree-sitter grammar
- [ ] GitHub Actions CI

### v2.0 — Écosystème
- [ ] Package manager `amc add`
- [ ] `async`/`await`
- [ ] Génériques avec vrai type checking
- [ ] Homebrew-core + AUR + .deb
- [ ] Site web + tour interactif
- [ ] Cross-compilation (ARM, WASM)

### v3.0 — Production
- [ ] Renommer `code_string`/`code_bool` → `amc_string`/`amc_bool`
- [ ] Backend LLVM (optionnel, remplace GCC)
- [ ] Module system avec cache binaire
- [ ] Standard library étendue (JSON, regex, datetime, crypto)
