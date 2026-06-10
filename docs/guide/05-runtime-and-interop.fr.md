# 5 · Runtime & interopérabilité C

## Modèle mémoire

Amalgame utilise **bdwgc** (le ramasse-miettes Boehm-Demers-Weiser) pour
toutes les allocations dynamiques. Le GC est un collecteur conservateur
mark-sweep qui réside dans `runtime/_runtime.h` (`#include <gc.h>`), et
est initialisé par le `int main()` généré :

```c
int main(int argc, char** argv) {
    GC_INIT();
    code_runtime_init_args(argc, argv);
    App_Program_Main((code_string*)argv);
    return code_exit_code;
}
```

Implications :

- **Pas d'appels `free`/`destroy`.** Toutes les allocations passent par `GC_MALLOC`.
  Le GC récupère automatiquement la mémoire inaccessible.
- **Pas encore de destructeurs déterministes.** Si vous avez besoin d'un `try { } finally`,
  utilisez la forme explicite `try`/`finally` — les fichiers et sockets ne sont pas
  fermés automatiquement.
- **Balayage conservateur** — des valeurs qui ressemblent à des pointeurs sans en être
  peuvent maintenir de la mémoire en vie. Évitez de stocker des secrets dans de grands
  tampons à longue durée de vie si cela vous importe.
- **Le coût d'allocation** est amorti — `GC_MALLOC` est de l'ordre de quelques
  centaines de nanosecondes sur Linux.

## Correspondance des types (Amalgame → C)

| Amalgame             | C                                                |
| -------------------- | ------------------------------------------------ |
| `int`                | `i64` (signé 64 bits)                            |
| `float`              | `double`                                         |
| `bool`               | `code_bool` (alias de `bool`)                    |
| `string`             | `code_string` (alias de `char*`)                 |
| `void`               | `void`                                           |
| `T?`                 | `T*` (pointeur nullable)                         |
| `T[]`                | `T*` (le tableau se dégrade en pointeur ; taille fournie par l'appelant) |
| `List<T>`            | `AmalgameList*`                                  |
| `Map<K,V>`           | `AmalgameMap*`                                   |
| `Set<T>`             | `AmalgameSet*`                                   |
| `class Foo` utilisateur     | `Foo*` (toujours alloué sur le tas)       |
| `enum Foo` simple    | `Foo` (enum C) — non-pointeur                    |
| `enum Foo` algébrique | `Foo` (struct union taguée) — type valeur       |

Les paramètres génériques (`T`) sont effacés en `void*`. Les valeurs
primitives (`int`, `bool`) sont transmises via des casts
`(void*)(intptr_t)` lors des passages aux interfaces génériques — il
n'existe pas d'objet de boxing.

Les noms de symboles C sont dérivés de `Namespace.Class.Method` →
`Namespace_Class_Method`. Les méthodes statiques et les méthodes
d'instance partagent le même schéma de nommage ; les méthodes d'instance
prennent `self` comme premier paramètre.

## Closures et appels d'ordre supérieur

Une lambda est compilée en une fonction C de premier niveau plus une
valeur `AmalgameClosure { fn, env }`. `_runtime.h` expose la structure
et trois wrappers d'appel :

```c
AmalgameClosure_call1(c, arg)
AmalgameClosure_call2(c, a, b)
AmalgameClosure_call3(c, a, b, d)
```

Les méthodes d'ordre supérieur de `List<T>` (`Filter`, `Map`, `Reduce`,
`ForEach`, `Any`, `All`, `CountIf`) sont implémentées sous forme d'inlines
statiques dans `_runtime.h` qui dispatchent via `_call1` (ou `_call2` pour
le réducteur `(acc, x) → acc` de `Reduce`). Au niveau du site d'appel
Amalgame, le CGen émet une compound-statement-expression GCC qui alloue
l'environnement, y copie les locaux capturés, et produit une closure
fraîche :

```c
AmalgameList_filter(xs, ({
    LamEnv_0* __env_0 = (LamEnv_0*)code_alloc(sizeof(LamEnv_0));
    __env_0->_n = n;             /* one line per capture */
    AmalgameClosure_new((void*)lam_0_fn, __env_0);
}))
```

Les éléments, arguments et résultats des lambdas sont tous des `void*`
boxés à la frontière de l'ABI C ; le site d'appel déboxe via
`(i64)(intptr_t)…` pour la forme i64. Les signatures non-int (p. ex. une
lambda retournant un `code_string`) nécessitent la couche de typage de
lambda qui est prévue pour la prochaine version.

## Appeler Amalgame depuis C

Compilez votre bibliothèque avec `--lib` et liez le `.o` :

```kotlin
// greeter.am
namespace MyLib

public class Greeter {
    public Name: string
    public Greeter(string name) { this.Name = name }
    public string Hello() { return "Hello, " + this.Name + "!" }
}
```

```bash
amc --lib greeter.am -o greeter
gcc -Iruntime -c greeter.c -o greeter.o
```

Consommateur C :

