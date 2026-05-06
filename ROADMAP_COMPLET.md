# Amalgame — Roadmap Complète

> Dernière mise à jour : mai 2026  
> État : **Bootstrap self-hosted validé** · 76/76 tests · `amc` compile ses propres sources

---

## ✅ PHASES TERMINÉES

### Phase 1 — Compilateur Vala (legacy, archivé)
- Lexer, Parser, Resolver, TypeChecker complets en Vala
- CGen (transpileur Amalgame → C) en Vala
- 126/126 tests unitaires passent
- Runtime C : `_runtime.h`, `Amalgame_String.h`, `Amalgame_Collections.h`,
  `Amalgame_IO.h`, `Amalgame_Math.h`, `Amalgame_Net.h`, `Amalgame_Console.h`
- **Archivé dans `archive/vala-bootstrap/`** — conservé comme filet de sécurité

### Phase 2 — Bootstrap Amalgame-en-Amalgame (TERMINÉ)

**Compilateur self-hosted `amc` :**
```
src/amalgame/
├── lexer/token.am       — 132 tokens (+ bitwise, compound assigns)
├── lexer/lexer.am       — 258 lignes
├── parser/ast.am        — NodeKind, AstNode
├── parser/parser.am     — 1211 lignes (Pratt parser complet)
├── generator/c_gen.am   — 2099 lignes (CGen Amalgame → C)
├── generator/gen_test.am — génère amc_lib.c en mode streaming
├── resolver/symbol.am   — SymbolTable, Symbol
├── resolver/resolver.am — FullResolver (777 lignes)
├── diagnostics.am       — DiagnosticFormatter
├── typechecker.am       — TypeChecker (783 lignes)
└── main.am              — AmalgameCompiler.Run()
```

**Métriques actuelles :**
| Fichier | Lignes |
|---------|--------|
| `amc_lib.c` (généré) | ~7000 |
| `c_gen.am` | 2099 |
| `parser.am` | 1211 |
| `resolver.am` | 777 |
| `typechecker.am` | 783 |
| Total `.am` compilateur | ~8500 |

**Self-hosting validé :**
```bash
./amc src/amalgame/**/*.am -o /tmp/amc_self
gcc -Isrc/transpiler/runtime /tmp/amc_self.c -lgc -lm -lcurl -o /tmp/amc2
diff <(./amc tests/samples/hello.am -o /tmp/v1 && cat /tmp/v1.c) \
     <(/tmp/amc2 tests/samples/hello.am -o /tmp/v2 && cat /tmp/v2.c)
# → vide : output identique ✅
```

**Features du langage supportées :**
- ✅ Variables `let`/`var`, types primitifs (int, float, double, string, bool)
- ✅ Classes, héritage, interfaces, data classes, records
- ✅ Enums simples + enums algébriques (tagged unions)
- ✅ Génériques `T → void*`
- ✅ Closures/lambdas (`x => x * 2`)
- ✅ Tuples : `(int, string)`, literals, destructuring `let (a, b) = f()`
- ✅ Match/Pattern matching : valeurs, ranges `75..99`, variants algébriques, wildcard `_`
- ✅ String interpolation `"hello {name}"`, multiline strings
- ✅ Collections : `List<T>`, `Map<K,V>`, `Set<T>`
- ✅ Stdlib : String, IO, Math, Net (curl), Console, File, Path
- ✅ Namespaces multi-fichiers
- ✅ Try/catch/throw/finally
- ✅ Null safety (`T?` types, `??` coalescing)
- ✅ for-in, while, if/else (one-liners inclus)
- ✅ **Opérateurs bitwise** : `&` `|` `^` `~` `<<` `>>`
- ✅ **Compound assigns** : `+=` `-=` `*=` `/=` `%=` `&=` `|=` `^=` `<<=` `>>=`
- ✅ Pipeline `|>`
- ✅ 36/37 samples compilent et tournent

---

## 🔧 PRIORITÉS EN COURS

