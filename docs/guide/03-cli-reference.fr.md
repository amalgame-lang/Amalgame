# 3 · Référence CLI

## Synopsis

```
amc [options] file1.am [file2.am ...] -o <output>     # compile
amc <subcommand> [args...]                            # autres commandes
```

Le compilateur émet toujours un fichier `.c`. Pour produire un binaire
natif, enchaînez avec `gcc` :

```bash
amc hello.am -o hello                                    # → hello.c
gcc -Iruntime hello.c -lgc -lm -o hello           # → hello (binaire)
./hello
```

Les en-têtes du runtime se trouvent dans `runtime/` à la racine du projet.

## Sous-commandes

| Commande | Rôle | Référence |
| -------- | ---- | --------- |
| `build [-o <out>] [-g] [-v] <entry.am>` | Compile + lie avec gcc pour produire un binaire exécutable en une seule étape (v0.7.9+). `-g`/`--debug` remplace `-O2` par `-O0 -g` (DWARF pour `amc dap`) depuis la v0.8.0. | `amc build --help` |
| `run [-o <out>] [-g] [-v] <entry.am> [-- args…]` | Compile, puis exécute le binaire (v0.7.9+) | `amc run --help` |
| `watch [-o <out>] [--run] [-v] <entry.am>` | Compile immédiatement, puis surveille le mtime et reconstruit à chaque modification (v0.7.9+) | `amc watch --help` |
| `fmt [-w] file.am`     | Formateur idempotent (sortie sur stdout, ou `-w` en place)        | `amc fmt --help` |
| `test [<dir>]`         | Découvre les `*_test.am`, compile + exécute, agrège les résultats         | `amc test --help` |
| `lsp`                  | Serveur LSP avec connaissance du workspace, via stdio JSON-RPC          | chap. 6 |
| `dap`                  | Proxy DAP via stdio — délègue à `lldb-dap`/`gdb --dap` (v0.8.0+) | chap. 6 |
| `new <name> [--template exe\|lib\|test\|service] [--vscode]` | Crée un nouveau projet. `--vscode` ajoute `.vscode/launch.json` + `settings.json` (v0.8.1+). | `amc new --help` |
| `package <action>` (alias `pkg`) | Gestion des packages du projet (voir ci-dessous)        | `amc package --help` |
| `migrate <file\|dir>`   | Traduction source-vers-Amalgame assistée par LLM               | chap. 8 |
| `generate "<prompt>"`  | Prose-vers-Amalgame assistée par LLM                            | chap. 8 |
| `explain <file.am>`    | Amalgame-vers-prose assistée par LLM                            | chap. 8 |

Chaque sous-commande gère ses propres flags et codes de sortie ; consultez
le chapitre référencé ou `--help` pour la surface complète.

### `amc package <action>` (alias `amc pkg`)

Opérations de packages locales au projet. Toutes les actions lisent/écrivent
`amalgame.toml` (dépendances) et `amalgame.lock` (SHAs Git résolus) dans
le répertoire de travail courant.

