# 1 · Premiers pas

## Ce qu'il faut savoir avant tout

Amalgame transpile vers du C portable. Compiler un programme Amalgame est
toujours un processus **en deux étapes** :

1. `amc your.am -o your` produit `your.c` (un fichier `.c`, PAS un binaire).
2. `gcc your.c -lgc -lm -o your` produit le véritable binaire natif.

Le `amc` que vous invoquez à l'étape 1 est lui-même le résultat de ce même
pipeline appliqué aux sources du compilateur dans `src/`.

## Installation

### Docker — l'IDE complet, zéro installation

La façon la plus rapide d'essayer Amalgame : un conteneur clé en main qui
démarre un VS Code dans le navigateur avec le compilateur, l'extension
(coloration syntaxique, LSP, débogueur), un projet d'exemple prêt à
lancer et la documentation — tout est pré-câblé, rien à configurer.

```bash
docker run --rm -p 8080:8080 ghcr.io/amalgame-lang/amalgame-ide:latest
```

Ouvre ensuite <http://localhost:8080>. Tu arrives sur l'exemple
`MyFirstApp` avec le guide épinglé dans la barre latérale — appuie sur
**F5** pour le compiler et le déboguer immédiatement.

- **Port déjà occupé ?** Mappe n'importe quel port hôte libre, par ex.
  `-p 8888:8080`, puis ouvre <http://localhost:8888>.
- **Multi-arch :** tourne nativement sur Intel/AMD (`amd64`) et sur Apple
  Silicon / Raspberry Pi 64-bit (`arm64`).
- **Conserver ton travail :** monte un dossier, par ex.
  `-v "$PWD/work:/home/coder/work"`.
- **Exposer au-delà de localhost ?** Définis un mot de passe avec
  `-e PASSWORD=…` (l'authentification est désactivée par défaut pour un
  usage local).

Tu préfères une installation native ? Choisis ta plateforme ci-dessous.

### Linux (Debian/Ubuntu)

```bash
sudo apt install -y gcc libgc-dev

git clone https://github.com/amalgame-lang/Amalgame.git
cd Amalgame
gcc -O2 -Iruntime snapshot/amc_lib.c \
    -lgc -lm -o snapshot/amc   # one-time: bootstrap from tracked C
./build_amc.sh                         # builds the self-hosted amc into ./amc (~5s)
./tests/run_all_tests.sh               # full suite: 578 PASS in ~42s
```

