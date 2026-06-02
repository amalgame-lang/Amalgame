# 6 · Build & outils

Ce chapitre parcourt tous les scripts et workflows qui permettent de
construire, tester et livrer Amalgame.

## Le compilateur et son snapshot

Amalgame conserve le compilateur auto-hébergé dans l'arborescence du
dépôt, accompagné d'un snapshot portable connu-bon :

- **`./amc`** — auto-hébergé, écrit en Amalgame. Le compilateur du
  quotidien. Source dans `src/`, produit par `./build_amc.sh`.
- **`./snapshot/amc`** — dernier `amc` connu-bon, capturé par
  `tools/save-snapshot.sh`. Le `snapshot/amc_lib.c` portable est
  commité ; une seule invocation `gcc` reconstruit le binaire sur
  n'importe quelle plateforme (voir `snapshot/INFO.md`). C'est le
  point d'entrée pour un démarrage à froid et le filet de secours
  lorsque `./amc` est cassé en cours de développement.

## Démarrage à froid (bootstrap)

Depuis un clone propre :

```bash
gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -o snapshot/amc
./build_amc.sh
```

Dépendances (Linux) :

- gcc
- libgc-dev (Boehm GC — la seule dépendance runtime depuis v0.8.31 ;
  HTTP a été déplacé vers le package externe `amalgame-net-http`,
  supprimant le lien libcurl)

macOS utilise `brew install bdw-gc` ; Windows utilise MSYS2 MinGW64
(`mingw-w64-x86_64-{gcc,gc}`). Le même `snapshot/amc_lib.c` sert de
point d'entrée multiplateforme partout.

## `./build_amc.sh` — build auto-hébergé

La boucle en cinq secondes :

```
Step 1   ./amc src/lexer/*.am src/parser/*.am … src/generator/gen_test.am -o gen_test
         gcc -O2 -Iruntime gen_test.c -o gen_test
Step 2   ./gen_test                 # generates src/amc_lib.c (and inspection bundles)
Step 3   gcc -Iruntime src/amc_lib.c -lgc -lm -o amc
```

Notes :

- L'étape 1 utilise `./amc` s'il existe, sinon `./snapshot/amc`. Depuis
  un clone propre, construisez d'abord `./snapshot/amc` via la commande
  de démarrage à froid ci-dessus.
- L'étape 1 tolère une sortie non nulle de `amc` tant que le `.c` de
  sortie a été produit. C'est le cas récurrent « je viens d'ajouter un
  builtin et le `amc` en cours ne le connaît pas encore » — `gcc` reste
  le vrai garde-fou de correction.