| Action | Rôle |
| ------ | ---- |
| `add <name\|url>[@<tag>] [--no-precompile]` | Clone + valide + enregistre la dépendance. Accepte `<name>@<tag>` (nom court résolu via l'index curé) ou une `<git-url>@<tag>` complète. Depuis la v0.6.0, le `@<tag>` est optionnel sur les **noms courts indexés** : amc résout automatiquement le tag le plus récent dont le `required-amalgame` est satisfait par la version d'amc en cours. Les URLs git complètes nécessitent toujours un `@<tag>` explicite. `--no-precompile` (v0.5.4+) saute la compilation au moment de l'installation des `[stdlib].sources` même si le manifeste déclare `precompile = true`. |
| `remove <name>[@<tag>] [...]` | Supprime la ou les dépendances de `amalgame.toml` + `amalgame.lock`. Le suffixe de sécurité `@<tag>` (v0.5.5+) refuse la suppression si le tag installé ne correspond pas — protection contre une faute de frappe sur la mauvaise version après un `update`. |
| `search <keyword> [--refresh]` | Correspondance par sous-chaîne dans les packages indexés. Chaque résultat liste les tags connus avec leur statut de compatibilité (`✓`/`✗` vis-à-vis de l'amc en cours) et un marqueur `← latest compatible`. Le TTL du cache de l'index est de 30 min depuis la v0.5.6 (rafraîchissement automatique) ; `--refresh` force un rechargement immédiat. |
| `versions <name> [--refresh]` | Raccourci : même sortie que `search` filtrée à un seul package. |
| `list` | Affiche les packages installés avec leur tag épinglé (depuis v0.5.5) — format : `<ClassName> @ <tag> — <slug>`. |
| `update <name>@<tag>` | Monte la version d'un tag épinglé (délègue à `add` en interne). |
| `cache clear [--all]` | Supprime le cache de l'index (et les packages avec `--all`). |

`amc test` installe automatiquement toute dépendance manquante avant
d'exécuter la suite, et lie les objets C vendorisés de `[stdlib].sources`
de chaque package dans chaque binaire de test — les backends comme SQLite
« fonctionnent sans configuration » sans flags `gcc` manuels. Depuis la
v0.5.3, les sources `.cpp` / `.cc` / `.cxx` sont compilées avec `g++`
(et la liaison finale du test passe par `g++` quand un package contribue
du C++), ce qui permet aux packages vendorisant une amalgamation C++ dans
le style DuckDB de fonctionner immédiatement.

### Référence du manifeste `amalgame.toml`

Le fichier `amalgame.toml` d'un package décrit à la fois ses métadonnées
(`[package]`) et la surface qu'il apporte aux consommateurs (`[stdlib]`).
Le `amalgame.toml` d'un projet utilisateur ne contient généralement que
`[package]` + `[dependencies]`.

```toml
[package]
name              = "amalgame-database-duckdb"
version           = "0.1.0"
license           = "Apache-2.0"
required-amalgame = ">=0.5.3"   # refuses install on older amc
schema-version    = 1           # refuses install on amc < this schema

[stdlib]
class     = "DuckDB"
header    = "runtime/Amalgame_Database_DuckDB.h"
namespace = "Amalgame.Database.DuckDB"
sources   = ["runtime/Amalgame_Database/duckdb/duckdb.cpp"]
cflags    = ""                  # extra flags for .c sources
cxxflags  = "-O3 -DNDEBUG -std=c++17"  # extra flags for .cpp/.cc/.cxx
libs      = ["stdc++"]          # bare names → -l<name> at link time

[stdlib.functions]
Open     = { returns = "AmalgameDuckDB*" }
Close    = { returns = "void" }
# Optional `returns_generic` (v0.8.40+) carries the AM-level
# generic shape so chained `.Get(i).Get(j)…` on the consumer side
# infers typed casts instead of falling through to `(void*)`.
# Example: `QueryAll` returning rows-of-columns of strings.
QueryAll = { returns = "AmalgameList*", returns_generic = "List<List<string>>" }
# …
```

| Clé                              | Section     | Depuis   | Rôle                                                                    |
| -------------------------------- | ----------- | ------- | -------------------------------------------------------------------------- |
| `name`, `version`, `license`     | `[package]` | v0.5.0 | Champs d'identité obligatoires                                                   |
| `description`, `authors`         | `[package]` | v0.5.0 | Affichés dans `amc package list` / search                                    |
| `required-amalgame`              | `[package]` | v0.5.0 | Contrainte sémantique sur l'amc en cours. Opérateurs (v0.6.0+) : `>=`, `>`, `<=`, `<`, `=`, `^` (caret — style npm, compatible 0.x), `~` (tilde — verrouille `major.minor`), version seule traitée comme `>=`. Ex. : `">=0.5.3"`, `"^0.6.0"`, `"~1.2.3"`. |
| `schema-version`                 | `[package]` | v0.5.3 | Refuse l'installation quand amc supporte un schéma de manifeste inférieur                  |
| `class`, `header`, `namespace`   | `[stdlib]`  | v0.5.0 | Câblage obligatoire pour le cgen + le résolveur                                    |
| `sources`                        | `[stdlib]`  | v0.5.0 | Chemins `.c` / `.cpp` vendorisés à compiler + lier                             |
| `cflags`                         | `[stdlib]`  | v0.5.3 | Flags gcc supplémentaires pour les sources `.c` du package                             |
| `cxxflags`                       | `[stdlib]`  | v0.5.3 | Flags g++ supplémentaires pour les sources `.cpp` / `.cc` / `.cxx` du package          |
| `libs`                           | `[stdlib]`  | v0.5.3 | Noms de bibliothèques bruts → `-l<name>` ajouté à la liaison finale de chaque consommateur        |
| `precompile`                     | `[stdlib]`  | v0.5.4 | Quand `true`, `amc package add` compile les sources à l'installation dans `~/.amalgame/packages/.../build/<platform>/`. Les appels suivants à `amc test`/`amc build` réutilisent le `.o` mis en cache. Contournable avec `--no-precompile`. |
| `[stdlib.functions]`             | section     | v0.5.0 | `<Method> = { returns = "<C-type>" }` — alimente le dispatch du cgen     |
| `returns_generic` (par fonction) | `[stdlib.functions]` | v0.8.40 | Forme générique au niveau AM (ex. `"List<List<string>>"`). Quand présent, les appels chaînés `.Get(i).Get(j)…` côté consommateur infèrent des casts typés (`(code_string)…` au lieu de `(void*)…`). Omettre pour les retours non-collection ou quand un niveau de `void*` est acceptable. |

### Opérateurs `required-amalgame` (v0.6.0+)

La chaîne de contrainte suit les conventions npm/Cargo. Exemples :

| Contrainte   | Correspond à                                                           |
| ------------ | ----------------------------------------------------------------- |
| `">=0.5.3"`  | Au moins 0.5.3 (comportement actuel, rétro-compatible)                   |
| `">0.5.5"`   | Strictement supérieur à 0.5.5                                       |
| `"<=0.6.0"`  | Au plus 0.6.0                                                     |
| `"<1.0.0"`   | Strictement inférieur à 1.0.0                                          |
| `"=0.5.5"`   | Correspondance exacte                                                       |
| `"^1.2.3"`   | `>=1.2.3, <2.0.0` (majeur verrouillé quand majeur > 0)                   |
| `"^0.5.3"`   | `>=0.5.3, <0.6.0` (mineur 0.x verrouillé, règle pré-stable)             |
| `"^0.0.5"`   | `=0.0.5` (0.0.x → patch exact, encore plus strict)                     |
| `"~1.2.3"`   | `>=1.2.3, <1.3.0` (verrouille toujours `major.minor`)                    |
| `"0.5.0"`    | Version seule — traitée comme `>=` pour la rétro-compatibilité avec les manifestes v0.5 |

Les opérateurs non reconnus sont traités comme « pas de contrainte »
(retourne vrai) afin de maintenir la compatibilité des anciens binaires
amc avec les futures extensions de syntaxe.

### Résolution automatique `amc package add <pkg>` (v0.6.0+)

Quand le suffixe `@<tag>` est omis sur un **nom court indexé**,
amc récupère l'index et choisit le tag le plus récent dont le
`required-amalgame` est satisfait par l'amc en cours :

```bash
$ amc package add duckdb
Auto-resolved 'duckdb' → 'duckdb@v0.1.1' (latest compatible with amc 0.6.0)
…
```

Les URLs git complètes nécessitent toujours un `@<tag>` explicite — l'index
est la source de vérité pour « ce qu'amc connaît », et les dépôts non
indexés se désinscrivent de la résolution automatique par conception.

Si aucune version n'est compatible (par exemple, tous les tags indexés
requièrent un amc plus récent que celui installé), la commande échoue avec
une indication invitant à exécuter `amc package versions <pkg>` pour
obtenir la liste complète des contraintes.

## Options

| Flag             | Effet                                                                 |
| ---------------- | ---------------------------------------------------------------------- |
| `-o <output>`    | Nom de base de la sortie. Le fichier `.c` est `<output>.c`. Par défaut `a.out`.   |
| `--lib`          | Compile en mode bibliothèque (aucun `int main()` émis, `[Library]` dans la sortie).   |
| `--check`        | Vérification de types uniquement, sans écrire le fichier `.c`. Utile pour les vérifications depuis l'éditeur. |
| `--color`        | Force la sortie ANSI colorée dans les diagnostics.                                |
| `--no-color`     | Désactive la sortie ANSI colorée.                                             |
| `--quiet`        | Supprime les messages de progression (uniquement les erreurs et le rapport final).         |
| `--verbose`      | Affiche des informations de compilation supplémentaires.                                                |
| `--version`      | Affiche la version et quitte.                                            |
| `--help`, `-h`   | Affiche l'usage et quitte.                                                  |

Tout argument positionnel se terminant par `.am` est traité comme un fichier
d'entrée. Les options inconnues provoquent une erreur avec un message d'usage.

## Codes de sortie

| Code | Signification                                                |
| ---- | ------------------------------------------------------ |
| `0`  | Succès.                                               |
| `1`  | Le résolveur et/ou le vérificateur de types a signalé une erreur, *ou* le CLI a été mal utilisé (pas d'entrée, flag inconnu, etc.). |

Le compilateur tente de progresser malgré les avertissements du résolveur —
il arrive souvent qu'un fichier `.c` soit généré en même temps que des
diagnostics. Fiez-vous au code de sortie comme source de vérité, pas à
la présence du fichier `.c`.

## Compilation multi-fichiers

Passez plusieurs fichiers `.am` ; le compilateur les fusionne, exécute
toutes les passes sur l'union et émet un seul fichier `.c` :

```bash
amc lexer.am parser.am main.am -o myapp
```

La directive `namespace` du premier fichier détermine le préfixe des
symboles C pour l'ensemble de l'unité de compilation — placez donc le
fichier le plus autoritaire en premier (ou assurez-vous que tous les
fichiers partagent le même namespace).

`Program.Main` de `main.am` devient le point d'entrée du binaire. Si
plusieurs fichiers déclarent `Program.Main`, le compilateur C rejettera
les symboles en double à l'édition de liens — un seul point d'entrée par
exécutable.

## Mode bibliothèque

```bash
amc --lib mylib.am -o mylib
# → mylib.c (no int main, marked "[Library]")
gcc -Iruntime -c mylib.c -o mylib.o
# Link mylib.o from a host program — see chapter 5 for C interop.
```

Détection automatique : un fichier sans `Program.Main` est automatiquement
traité comme une bibliothèque (aucun flag `--lib` nécessaire).

## `amc build / run / watch` (depuis v0.7.9)

Le verbe de compilation de premier rang. `amc build` exécute amc sur le
fichier d'entrée **et** invoque gcc avec les bons flags (en-têtes du
runtime, `lib/libamalgame.a`, archives de façades des packages installés,
fichiers `.o` vendorisés des packages, `[stdlib].libs` déclarés par chaque
package). Le résultat est un binaire exécutable — sans étape gcc séparée.

```bash
amc build hello.am             # produces ./hello
amc build -o myapp src/main.am # explicit output
amc build -v hello.am          # verbose: prints the gcc command
amc build -g hello.am          # debug build (-O0 -g for `amc dap`)
```

Le flag `-g` / `--debug` (v0.8.0+) remplace `-O2` par `-O0 -g` à la
fois pour la liaison gcc en une étape et pour le chemin C++ en deux étapes
(DuckDB, etc.). Le cgen émet des directives `#line N "foo.am"` à chaque
statement, de sorte que DWARF transporte les noms de fichiers `.am` et les
numéros de ligne — les débogueurs placent directement les points d'arrêt
sur les sources `.am`, sans source maps. Voir `amc dap` au chapitre 6 et
le workflow de débogage en fin de chapitre 6.

`amc run` enchaîne une compilation avec `Process.Run` sur le binaire
résultant. Les arguments après le sentinel `--` sont transmis à l'argv
du programme utilisateur :

```bash
amc run hello.am               # build then run, no user args
amc run server.am -- --port 8080 --verbose
```

`amc watch` compile une fois, puis surveille le mtime du fichier d'entrée
toutes les 500 ms et reconstruit à chaque modification. Avec `--run`, il
ré-exécute également le binaire après chaque reconstruction réussie —
pratique pour les boucles de développement :

```bash
amc watch --run server.am      # rebuild + restart on save
amc watch hello.am             # rebuild only (manual ./hello to run)
```

La v1 surveille uniquement le fichier d'entrée explicitement nommé en
ligne de commande. Les imports transitifs sont hors périmètre jusqu'à ce
que le package `Amalgame.IO.FileWatcher` dispose d'un mode événementiel
(post-D dans la roadmap).

La forme avec arguments positionnels `amc foo.am -o foo` fonctionne
toujours (émet uniquement `foo.c` — sans étape gcc) et reste supportée
pour les cas limites où les utilisateurs souhaitent brancher leur propre
commande gcc (cross-compilation CI, flags personnalisés, cibles embarquées).

## Workflows typiques

### Test rapide d'un fichier unique

```bash
amc build hello.am && ./hello   # since v0.7.9
# OR the explicit two-step form
amc hello.am -o hello
gcc -Iruntime hello.c -lgc -lm -lz -o hello
./hello
```

### Vérification de types à la sauvegarde (intégration éditeur)

```bash
amc --check --no-color foo.am
echo $?     # 0 = OK, 1 = errors printed on stderr
```

### Compilation d'un projet multi-fichiers

Depuis la v0.7.9, `amc build` avec un point d'entrée unique couvre la
plupart des cas — amc résout les instructions `import` transitivement
depuis l'entrée, il suffit généralement de :

```bash
amc build src/main.am          # → ./main
```

Pour les anciennes versions v0.7.x (ou pour un contrôle explicite sur la
liste de fichiers), la forme en deux étapes fonctionne toujours :

```bash
amc \
    src/util/strings.am \
    src/util/collections.am \
    src/server/router.am \
    src/main.am \
    -o build/app

gcc -O2 -Iruntime build/app.c -lgc -lm -o build/app
```

### Recompilation du compilateur lui-même

```bash
./build_amc.sh        # ~5s, full self-host (runs amc on its own sources)
```

Le comportement de ce script est décrit au chapitre 6.

## Environnement

Le CLI est hermétique — il ne lit pas les variables d'environnement. Les
en-têtes du runtime côté compilation proviennent de `gcc -I<runtime-dir>`.

## Débogage du compilateur

Si `amc` produit du C invalide, réduisez l'entrée en supprimant la moitié
jusqu'à ce que l'échec disparaisse, puis signalez le problème avec la
reproduction minimale. Flags utiles lors du tri :

- `--quiet` pour faire taire les messages « OK » et se concentrer sur les erreurs
- `--no-color` afin que la sortie des extraits reste compatible avec grep
- Inspectez directement le fichier `.c` émis — le nommage suit
  `<Namespace>_<Class>_<Method>` et se prête bien à grep.

Pour les détails internes approfondis (comment le pipeline parcourt l'AST,
où ajouter un nouveau builtin, comment le CGen émet une fonctionnalité),
voir le chapitre 7.