### Priorité 1 — Performance gen_test (BLOQUANT)
**Problème :** `gen_test` génère `amc_lib.c` (~7000 lignes) via une `List<string>` GC.
À -O0 (défaut Vala amc) : ~2m30. À -O2 : ~1m. Inacceptable.

**Fix en cours :** mode streaming dans l'Emitter — `EmitLine` écrit directement
sur `FILE*` via `File_StreamLine()` au lieu d'accumuler en mémoire.

**Commande temporaire (contournement) :**
```bash
./build/amc [sources] -o gen_test          # génère gen_test.c avec -O0
gcc -O2 -Isrc/transpiler/runtime gen_test.c -lgc -lm -o gen_test  # recompile en -O2
./gen_test                                 # ~1min au lieu de 2m30
```

### Priorité 2 — Résolution membres TypeChecker
`CheckMemberExpr` retourne `"?"` pour tous les accès membres.
Le `FullResolver.Members` (MemberTable) est construit en Pass 1
mais pas encore utilisé par le TypeChecker.

**Fix :** dans `typechecker.am`, `CheckMemberExpr` doit appeler
`this.Symbols.GetMemberType(baseType, memberName)`.

### Priorité 3 — Mode `--lib`
`amc --lib file.am` compile sans émettre de `main()`.
Infrastructure prête dans `main.am` (`IsLib` flag).
Manque : la détection du flag dans `amc_main.c` (déjà fait) +
tester que la sortie est bien une bibliothèque linkable.

### Priorité 4 — Supprimer `src/core/` Vala
Le code Vala n'est plus nécessaire. À archiver :
```bash
git mv src/core/ archive/vala-bootstrap/src/core/
git mv src/main.vala archive/vala-bootstrap/
```
Conserver `build/amc` binaire comme filet de sécurité.

### Priorité 5 — Porter le Resolver Vala → améliorer `resolver.am`
Le `FullResolver` actuel est un scope 2-niveaux (global + local).
Pas de nesting réel (boucles imbriquées, closures, etc.).
Porter le resolver Vala complet avec scope chaîné.

### Priorité 6 — Diagnostics enrichis (source display + ANSI)
`diagnostics.am` a les stubs. Activer :
- Affichage de la ligne source avec curseur `^`
- Couleurs ANSI (après fix `\x` dans le lexer)

---

## 🏗️ ARCHITECTURE

```
Amalgame/
├── src/
│   ├── amalgame/              ← Compilateur en Amalgame (ACTIF)
│   │   ├── lexer/
│   │   │   ├── token.am       ← 132 TokenTypes
│   │   │   └── lexer.am
│   │   ├── parser/
│   │   │   ├── ast.am         ← NodeKind + AstNode
│   │   │   └── parser.am      ← 1211 lignes
│   │   ├── generator/
│   │   │   ├── c_gen.am       ← 2099 lignes
│   │   │   ├── gen_test.am    ← génère amc_lib.c (streaming)
│   │   │   └── gen_bootstrap.am ← génère bootstrap bundles (rapide)
│   │   ├── resolver/
│   │   │   ├── symbol.am      ← SymbolTable
│   │   │   └── resolver.am    ← FullResolver + MemberTable
│   │   ├── diagnostics.am     ← DiagnosticFormatter
│   │   ├── typechecker.am     ← TypeChecker
│   │   ├── main.am            ← AmalgameCompiler (--lib, --check, exit codes)
│   │   ├── amc_main.c         ← CLI C (--lib --check --color --quiet --version)
│   │   ├── amc_lib.c          ← GÉNÉRÉ (~7000 lignes)
│   │   └── amc_bootstrap_lib.c ← GÉNÉRÉ (4221 lignes)
│   └── transpiler/
│       └── runtime/           ← Headers C partagés
│           ├── _runtime.h     ← AmalgameList, File_WriteLines, File_StreamLine
│           ├── Amalgame_String.h
│           ├── Amalgame_Collections.h
│           ├── Amalgame_IO.h
│           ├── Amalgame_Math.h
│           ├── Amalgame_Net.h
│           └── Amalgame_Console.h
├── tests/
│   └── samples/               ← 37 fichiers .am de test
├── build/amc                  ← Compilateur Vala (bootstrap, binaire)
├── amc                        ← Compilateur Amalgame self-hosted
└── build_amc.sh               ← Script de rebuild complet
```

