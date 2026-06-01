# Outillage éditeur & débogage

Amalgame est livré avec deux serveurs qui transforment un simple éditeur en IDE :

- **`amc lsp`** — un serveur de langage (diagnostics, complétion, go-to-def,
  survol, renommage, …). Votre éditeur dialogue avec lui via stdio.
- **`amc dap`** — un Debug Adapter : véritables breakpoints, pas-à-pas et
  inspection de variables dans votre source `.am`, en s'appuyant sur gdb/lldb.

Les deux sont câblés automatiquement par l'extension VS Code. Ce chapitre
couvre l'installation + le workflow côté utilisateur ; pour le fonctionnement
interne du bridge DAP (traduction gdb-MI, directives `#line`) et le tableau
complet des prérequis, voir [06-build-and-tooling.fr.md](06-build-and-tooling.fr.md).

---

## Configuration de l'éditeur

### VS Code

Installez l'extension Amalgame (le tarball de release embarque le `.vsix` ;
`install.sh` l'installe automatiquement si un VS Code est détecté). L'ouverture
d'un fichier `.am` démarre `amc lsp` automatiquement.

L'extension lit ces réglages :

| Réglage | Rôle |
|---------|---------|
| `amalgame.serverPath` | chemin de l'`amc` utilisé comme serveur LSP (par défaut `amc` dans le PATH ; mettre `${workspaceFolder}/amc` pour utiliser un build local) |
| `amalgame.enableLsp` | active/désactive le serveur de langage |
| `amalgame.dapServerPath` | `amc` utilisé pour le débogage (vide → repli sur `serverPath`) |
| `amalgame.dapBridge` | mode bridge activé/désactivé (activé par défaut ; voir ci-dessous) |
| `amalgame.dapShowRuntime` | affiche les frames d'aide runtime/GC dans la pile d'appels (désactivé par défaut) |

### Autres éditeurs

Tout éditeur compatible LSP (Neovim, Helix, Emacs/eglot, …) peut piloter
`amc lsp` via stdio — pointez son client de serveur de langage sur le binaire
`amc` avec l'argument `lsp`. Le protocole est du LSP standard ; aucun client
spécifique à Amalgame n'est requis.

---

## Ce que le serveur de langage vous apporte

`amc lsp` implémente la surface de navigation + édition attendue :

- **Diagnostics** — publiés à l'ouverture et à chaque modification (erreurs de
  parsing + de typage, lints).
- **Complétion** — y compris `import Amalgame.<cursor>` listant les packages
  installés + embarqués.
- **Survol** et **aide à la signature**.
- **Go to definition / declaration / type definition**.
- **Find references** et **document highlight** (mise en évidence de chaque
  usage du symbole sous le curseur).
- **Document symbols** (plan) et **folding ranges**.
- **Renommage** dans tout le document, **inlay hints**, **code actions**
  (par ex. « ajouter l'import pour ce package »), et **hiérarchie d'appels**.

> **Faux positif connu.** Le LSP peut signaler `Unknown symbol` sur des types
> provenant d'un package externe, même quand `amc build` compile
> proprement — l'éditeur ne charge pas toujours le lockfile du projet. Faites
> confiance au build ; c'est le diagnostic qui est en faute, pas votre code.
> (Également noté dans [12-packages.fr.md](12-packages.fr.md).)

---

## Déboguer un programme

`amc dap` (depuis v0.8.0) est un serveur Debug Adapter Protocol. Le flux :

### 1. Compiler avec les infos de debug

```bash
amc build -g myprog.am -o myprog     # -g embeds DWARF
```

Le générateur de code émet des directives `#line N "myprog.am"`, de sorte que
le débogueur fait correspondre le C exécuté à votre source `.am` — sans fichier
source-map séparé.

### 2a. Dans VS Code

Avec l'extension installée, créez une configuration de lancement de type
`amc` pointant vers le binaire compilé (compilez d'abord avec `amc build -g`),
posez des breakpoints dans le fichier `.am`, et démarrez le débogage.
L'extension lance `amc dap` pour vous.

Par défaut, `amc dap` s'exécute en **mode bridge** : amc pilote `gdb` lui-même
et traduit DAP↔gdb-MI in-process, de sorte que les variables
`List`/`Map`/`Set`/`Closure` sont joliment affichées avec des résumés et un
drill-down, et que les frames d'aide runtime (`Amalgame_*`, `GC_*`) sont
masquées de la pile d'appels. Sur un système sans gdb, il se rabat de façon
transparente sur le proxy historique ; `amc dap --raw` force le proxy
explicitement (lldb-dap sur macOS, `gdb --dap` sur Linux/MSYS2 avec gdb ≥ 14).

### 2b. Depuis le CLI (sans éditeur)

Vous pouvez déboguer directement avec un débogueur standard :

```bash
amc build -g hello.am
lldb ./hello
(lldb) breakpoint set --file hello.am --line 5
(lldb) run
```

Comme le binaire embarque du DWARF avec les infos de ligne `.am`, les
breakpoints et le pas-à-pas fonctionnent sur votre source, pas sur le C généré.

---

## Piloter les serveurs manuellement

Les deux serveurs parlent leurs protocoles via stdio, ce qui est pratique pour
déboguer l'outillage lui-même. Le LSP attend du JSON-RPC encadré par
`Content-Length` :

```bash
echo -e 'Content-Length: 41\r\n\r\n{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}' | amc lsp
```

Vous ne ferez normalement pas cela — c'est l'éditeur qui s'en charge — mais
c'est le moyen le plus rapide de confirmer qu'`amc lsp` est vivant et de savoir
quel `amc` est utilisé.

---

## Pièges

- **`-g` est requis pour le débogage au niveau source.** Sans lui, le binaire
  n'a pas de DWARF et les breakpoints ne se lieront pas aux lignes `.am`.
- **Le mode bridge nécessite gdb** (Linux/MSYS2, gdb ≥ 14) ; le proxy brut
  nécessite `lldb-dap` (macOS) ou `gdb --dap`. Sur une machine n'ayant ni l'un
  ni l'autre, le débogage est indisponible même si le programme s'exécute
  parfaitement — installez-en un (voir le tableau des prérequis dans
  [06-build-and-tooling.fr.md](06-build-and-tooling.fr.md)).
- **Les frames runtime sont masquées par défaut.** Si vous *voulez* voir les
  frames `Amalgame_*` / GC dans la pile d'appels, activez
  `amalgame.dapShowRuntime`.
- **Le faux positif `Unknown symbol` du LSP** sur les types de packages est
  attendu — fiez-vous à `amc build`.

Pour le pipeline de build, le bootstrap du snapshot et les internes du DAP,
voir [06-build-and-tooling.fr.md](06-build-and-tooling.fr.md) ; pour les verbes
CLI eux-mêmes, [03-cli-reference.fr.md](03-cli-reference.fr.md).
