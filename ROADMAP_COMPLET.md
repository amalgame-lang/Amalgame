# Amalgame — Roadmap Complète

## ✅ JALONS ATTEINTS

### Phase 1 — Langage Vala (TERMINÉ)
- Lexer, Parser, Resolver, TypeChecker en Vala
- CGen (transpileur Amalgame → C) en Vala
- 126/126 tests unitaires passent
- Runtime C : `_runtime.h`, `Amalgame_String.h`, `Amalgame_Collections.h`, `Amalgame_IO.h`, `Amalgame_Math.h`, `Amalgame_Net.h`, `Amalgame_Console.h`

### Phase 2 — Bootstrap (TERMINÉ)
- **`amc_bootstrap`** : compilateur Amalgame écrit en Amalgame (via Vala amc)
  - Lexer en Amalgame (`lexer.am`)
  - Parser en Amalgame (`parser.am`) — 1154 lignes
  - CGen en Amalgame (`c_gen.am`) — 2000+ lignes
  - Résultat : `amc_bootstrap_lib.c` (4317 lignes)

- **`amc`** : compilateur final Amalgame (compiler Amalgame avec amc_bootstrap)
  - `main.am` — AmalgameCompiler.Run()
  - `diagnostics.am` — messages d'erreur
  - `resolver/symbol.am` — symbol table
  - Résultat : `amc_lib.c` (4913 lignes)

- **Pipeline** :
  ```
  .am sources → build/amc → gen_test → amc_bootstrap_lib.c + amc_lib.c
  amc_bootstrap_lib.c + amc_bootstrap_main.c → gcc → amc_bootstrap
  amc_lib.c + amc_main.c → gcc → amc
  ```

### Phase 2 — Features implémentées dans le CGen bootstrap
- ✅ Variables, types primitifs (int, float, string, bool)
- ✅ Classes, héritage, interfaces, data classes, records
- ✅ Enums simples + **enums algébriques (tagged unions)**
- ✅ Génériques `T → void*`
- ✅ Closures/lambdas (`x => x * 2` via macro C)
- ✅ **Tuples** : return types `(int, string)`, literals `(a, b)`, destructuring `let (a, b) = f()`
- ✅ **Match/Pattern matching** : valeurs, ranges `75..99`, variants algébriques, wildcard `_`
- ✅ String interpolation `"hello {name}"`
- ✅ Collections : `List<T>`, `Map<K,V>`, `Set<T>`
- ✅ Stdlib : String, IO, Math, Net (avec curl), Console
- ✅ Namespaces multi-fichiers
- ✅ Try/catch (basique)
- ✅ Null safety (`?` types)
- ✅ for-in loops, while, if/else
- ✅ **35/37 samples compilent et s'exécutent** (+ 1 lib = 36/37)

### Bilan samples
```
✅ 34 exécutables compilent et tournent correctement
📦  1 bibliothèque (library.am)  
❌  2 limitations connues :
    - pattern_advanced : string patterns, is-guards
    - stdlib_tcp_server : réseau TCP (test de connectivité)
```

---

## 🔄 EN COURS / PROCHAINES ÉTAPES

### Priorité 1 — Corrections mineures
- [ ] Fixer `pattern_advanced` (string patterns dans match)
- [ ] Warning `Token_ToString` : pointer/integer mismatch (Token.Type enum)
- [ ] Warning curl `const char*` dans `Amalgame_Net.h:148`
- [ ] Mode `--lib` dans `amc` pour compiler des bibliothèques sans `main()`

### Priorité 2 — Self-hosting complet
- [ ] **`amc` se compile lui-même** sans `amc_bootstrap`
  ```bash
  ./amc src/amalgame/**/*.am -o /tmp/amc_self
  gcc -Isrc/transpiler/runtime /tmp/amc_self.c -lgc -lm -lcurl -o /tmp/amc2
  ./amc2 tests/samples/hello.am -o /tmp/hello && gcc ... && ./hello
  ```
- [ ] Vérifier que `amc2` produit le même output que `amc`

### Priorité 3 — TypeChecker en Amalgame
- [ ] Porter `src/core/analyzer/typechecker.vala` (1221 lignes) → `src/amalgame/typechecker.am`
- [ ] Types inférés correctement (pas de `void*` partout)
- [ ] Erreurs de type au compile-time

### Priorité 4 — Resolver complet en Amalgame
- [ ] Porter `src/core/analyzer/resolver.vala` (1187 lignes) → `src/amalgame/resolver/resolver.am`
- [ ] `src/amalgame/resolver/symbol.am` existe déjà (370 lignes)
- [ ] Résolution de symboles cross-fichiers

### Priorité 5 — Supprimer le Vala
- [ ] Remplacer `build/amc` par `./amc` partout
- [ ] Supprimer `src/core/` (Vala amc)
- [ ] Supprimer Vala comme dépendance

