# Amalgame — Roadmap Complet & Vision Globale
## Document de continuation — Mai 2026

---

## État actuel : v0.9.4

### Ce qui est FAIT ✅

#### Phase 1 — Transpileur MVP (Vala) — COMPLET
- Lexer, Parser, AST, Resolver, TypeChecker, CGenerator (tous en Vala)
- 126/126 tests passent
- Namespaces, library mode, multi-fichiers
- Distribution scripts

#### Phase 2 — Écosystème stdlib — COMPLET
- Runtime C (GC, allocations)
- Stdlib : IO (Console, File, Path), Math, String, Collections (List, Map, Set), Net (Http, TCP, UDP)
- Enums simples + riches, interfaces avec vtable dispatch
- Multi-file compilation avec merge AST

#### Phase 2 — Bootstrap (en cours) — v0.9.4
```
src/amalgame/
├── diagnostics.am  ← formatage erreurs
├── main.am         ← entry point compilateur
├── lexer/
│   ├── token.am    ✅ tokenize + compile GCC
│   └── lexer.am    ✅
├── parser/
│   ├── ast.am      ✅ compile GCC
│   └── parser.am   ✅
├── resolver/
│   └── symbol.am   ✅
└── generator/
    └── c_gen.am    ✅ génère C compilable
```

---

## Roadmap détaillé — Ce qui reste à faire

### Court terme : Compléter le bootstrap (v0.9.5 → v1.0.0)

#### v0.9.5 — CGen : type inference
- `let n = Ast.Class(...)` → inférer `AstNode*` (pas `void*`)
- `new List<AstNode>()` → `AmalgameList_new()`
- `List<T>.Get()` → cast correct vers `T*`
- Objectif : `ast.am → C → GCC 0 warnings`

#### v0.9.6 — CGen : corps de méthodes complexes
- `for i in 0..n` → boucle C `for(i64 i=0; i<n; i++)`
- `String_xxx()` → appels stdlib directs
- `Console.WriteError()` → `fprintf(stderr, ...)`
- `args.Length()` + `args.Get(i)` → tableau C
- Objectif : `lexer.am → C → GCC OK`

#### v0.9.7 — Bootstrap complet
- Générer parser.am + symbol.am + c_gen.am + main.am
- Linker tout : `gcc token.o lexer.o ast.o parser.o symbol.o c_gen.o main.o -o amc_bootstrap`
- Test : `./amc_bootstrap hello.am -o hello && ./hello`
- **Milestone : Amalgame compilé par Amalgame pour la 1ère fois !**

#### v0.9.8 — Validation bootstrap
- `amc bootstrap validate` : Stage 1 == Stage 2
- Binaire identique (ou fonctionnellement équivalent)
- Tests : tous les 126 tests passent avec amc_bootstrap

#### v1.0.0 — Release
- Bootstrap stable
- Documentation complète
- Supprimer dépendance Vala de la pipeline principale
- Archiver Vala dans `legacy/` (garder pour référence)
- Nouveau README annonçant le bootstrap

---

### Moyen terme : Outils développeur (v1.1.x)

#### LSP Server (Language Server Protocol)
**Fichier** : `src/amalgame/lsp/lsp_server.am`

Fonctionnalités :
- `textDocument/completion` — autocomplétion
- `textDocument/hover` — type au survol
- `textDocument/definition` — aller à la définition
- `textDocument/diagnostics` — erreurs en temps réel
- `textDocument/formatting` — formatage automatique
- `workspace/symbol` — recherche de symboles

Protocole : JSON-RPC sur stdin/stdout (standard LSP)

Dépend de :
- Bootstrap compiler (pour parser en temps réel)
- Un serveur JSON minimal (à écrire en Amalgame ou C)

#### VSCode Extension
**Repo séparé** : `amalgame-vscode`

- Syntaxe highlighting (TextMate grammar)
- Intégration LSP
- Snippets (class, method, enum, etc.)
- Build tasks
- Débogage (via DAP)

#### DAP Server (Debug Adapter Protocol)
- Breakpoints
- Step over/into/out
- Variables watch
- Call stack

---

### Long terme : Cibles alternatives (v1.2.x+)

#### WASM (WebAssembly)
- Compiler Amalgame → WAT (WebAssembly Text)
- Puis WAT → WASM
- Permettrait du code Amalgame dans le browser
- Dépend : finalisation du générateur C d'abord

#### LLVM IR
- Passer par LLVM pour des optimisations avancées
- Cross-compilation (ARM, RISC-V, etc.)
- LTO (Link Time Optimization)
- Complexité élevée — pour v2.0+