- `main.am` est intentionnellement **exclu** de la liste de sources de
  gen_test. Il déclare son propre `Program.Main` (le point d'entrée CLI),
  qui entrerait en conflit avec le `Program.Main` de `gen_test.am` dans
  le binaire bundlé. `main.am` n'est compilé dans `amc_lib.c` que via
  gen6 à l'étape 2.
- L'ordre des fichiers dans `AMC_SOURCES` est important pour le CGen de
  bootstrap : les classes doivent apparaître avant leurs dépendantes
  (car la passe 2 émet les déclarations forward + les corps fichier par
  fichier). `diagnostics.am` est listé avant `resolver.am` pour cette
  raison — `SourceMap` et `SourceSnippet` doivent être visibles.

Le build est hermétique — pas de variables d'environnement, pas d'état
global. Relancer `./build_amc.sh` est toujours sans danger.

## Tests — bundles AM via `amc test`

Depuis 2026-05-22 la suite de tests vit comme des bundles `*_test.am`
auto-découverts par `amc test`. Chaque bundle compile ses samples une
seule fois et grep-vérifie la stdout capturée pour chaque assertion —
~6× plus rapide que les anciens runners bash (stdlib : 3m34s → 8s).
Les runners bash legacy ont été supprimés le 2026-05-24 ; `amc test`
est désormais le seul chemin.

| Bundle                            | Cas  | Couverture                                       |
| --------------------------------- | ---- | ------------------------------------------------ |
| `tests/fmt/fmt_test.am`           | 12   | Formatter idempotence + semantic preservation    |
| `tests/amc_new/amc_new_test.am`   | 38   | `amc new` scaffolder smoke tests                 |
| `tests/stdlib_bundle/stdlib_test.am` | 196  | IO/String/Collections/Json/Toml/Path/MsgPack/PR + 2 e2e |
| `tests/core_bundle/core_test.am`  | 332  | Core lang, namespace, interfaces, enums, lambda, LSP, DAP, LLM tooling |
| **Total**                         | **578** | + 5 SKIP (HTTP moved to amalgame-net-http)  |

```bash
./amc test ./tests/                 # tout (~42s) — auto-discovery
./amc test ./tests/core_bundle/     # une suite (29s)
./amc test ./tests/stdlib_bundle/   # stdlib uniquement (8s)
./tests/run_all_tests.sh            # wrapper d'une ligne autour de `amc test ./tests/`
```

### Flags

| Flag             | Effet                                                                                       |
| ---------------- | ------------------------------------------------------------------------------------------- |
| `--filter <pat>` | N'exécute que les `*_test.am` dont le chemin contient `<pat>` (substring, pas une regex)    |
| `--ci`           | Sortie terse : drop `[PASS]`/`[SKIP]` et les en-têtes par fichier, garde `[FAIL]` + tally  |
| `--list`         | Imprime les chemins découverts (post-filter) et exit 0 — pas de compile, pas de run         |
| `--help` / `-h`  | Imprime ce tableau                                                                          |

`amc test` prune les répertoires `fixtures/` lors du crawl : les
fichiers fixture (LSP workspace, test-runner self-test) restent
accessibles via un chemin explicite (`amc test ./tests/fixtures/<x>/`)
mais ne sont pas auto-exécutés depuis la racine.

## Intégration continue

`.github/workflows/ci.yml` — s'exécute à chaque push et PR vers `main` /
`develop` :

| Job     | Tourne sur             | Ce qu'il fait                                                      |
| ------- | ---------------------- | ------------------------------------------------------------------ |
| linux   | ubuntu-latest          | dépendances apt · `gcc snapshot/amc_lib.c` · `./build_amc.sh` · tests |
| macos   | macos-latest (arm64)   | dépendances brew · `gcc src/amc_lib.c` · smoke compile hello       |
| windows | windows-latest (MSYS2) | dépendances pacman · `gcc src/amc_lib.c` · smoke compile hello     |

Les trois plateformes valident que le `amc_lib.c` suivi est portable et
produit un binaire fonctionnel.

## Releases

`.github/workflows/release.yml` — s'exécute à chaque push d'un tag `v*` :

| Job           | Ce qu'il produit                                         |
| ------------- | -------------------------------------------------------- |
| build-linux   | `amc-X.Y.Z-linux-x86_64.tar.gz` + `.sha256`              |
| build-macos   | `amc-X.Y.Z-macos-arm64.tar.gz` + `.sha256`               |
| build-windows | `amc-X.Y.Z-windows-x86_64.zip` (DLLs incluses) + `.sha256` |
| publish       | agrège les checksums, crée une GitHub Release            |

Le zip Windows embarque les DLLs MinGW contre lesquelles le binaire est
lié (libgc, libgcc_s_seh, libwinpthread, etc.) — les utilisateurs
installent le zip et lancent `amc.exe` sans aucune dépendance externe.

Déclencher une release :

```bash
git tag v0.4.0
git push origin v0.4.0
# La CI prend ~10 minutes, la release apparaît sur GitHub une fois terminée.
```

`workflow_dispatch` est également activé pour tester le workflow sans
couper une vraie release.

## Installeur Inno Setup (Windows)

`install/windows/amalgame.iss` produit un installeur `.exe` pour les
utilisateurs Windows via [Inno Setup 6+](https://jrsoftware.org/isinfo.php) :

```bash
# Drop a portable MinGW64 (e.g. from winlibs.com) into install/windows/gcc-bundle/
iscc install/windows/amalgame.iss
# → Output/amalgame-X.Y.Z-setup.exe
```

L'installeur livre `amc.exe`, `runtime/_runtime.h`, la documentation,
et la chaîne d'outils MinGW64 embarquée, de sorte que les utilisateurs
obtiennent un `amc` + `gcc` fonctionnels dès l'installation.

## Formule Homebrew

`install/homebrew/amalgame.rb` — pour la distribution via `brew tap`.
Mettez à jour la version et le SHA256 à chaque release.

## Hygiène locale

- `git clean -fdx` supprimera les `.c` générés et les binaires dans
  l'arbre de travail. Attention — cela supprime aussi `gen_test`, `amc`
  et `src/amc_lib.c`. Utilisez `./build_amc.sh` ensuite pour régénérer.
- `./CLEANUP.sh` est un helper legacy qui supprimait d'anciens artefacts
  de débogage. Il est essentiellement sans effet aujourd'hui.

## Support éditeur

`editors/vscode/` est une extension VS Code complète (grammaire
TextMate, configuration de langage, client LSP, README). Installer pour
le développement :

```bash
ln -s "$(pwd)/editors/vscode" ~/.vscode/extensions/amalgame-0.1.0
# Reload window: Ctrl+Shift+P → Developer: Reload Window
```

L'extension démarre `amc lsp` sur les fichiers `.am` via stdio JSON-RPC.
Pointez-la vers votre build local en configurant vos paramètres VS Code :

```json
"amalgame.serverPath": "/abs/path/to/Amalgame/amc"
```

Le serveur LSP (depuis v0.3.4 pour les diagnostics, v0.3.5 pour le
survol et la complétion) fournit actuellement :

- **Diagnostics** — erreurs du résolveur et du typechecker publiées à
  chaque `didOpen` / `didChange`, avec le token fautif souligné.
- **Survol** — infobulle Markdown affichant le type inféré de
  l'identifiant sous le curseur (`name: type`). `null` quand le
  curseur n'est pas sur une expression typée.
- **Complétion** — liste globale de symboles (types builtins,
  fonctions, classes / enums utilisateur) avec des indices
  `CompletionItemKind`. Le déclencheur `.` est réservé pour la
  complétion de membres v2.5 (`obj.<cursor>` restreint au type du
  récepteur).

`amc lsp` fonctionne en mode stdio par défaut. Pilotez-le manuellement
pour le débogage via :

```bash
echo -e 'Content-Length: 41\r\n\r\n{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}' | ./amc lsp
```

## Débogage des programmes `.am` (`amc dap`, depuis v0.8.0)

`amc dap` est un serveur Debug Adapter Protocol. Depuis la Phase 7
(post-v0.8.46) il fonctionne par défaut en **mode bridge** : amc démarre
`gdb --interpreter=mi3` lui-même et traduit DAP↔gdb-MI en interne, de
sorte que les variables `AmalgameList*` / `AmalgameMap*` / `AmalgameSet*`
/ `AmalgameClosure*` s'affichent avec des résumés et des enfants
dépliables, et que les frames d'aide runtime (`Amalgame_*`,
`_runtime_*`, `GC_*`) sont masqués dans la pile d'appel. Sur les
machines sans gdb (macOS standard, etc.), `amc dap` bascule
transparentement sur le proxy legacy. `amc dap --raw` est l'opt-out
explicite : il `execvp()` vers le backend DAP natif du système
(`lldb-dap` sur macOS, `gdb --dap` sur Linux/MSYS2 avec gdb ≥ 14),
exactement comme le faisait v0.8.0. Pas de fichiers source map dans les
deux cas : le cgen émet des directives `#line N "foo.am"` de sorte que
DWARF transporte nativement les noms de fichiers `.am` et les numéros de
ligne.

### Prérequis

| OS      | Installer l'un des                                                    |
| ------- | --------------------------------------------------------------------- |
| Linux   | `lldb-dap` depuis LLVM 18+ (`wget https://apt.llvm.org/llvm.sh && sudo ./llvm.sh 18 && sudo apt install -y lldb-18`) |
| macOS   | Xcode Command Line Tools 14+ (`xcode-select --install`)                |
| Windows | gdb 14+ via MSYS2 (`pacman -S mingw-w64-x86_64-gdb`) — en attente v0.8.1 |

### Workflow — VS Code (recommandé)

1. Générer un projet avec le câblage déjà en place :
   ```bash
   amc new myapp --vscode      # writes .vscode/launch.json + settings.json
   ```
2. Builder avec DWARF :
   ```bash
   cd myapp && ./build.sh -g   # forwards -g to `amc build`
   ```
3. Dans VS Code : ouvrir `src/main.am`, cliquer dans la gouttière pour
   poser un point d'arrêt, appuyer sur **F5**. Le menu déroulant F5
   propose deux configurations prêtes à l'emploi — choisir
   « Debug myapp (Linux/macOS) » ou « (Windows) » selon votre plateforme.

L'extension VS Code Amalgame (v0.3.0+) enregistre le type de debug `amc`
et expose une `DebugAdapterDescriptorFactory` qui démarre `amc dap` avec
le chemin issu de `amalgame.dapServerPath` (ou `amalgame.serverPath`
comme repli). Les logs atterrissent dans le canal de sortie
« Amalgame DAP ».

### Workflow — lldb CLI (sans VS Code)

```bash
amc build -g hello.am
lldb-18 ./hello
(lldb) breakpoint set --file hello.am --line 5
(lldb) run
(lldb) frame variable     # locals visible by name + Amalgame type
(lldb) next               # step over
(lldb) continue
```

Les champs DWARF `DW_AT_decl_file` / `DW_AT_decl_line` pointent vers
`hello.am`, de sorte que lldb résout la ligne source `.am` en une
adresse d'instruction réelle sans aucune configuration supplémentaire.

### Stratégie & limites

Le basculement de la Phase 7 (post-v0.8.46) a fait du bridge en interne
(« Approche A ») le comportement par défaut. amc démarre
`gdb --interpreter=mi3 --nx --quiet` via fork + pipe + `poll()`,
analyse les enregistrements MI3 et réécrit le trafic dans les deux
sens :

- `AmalgameList*` s'affiche comme `List[N]` avec les enfants `[0]`…
  `[N-1]` ; `AmalgameMap*` / `AmalgameSet*` reflètent leurs tailles ;
  `AmalgameClosure*` s'affiche comme `λ env=0x…`.
- Les frames de la pile correspondant à `^Amalgame_`, `^_runtime_`,
  `^GC_` sont filtrés ; `stepIn` avance automatiquement jusqu'à atterrir
  sur une frame visible (budget de 8 sauts) pour que l'utilisateur ne
  reste jamais bloqué au milieu du runtime.
- `amc dap --show-runtime` conserve toutes les frames visibles.

Le proxy legacy v0.8.0 (Approche C) reste disponible sous
`amc dap --raw` : un `execvp` transparent vers `lldb-dap` sur macOS
ou `gdb --dap` sur Linux/MSYS2. Utile pour les utilisateurs confrontés
à une régression du bridge, ou sur les machines où gdb n'est pas installé
(amc bascule automatiquement dans ce cas — aucun flag nécessaire).