```c
#include "_runtime.h"
#include "Amalgame_String.h"

typedef struct _MyLib_Greeter MyLib_Greeter;
extern MyLib_Greeter* MyLib_Greeter_new(code_string name);
extern code_string MyLib_Greeter_Hello(MyLib_Greeter* self);

int main(void) {
    code_runtime_init();
    MyLib_Greeter* g = MyLib_Greeter_new("World");
    printf("%s\n", MyLib_Greeter_Hello(g));
    return 0;
}
```

```bash
gcc -Iruntime consumer.c greeter.o -lgc -lm -o app
./app
# Hello, World!
```

Un test e2e opérationnel se trouve dans `tests/samples/lib_e2e.am` /
`lib_e2e_consumer.c` et s'exécute dans le cadre de `tests/core_bundle/core_test.am`
(invoquez avec `./amc test ./tests/core_bundle/`).

## Appeler C depuis Amalgame

Il n'existe pas de mot-clé FFI aujourd'hui. Deux approches pragmatiques :

### 1. Ajouter la fonction au header runtime

Déposez votre déclaration dans un header sous `runtime/` et déclarez-la
comme builtin dans le resolver :

```c
// runtime/Amalgame_String.h
static inline i64 String_DamerauLevenshtein(code_string a, code_string b) {
    /* … your impl … */
}
```

```kotlin
// src/resolver/resolver.am — RegisterBuiltins()
this.DeclareGlobal("String_DamerauLevenshtein", "int", false)
```

Vous pouvez désormais appeler `String.DamerauLevenshtein(a, b)` depuis
n'importe quel fichier `.am`.

C'est exactement ainsi que fonctionne la stdlib en interne.

### 2. Inliner un site d'appel C via le corps d'une méthode

Moins élégant, mais fonctionnel pour des bindings ponctuels : placez
l'utilitaire dans le header runtime, puis écrivez une fine classe Amalgame
qui ne fait que transférer. Le CGen émettra des appels C normaux.

### 3. Blocs C inline — `@c { … }` (v0.7.4+)

Depuis la v0.7.4 (projet G), Amalgame dispose d'un bloc C inline balisé
qui permet au corps d'une méthode de plonger directement en C sans passer
par un détour de header runtime. Les octets du corps traversent le lexer
de manière opaque jusqu'au `}` correspondant, le resolver / typechecker /
linter traitent le nœud comme opaque, et le cgen splice la source
verbatim à l'intérieur d'une instruction composée (`{ … }`) afin que
toute variable locale déclarée dans le bloc reste limitée à sa portée.

```kotlin
namespace App

public class CTools {
    public static int CLen(string s) {
        @c {
            return (int) strlen(s);
        }
    }

    public static int Doubled(int x) {
        @c {
            int local = x;
            return local + local;
        }
    }
}
```

Deux **directives de portée fichier** complémentaires sont reconnues en
tête de tout fichier `.am`, avant ou entre les déclarations de classes :

- **`@c_include "<header.h>"`** — émet le `#include` C correspondant en
  haut du `.c` généré. Forme avec chevrons si l'argument commence par `<`,
  forme avec guillemets sinon :
  ```kotlin
  @c_include "<ctype.h>"      // → #include <ctype.h>
  @c_include "my_local.h"     // → #include "my_local.h"
  ```
  Le prélude runtime tire déjà `<stdio.h>` / `<stdlib.h>` /
  `<string.h>` / `<stdint.h>` / `<math.h>` / `<gc.h>` — ne recourez
  à `@c_include` que pour ce que le prélude n'apporte pas.

- **`@c_link "name"`** — apparaît sous forme de commentaire
  `/* link: -lname */` dans le `.c` émis. Le MVP ne le propage pas
  à l'étape gcc interne d'`amc test` (vous passez toujours `-l<name>`
  vous-même pour les workflows `amc -o`) ; le commentaire est là afin
  que `grep '^/\* link:' yourfile.c` vous donne la ligne de link
  complète.

**Sémantique du corps :**

- Tout paramètre Amalgame en portée est visible à l'intérieur du bloc
  avec sa représentation C (`code_string` / `i64` / pointeur de struct).
- L'instruction C `return` retourne depuis la méthode Amalgame englobante
  (car le splice réside à l'intérieur du corps de la méthode, pas dans
  une fonction imbriquée). Utilisez-la de la même façon qu'un `return`
  Amalgame ordinaire.
- Le corps doit contenir du C valide — le frontend Amalgame ne parse pas
  l'intérieur de `{ … }`. Les chaînes, les littéraux de caractères, les
  commentaires simples et les commentaires de bloc sont reconnus par le
  comptage d'accolades du lexer afin qu'un `}` dans `"abc}def"` / `'}'`
  / `/* … */` ne ferme pas le bloc prématurément.