#### .NET / IL
- Interop avec l'écosystème .NET
- Moindre priorité

---

### Fonctionnalités langage à compléter (v1.1.x)

#### Limitations actuelles à corriger
1. `AstNode?` comme return type → pas supporté → utiliser sentinel
2. `super()` → non supporté (héritage limité)
3. `&&`/`||` multilignes → seulement dans `(...)`
4. `return null` dans méthode non-nullable → TypeChecker bloque
5. `;` non supporté pour multiple stmts sur une ligne
6. Generics dans classes utilisateur (pas seulement stdlib)
7. TypeChecker forward refs (méthodes déclarées avant utilisation)
8. `string ==` sur retour de méthode directement

#### Features planifiées
- **Async/Await** : `async`, `await`, `Task<T>` (inspiré C#)
- **Pipeline operator** : `x |> f |> g` (inspiré F#/Elixir)
- **Extension methods** : `extend String { ... }`
- **Decorators** : `@deprecated`, `@inline`, etc.
- **Result/Option** : `Result<T, E>`, `Option<T>`
- **Generics complets** : `class Stack<T> { ... }` avec contraintes
- **Pattern matching avancé** : guards, nested, wildcard `_`
- **Type aliases** : `type UserId = string`
- **Computed properties** : `get/set`
- **Operator overloading**

---

### Package Manager (v1.1.x)

**Fichier** : `src/amalgame/pkg/pkg_manager.am`

Déjà commencé en Vala (`src/pkg/package_manager.vala`).
À réécrire en Amalgame.

Fonctionnalités :
- `amc pkg add nom/package`
- `amc pkg remove nom/package`
- `amc pkg list`
- `amc pkg publish`
- Registry central (GitHub-based ou custom)
- `amalgame.json` — manifest de projet
- Dépendances transitives
- Versioning sémantique

---

## Question : Refaire le repository ?

### Recommandation : NON pour l'instant

Stratégie en 3 étapes :

1. **Maintenant** : Merger `feature/bootstrap` → `develop` → `main`
2. **v1.0** : Branche `feature/pure-amalgame` — retirer Vala progressivement
3. **v2.0** : Archiver Vala dans `legacy/` — le compilateur principal est en Amalgame

### Si tu veux vraiment repartir à zéro (v2.0+)
- Nouveau repo : `amalgame-lang` ou `amalgame2`
- Copier uniquement :
  - `src/amalgame/` (le bootstrap)
  - `src/transpiler/runtime/` (headers C)
  - `tests/` (test suite)
  - `docs/` (documentation)
- Garder l'ancien en lecture seule comme archive

---

## Structure finale prévue du projet

```
Amalgame/
├── src/
│   ├── amalgame/          ← compilateur en Amalgame (bootstrap)
│   │   ├── main.am        ← entry point
│   │   ├── diagnostics.am ← formatage erreurs
│   │   ├── lexer/         ← tokenisation
│   │   ├── parser/        ← parsing AST
│   │   ├── resolver/      ← résolution symboles
│   │   ├── typechecker/   ← vérification types (à écrire)
│   │   ├── generator/     ← génération C
│   │   ├── lsp/           ← LSP server (futur)
│   │   └── pkg/           ← package manager (futur)
│   ├── transpiler/
│   │   └── runtime/       ← headers C (_runtime.h, stdlib headers)
│   └── legacy/            ← ancienne implem Vala (archivée)
│       ├── core/
│       └── transpiler/
├── tests/                 ← suite de tests .am
├── docs/                  ← documentation
├── bootstrap/             ← binaire bootstrap stable
├── install/               ← scripts d'installation
└── .github/               ← CI/CD workflows
```

---

## Pour reprendre dans une nouvelle conversation

```
Je travaille sur le langage de programmation Amalgame.
Transpileur Amalgame → C, écrit d'abord en Vala, en cours de bootstrap en Amalgame.

État : branche feature/bootstrap, version v0.9.4, 126/126 tests PASS.

Fichiers clés :
- src/amalgame/ : le compilateur bootstrap en cours
- src/core/ + src/transpiler/ : compilateur Vala fonctionnel
- build/amc : compilateur actuel (à rebuilder avec ./compile.sh)

Dernière session :
- CGen (c_gen.am) génère token.am et ast.am → C → GCC OK
- main.am et diagnostics.am créés (pas encore compilés/testés)
- CONTINUATION.md et ROADMAP_COMPLET.md à la racine du repo

Prochaine étape : v0.9.5 — améliorer CGen pour type inference
(ast.am → C sans warnings, puis lexer.am → C compilable)
```