### Priorité 6 — Features avancées du langage
- [ ] **String patterns dans match** : `"hello" => ...`
- [ ] **Is-guards** : `x if x > 0 => ...`
- [ ] **Closures capturantes** (actuellement lambdas simples via macro)
- [ ] **Multi-return natif** (tuples avec vrai TypeChecker)
- [ ] **Interfaces avec génériques** : `IComparable<T>`
- [ ] **Async/await** (future)
- [ ] **Modules/packages** système

### Priorité 7 — Outillage
- [ ] LSP (Language Server Protocol) pour IDEs
- [ ] Formatter (`amc fmt`)
- [ ] Linter (`amc lint`)
- [ ] Package manager (`amc add`)

---

## 🏗️ ARCHITECTURE ACTUELLE

```
src/
├── amalgame/           ← Compilateur en Amalgame (ACTIF)
│   ├── lexer/
│   │   ├── token.am
│   │   └── lexer.am
│   ├── parser/
│   │   ├── ast.am
│   │   └── parser.am   (1154 lignes)
│   ├── generator/
│   │   ├── c_gen.am    (2000+ lignes)
│   │   └── gen_test.am (génère les libs C)
│   ├── resolver/
│   │   └── symbol.am   (370 lignes)
│   ├── diagnostics.am
│   ├── main.am         (AmalgameCompiler.Run())
│   ├── amc_bootstrap_lib.c  (4317 lignes - généré)
│   ├── amc_bootstrap_main.c (entry point C bootstrap)
│   ├── amc_lib.c            (4913 lignes - généré)
│   └── amc_main.c           (entry point C amc)
├── core/               ← Compilateur en Vala (LEGACY)
│   ├── analyzer/
│   │   ├── resolver.vala    (1187 lignes)
│   │   ├── symbol.vala
│   │   └── typechecker.vala (1221 lignes)
│   ├── lexer/
│   ├── parser/
│   └── ...
└── transpiler/
    └── runtime/        ← Headers C partagés
        ├── _runtime.h
        ├── Amalgame_String.h
        ├── Amalgame_Collections.h
        ├── Amalgame_IO.h
        ├── Amalgame_Math.h
        ├── Amalgame_Net.h
        └── Amalgame_Console.h

tests/
└── samples/            ← 37 fichiers .am de test
    ├── variables.am, hello.am, ...
    └── bootstrap_probe.am  ← test complet features avancées
```

## 🔧 COMMANDES DE BUILD

```bash
# Rebuild complet (amc_bootstrap + amc)
./build/amc src/amalgame/lexer/token.am \
            src/amalgame/lexer/lexer.am \
            src/amalgame/parser/ast.am \
            src/amalgame/parser/parser.am \
            src/amalgame/generator/c_gen.am \
            src/amalgame/generator/gen_test.am \
            -o gen_test && timeout 60 ./gen_test

gcc -Isrc/transpiler/runtime \
    src/amalgame/amc_bootstrap_lib.c \
    src/amalgame/amc_bootstrap_main.c \
    -lgc -lm -lcurl -o amc_bootstrap

gcc -Isrc/transpiler/runtime \
    src/amalgame/amc_lib.c \
    src/amalgame/amc_main.c \
    -lgc -lm -lcurl -o amc

# Tests
./tests/run_all_tests.sh 2>&1 | tail -3

# Bilan samples
pass=0; fail=0; lib=0
for sample in tests/samples/*.am; do
    name=$(basename ${sample%.am})
    timeout 5 ./amc ${sample} -o /tmp/bilan_${name} 2>/dev/null || { fail=$((fail+1)); continue; }
    if grep -q "Program_Main" /tmp/bilan_${name}.c 2>/dev/null; then
        gcc -Isrc/transpiler/runtime /tmp/bilan_${name}.c \
            -lgc -lm -lcurl -o /tmp/bilan_${name}_bin 2>/dev/null && \
        pass=$((pass+1)) || fail=$((fail+1))
    else
        gcc -Isrc/transpiler/runtime -c /tmp/bilan_${name}.c \
            -o /tmp/bilan_${name}.o 2>/dev/null && \
        lib=$((lib+1)) || fail=$((fail+1))
    fi
done
echo "✅ $pass exécutables, 📦 $lib bibliothèques, ❌ $fail échecs"
```

## 📈 MÉTRIQUES

| Métrique | Valeur |
|----------|--------|
| Tests Vala | 126/126 ✅ |
| Samples compilés | 35/37 ✅ |
| Lignes amc_lib.c | 4913 |
| Lignes c_gen.am | ~2000 |
| Lignes parser.am | ~1154 |
| Lignes total Amalgame | ~5000 |