---

## 🔧 COMMANDES DE BUILD

### Rebuild complet (nouveau workflow)
```bash
# Générer les bootstrap bundles (rapide ~5s)
./build/amc src/amalgame/lexer/token.am src/amalgame/lexer/lexer.am \
            src/amalgame/parser/ast.am src/amalgame/parser/parser.am \
            src/amalgame/generator/c_gen.am src/amalgame/resolver/symbol.am \
            src/amalgame/resolver/resolver.am \
            src/amalgame/diagnostics.am src/amalgame/typechecker.am \
            src/amalgame/main.am src/amalgame/generator/gen_test.am \
            -o gen_test

# Recompiler avec -O2 pour la vitesse
gcc -O2 -Isrc/transpiler/runtime gen_test.c -lgc -lm -o gen_test
./gen_test

# Compiler amc
gcc -Isrc/transpiler/runtime src/amalgame/amc_lib.c src/amalgame/amc_main.c \
    -lgc -lm -lcurl -o amc
```

### Tests
```bash
./tests/run_tests.sh 2>&1 | tail -3   # 76/76 tests Vala
./amc tests/samples/hello.am -o /tmp/hello && gcc -Isrc/transpiler/runtime \
    /tmp/hello.c -lgc -lm -o /tmp/hello_bin && /tmp/hello_bin
```

### Self-hosting check
```bash
./amc src/amalgame/lexer/token.am src/amalgame/lexer/lexer.am \
      src/amalgame/parser/ast.am src/amalgame/parser/parser.am \
      src/amalgame/generator/c_gen.am src/amalgame/resolver/symbol.am \
      src/amalgame/resolver/resolver.am src/amalgame/diagnostics.am \
      src/amalgame/main.am -o /tmp/amc_self
gcc -Isrc/transpiler/runtime /tmp/amc_self.c src/amalgame/amc_main.c \
    -lgc -lm -lcurl -o /tmp/amc2
/tmp/amc2 tests/samples/hello.am -o /tmp/hello_v2
```

---

## 📈 MÉTRIQUES

| Métrique | Valeur |
|----------|--------|
| Tests Vala (run_tests.sh) | **76/76 ✅** |
| Samples compilés | **36/37 ✅** (stdlib_tcp_server = réseau) |
| Self-hosting | **✅ validé** (output identique) |
| `amc_lib.c` généré | ~7000 lignes |
| `c_gen.am` | 2099 lignes |
| `parser.am` | 1211 lignes |
| `resolver.am` | 777 lignes |
| `typechecker.am` | 783 lignes |
| Opérateurs supportés | +bitwise +compound assigns |
| Warnings gcc | **0** |

---

## 🗺️ ROADMAP MOYEN TERME

### v0.2 — Compilateur stable
- [ ] Fix performance gen_test (streaming CGen)
- [ ] TypeChecker membre resolution via MemberTable
- [ ] Mode `--lib` testé et documenté
- [ ] Supprimer `src/core/` Vala
- [ ] Parser : `{ }` empty blocks, multi-line expr

### v0.3 — Langage complet
- [ ] Escapes `\x` `\u` dans le lexer
- [ ] `obj.Method()` syntax pour strings (`.Length`, `.Contains`, `.Split`...)
- [ ] Inferrence de types génériques
- [ ] Import resolution (actuellement ignoré)
- [ ] Diagnostics avec source display + ANSI

### v0.4 — Écosystème
- [ ] Package manager (`amc add`)
- [ ] LSP / VS Code extension complète
- [ ] Stdlib étendue (JSON, regex, datetime)
- [ ] Documentation générée automatiquement

### v1.0 — Production
- [ ] Renommer `code_string`/`code_bool` → `amc_string`/`amc_bool`
- [ ] Optimisation `-O2` par défaut
- [ ] Cross-compilation
- [ ] WASM target