Les suites de tests individuelles peuvent être lancées via
`./amc test ./tests/<bundle>/` où `<bundle>` est l'un des suivants :
`fmt`, `amc_new`, `stdlib_bundle` ou `core_bundle` — voir
[06-build-and-tooling.md](06-build-and-tooling.md#tests).

### macOS (Apple Silicon ou Intel)

```bash
brew install bdw-gc curl
git clone https://github.com/amalgame-lang/Amalgame.git
cd Amalgame
gcc -O2 -Iruntime src/amc_lib.c \
    -lgc -lm -o amc
./amc --version
```

`amc_lib.c` est le snapshot canonique multiplateforme du compilateur
auto-hébergé — il est versionné dans git, donc n'importe quelle plateforme
disposant d'un `gcc` fonctionnel peut bootstrapper depuis un clone propre.

### Windows (MSYS2 MINGW64)

```bash
pacman -S mingw-w64-x86_64-{gcc,gc,curl}
git clone https://github.com/amalgame-lang/Amalgame.git
cd Amalgame
gcc -O2 -Iruntime src/amc_lib.c \
    -lgc -lm -o amc.exe
./amc.exe --version
```

Pour un installeur `.exe` pré-packagé qui embarque un MinGW64 portable,
voir `install/windows/amalgame.iss` (script Inno Setup).

### Binaire pré-compilé

Chaque release taguée dans les
[GitHub Releases](https://github.com/amalgame-lang/Amalgame/releases)
livre des archives `amc-X.Y.Z-{linux,macos,windows}-...`. La commande
en une ligne sélectionne la bonne pour votre OS/architecture :

```bash
curl -fsSL https://raw.githubusercontent.com/amalgame-lang/Amalgame/main/install/install.sh | sh
```

### Raspberry Pi (et autres Linux ARM64)

`amc` livre un build natif **`linux-arm64`**, donc la commande en une ligne
ci-dessus fonctionne sur un Raspberry Pi tournant sous un **OS 64 bits**
(Raspberry Pi OS 64-bit, Ubuntu pour Pi, …) — c'est-à-dire que `uname -m`
retourne `aarch64`. Installez d'abord les dépendances runtime, comme sur
n'importe quelle machine Debian/Ubuntu :

```bash
sudo apt-get install -y build-essential libgc-dev libssl-dev curl
curl -fsSL https://raw.githubusercontent.com/amalgame-lang/Amalgame/main/install/install.sh | sh
amc --version
```

> **Le 32 bits n'est pas pris en charge.** Sur un Pi 3/4/5 tournant sous un
> OS 32 bits, `uname -m` affiche `armv7l`/`armv6l` et l'installeur s'arrête
> avec un message explicite — reflashez un OS 64 bits, ou compilez depuis les
> sources (le snapshot `amc_lib.c` compile partout avec une toolchain C +
> Boehm GC). L'archive `linux-arm64` est compilée nativement en CI sur un
> vrai runner aarch64, pas en cross-compilation.

## Développer dans VS Code (la voie facile)

`install.sh` installe automatiquement l'**extension Amalgame pour VS Code**
s'il détecte VS Code (la release embarque le `.vsix` ; définissez
`AMC_NO_EDITORS=1` pour passer cette étape). Si vous avez installé amc
d'une autre façon, installez-la manuellement une seule fois :

```bash
code --install-extension <amalgame-X.Y.Z.vsix>   # from share/amalgame/editors/vscode/
```

Il suffit ensuite d'**ouvrir n'importe quel fichier `.am`** — l'extension
démarre `amc lsp` et vous obtenez un véritable IDE, sans configuration :

- **Diagnostics en direct** — les mêmes erreurs que `amc --check` remonte,
  au fil de la frappe.
- **Complétion** — symboles, méthodes, et `import Amalgame.<curseur>`
  listant la stdlib + vos packages installés.
- **Navigation & recherche** — aller à la définition (**F12**), **Trouver
  toutes les références** (**Shift+F12**), **recherche de symboles** à
  l'échelle du projet (**Ctrl+T**), **plan** du fichier (**Ctrl+Shift+O**),
  et mise en surbrillance du symbole courant. La hiérarchie d'appels, les
  actions de code, les inlay hints et le repliage sont également disponibles.
- **Survol, renommage** (**F2**), et **formatage** (`amc fmt`).
- **Exécution** — depuis le terminal intégré : `amc run hello.am`
  (compilation + exécution), ou `amc build hello.am` puis `./hello`.
- **Débogage** — appuyez sur **F5** avec un `launch.json` de type `amc` :
  vrais points d'arrêt et pas à pas dans votre source `.am`.

Une bonne première session : `mkdir hello && cd hello`, ouvrez le dossier
dans VS Code, créez `hello.am` (section suivante), et lancez
`amc run hello.am` dans le terminal. Diagnostics, complétion et recherche
fonctionnent immédiatement.

> L'extension est basée sur LSP/DAP et ne nécessite aucune configuration
> pour le cas courant. Pour la liste complète des fonctionnalités, les
> réglages de l'éditeur, les autres éditeurs (Neovim, …), et les
> internals du débogueur, voir le chapitre 15,
> [Outillage éditeur & débogage](15-debugging.md).

## Bonjour, monde

Enregistrez sous `hello.am` :

```kotlin
namespace App
import Amalgame.IO

public class Program {
    public static void Main(string[] args) {
        Console.WriteLine("Hello, Amalgame!")
    }
}
```

Compilez et exécutez :

```bash
./amc hello.am -o hello
gcc -Iruntime hello.c -lgc -lm -o hello
./hello
# → Hello, Amalgame!
```

Quelques conventions importantes :

- Chaque fichier commence par `namespace Foo.Bar` — les symboles C émis
  pour ce fichier sont préfixés par `Foo_Bar_`.
- Le point d'entrée runtime est `Program.Main(string[] args)`. Si un
  fichier définit un `Program.Main`, le compilateur ajoute un wrapper C
  `int main()` en fin de sortie. Sinon, le fichier est compilé en
  bibliothèque (sans `main()`), ce qui le rend approprié pour être lié à
  un autre programme.
- Les lignes `import Amalgame.X` sont acceptées mais actuellement
  informatives — le résolveur connaît la stdlib de manière globale.

## Un premier programme un peu plus grand

```kotlin
// add.am
namespace App
import Amalgame.IO

public class Program {
    public static int Add(int a, int b) {
        return a + b
    }

    public static void Main(string[] args) {
        let n = Program.Add(2, 3)
        Console.WriteLine("2 + 3 = {String.FromInt(n)}")
    }
}
```

```bash
./amc add.am -o add && gcc -Iruntime add.c -lgc -lm -o add && ./add
# → 2 + 3 = 5
```

## Mode bibliothèque (sans `main()`)

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
./amc --lib greeter.am -o greeter
# → Generated: greeter.c (... lines) [Library]
gcc -Iruntime -c greeter.c -o greeter.o
# Link greeter.o into another program from C, Amalgame, or another lib.
```

## Où aller ensuite

- Le [tour du langage](02-language-tour.md) est la lecture rapide.
- La [référence CLI](03-cli-reference.md) liste chaque option.
- Si vous rencontrez une erreur que vous ne comprenez pas, consultez
  « Lire les diagnostics » dans le tour du langage.