**Sandbox / sécurité :** il n'y en a pas. Le C inline peut planter le
programme, provoquer des fuites mémoire et violer les invariants du GC.
Traitez `@c { … }` comme vous traiteriez `unsafe { … }` en Rust —
utilisez-le quand vous n'avez pas le choix, documentez pourquoi, et
isolez la surface non sécurisée derrière un wrapper Amalgame typé.

**Blocs `@c { … }` de portée fichier (v0.7.5+).** Une seconde forme de
`@c { … }` réside au niveau supérieur d'un fichier `.am` (entre ou avant
les déclarations de classes). Le corps est splicé dans le `.c` émis en
dehors de toute fonction, ce qui permet à un module de :

- déclarer des globaux d'état à l'échelle du processus (mutex, fd, drapeau d'init) ;
- inclure des headers libc / OS via des lignes `#include` brutes (à la place de
  `@c_include`) ;
- définir des fonctions statiques utilitaires partagées ensuite par plusieurs
  méthodes de classe.

```amalgame
namespace Amalgame.Logging

@c {
    #include <stdio.h>
    static int g_min_level = 1;
    static FILE *g_sink = NULL;
}

public class Log {
    public static void SetMinLevel(s: string) {
        @c { g_min_level = parse_level(s); }
    }
    // … etc.
}
```

C'est la fonctionnalité clé qui a permis l'arc de pureté de la stdlib en
v0.7.6 : les headers runtime sont passés de 18 à 9 tandis que huit modules
— Logging, DateTime, FileWatcher, Service, Random, Crypto, BuildInfo,
Math + Math.Vec — migraient vers des modules purement AM avec des blocs
`@c { … }` pour les primitives liées à l'OS. La v0.7.7 a ensuite déplacé
ces mêmes modules hors de la stdlib embarquée vers des packages externes
sur `amalgame-lang/` — voir le tableau au chapitre 4.

> `@out = expr;` issu de la proposition originale v0.7.4 a été **supprimé**.
> Les corps utilisent le `return …;` C ordinaire vis-à-vis du type de retour
> déclaré de la méthode, ce qui est plus propre et évite une réécriture de
> frontière cachée.

## Notes ABI

- Le compilateur ne **mantle pas** les types dans les noms de symboles — seulement
  le chemin de namespace. Les surcharges ne sont donc pas supportées (deux méthodes
  avec le même `Class.Name` mais des signatures différentes entreraient en conflit
  à l'édition de liens).
- La visibilité public/private correspond à C `static` (private) ou rien
  (public). Les méthodes privées ne peuvent pas être appelées depuis une
  autre unité de traduction.
- Les décorateurs correspondent à des attributs C :
  - `@inline` → mot-clé `inline`
  - `@deprecated` → `__attribute__((deprecated))`

## Chaînes : zéro coût caché

`code_string` est un `char*`. La concaténation passe par
`code_string_concat(a, b)` qui alloue un nouveau tampon GC de
`strlen(a) + strlen(b) + 1`. Passer un littéral est gratuit.

Il n'y a pas de cache de longueur — `String.Length(s)` appelle `strlen(s)` — donc
les boucles chaudes qui demandent la longueur devraient la hisser hors de
la boucle :

```kotlin
let len = String.Length(text)         // O(strlen)
for i in 0..len { /* … */ }            // corps en O(1) par itération
```

## Threading & async — packages externes

Le runtime embarqué est mono-thread par défaut ; le GC est configuré pour
le thread principal uniquement. Deux packages de l'écosystème prennent
en charge la concurrence pour vous éviter d'écrire des blocs `@c { … }`
manuels autour de pthread / ucontext :

- **[`amalgame-threading`](https://github.com/amalgame-lang/amalgame-threading)**
  — threads POSIX + Mutex + Channel borné + `Thread.Spawn /
  Join / Sleep`. Chaque thread spawné est enregistré auprès de bdwgc
  (via `GC_pthread_create`) afin que les locaux restent balayables.
  L'outil idéal pour le travail CPU-bound qui bénéficie du multi-cœur.
- **[`amalgame-async`](https://github.com/amalgame-lang/amalgame-async)**
  — coroutines stackful sur ucontext POSIX, scheduler round-robin
  mono-thread avec channels de parking, `WaitFdReadable` / `WaitFdWritable`
  piloté par epoll (Linux), `FiberCancel` / `IsCancelled` coopératif,
  et un helper ergonomique `WithTimeout(closure, arg, ms)`. L'outil idéal
  pour le travail I/O-bound — voir le benchmark dans
  [docs/proposals/amalgame-async.md](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-async.md)
  montrant 1,5×–9× de débit vs thread-par-connexion sur des charges
  HTTP/1.1.

Ils se composent de manière orthogonale (spawner N threads OS, faire
tourner un scheduler dans chacun — M:N est prévu pour `amalgame-async`
v0.4). Si vous forkez ou spawner des threads POSIX depuis vos propres
blocs `@c { … }` sans passer par l'un de ces packages, vous êtes seul
responsable de la sécurité vis-à-vis de libgc : le wrapper
`GC_pthread_create` est ce qui tient le collecteur informé des piles des
nouveaux threads.
