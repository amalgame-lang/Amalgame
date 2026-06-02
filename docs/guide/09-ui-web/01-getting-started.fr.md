# 01 — Premiers pas

## Prérequis

- **`amc` 0.8.17 ou version ultérieure.** ui-web v0.0.5+ s'appuie sur les
  appels chaînés inter-packages, les littéraux de listes et le lowering
  `new X(...).Method()` ; les anciennes versions d'amc échouent à
  l'inférence de types ou ne savent pas les abaisser.
  La v0.8.16 a corrigé le comportement quadratique sur la longueur des
  chaînes, la v0.8.17 a corrigé la compilation CI sous macOS.
- **Un moteur webview système** correspondant à la cible de compilation :

  | OS                 | Moteur     | Installation                                      |
  |--------------------|------------|---------------------------------------------------|
  | Debian / Ubuntu    | WebKitGTK  | `sudo apt install libwebkit2gtk-4.1-dev pkg-config` |
  | Fedora             | WebKitGTK  | `sudo dnf install webkit2gtk4.1-devel pkgconf` |
  | Arch               | WebKitGTK  | `sudo pacman -S webkit2gtk-4.1 pkgconf`      |
  | macOS              | WKWebView  | livré avec l'OS depuis 10.10                |
  | Windows 11         | WebView2   | intégré à l'OS                               |
  | Windows 10 ≥ 1803  | WebView2   | Microsoft installe automatiquement le runtime evergreen |
  | Windows 10 < 1803  | WebView2   | embarquez le bootstrapper (voir le README principal) |

## Installation

Depuis le projet où vous allez écrire l'application :

```sh
amc package add ui-web
```

Cette commande récupère `amalgame-ui-web` dans `~/.amalgame/packages/`,
épingle la version dans `amalgame.lock` et permet à `amc` de résoudre
l'instruction `import Amalgame.UI.Web`.

Si vous préférez un scaffolding, `amc new --template ui-web-form` génère
un projet complet (`amalgame.toml`, `src/main.am`, `build.sh`,
`.gitignore`) — consultez le README principal pour les options disponibles.

## Une fenêtre minimale

`src/hello.am` :

```amalgame
import Amalgame.UI.Web

class Program {
    public static void Main() {
        let win: Window = new Window("Hello", 480, 320, false)
        if (!win.IsValid()) {
            Console.WriteError("ui-web: failed to create webview slot")
            return
        }

        Page.New()
            .SetTitle("Hello, Amalgame!")
            .SetBody(
                Element.Stack()
                    .AddChild(Element.Heading("Hello, Amalgame"))
                    .AddChild(Element.Label("Welcome to ui-web v0.0.5."))
                    .AddChild(Element.Button("Click me")
                        .OnClick((req: string) => "\"clicked\"")
                        .OnResult("out"))
                    .AddChild(Element.Pre("").Id("out"))
            )
            .ApplyTo(win)

        win.Run()
        win.Destroy()
    }
}
```

Le constructeur `Window` prend `(title, width, height, debug)`.
`debug=true` expose les DevTools (`Ctrl+Shift+I`) ; laissez-le à `false`
dans les versions livrées.

## Point d'entrée raccourci — `Form` + `Application.Run` (v0.0.7)

Pour les applications dont la lisibilité est proche du modèle WinForms,
utilisez le wrapper de valeur `Form` et laissez `Application.Run` gérer
toute la mécanique `Window + Page + ApplyTo + Run + Destroy` :

```amalgame
import Amalgame.UI.Web

class Program {
    public static void Main() {
        let f: Form = new Form("Hello", 480, 320)
        f.SetBody(
            Element.Stack()
                .AddChild(Element.Heading("Hello, Amalgame"))
                .AddChild(Element.Button("Click me")
                    .OnClick((req: string) => "\"clicked\"")
                    .OnResult("out"))
                .AddChild(Element.Pre("").Id("out"))
        )
        Application.Run(f)
    }
}
```

`Form` est une classe plate (pas une base à sous-classer — le dispatch
statique d'AM rend les surcharges parent-virtuelles peu fiables). Elle
transporte le titre, la taille, le corps, le thème, le flag debug, ainsi
qu'une `Closure` optionnelle `OnLoad(handler)` qui se déclenche une fois
la fenêtre ouverte, vous permettant d'effectuer des enregistrements
tardifs via `win.Bind` :

```amalgame
let f: Form = new Form("Editor", 1024, 768)
f.SetTheme("auto")
f.SetDebug(false)
f.OnLoad((req: string) => {
    // enregistrez ici les bindings tardifs une fois la webview active
    return ""
})
f.SetBody( … )
Application.Run(f)
```

Les deux styles produisent des binaires identiques — `Application.Run`
est du sucre syntaxique sur la séquence explicite
`new Window(...) → Page.ApplyTo → win.Run()`.

## Script de compilation

Un `build.sh` typique sous Linux (miroir de ce que génère `amc new`) :

```sh
#!/bin/sh
set -eu

AMC="${AMC:-amc}"
APP_NAME="hello"
PKG_DIR="${AMALGAME_PACKAGES_DIR:-$HOME/.amalgame/packages}"
UIWEB="$PKG_DIR/amalgame-ui-web"

# 1. Compile l'implémentation C++ de la webview une seule fois.
test -f "$UIWEB/runtime/vendor/webview/webview.o" || \
    g++ -c -O2 -DWEBVIEW_GTK \
        "$UIWEB/runtime/vendor/webview/webview.cc" \
        -o "$UIWEB/runtime/vendor/webview/webview.o"

# 2. Compile la couche de glue C.
test -f "$UIWEB/runtime/Amalgame_UI_Web.o" || \
    gcc -c -O2 -I"$UIWEB/runtime" \
        "$UIWEB/runtime/Amalgame_UI_Web.c" \
        -o "$UIWEB/runtime/Amalgame_UI_Web.o"

# 3. Compile facade.am en bibliothèque statique une seule fois par version ui-web.
(cd "$UIWEB" && test -f facade.o || ("$AMC" --lib facade.am --quiet && \
    gcc -c -O2 -Iruntime a.out.c -o facade.o))

# 4. Compile notre app + édition de liens.
"$AMC" -o "$APP_NAME" "src/main.am" --external "$UIWEB/facade.am" --quiet
gcc -O2 \
    -I"$UIWEB/runtime" -I"$UIWEB/runtime/vendor/webview" \
    "${APP_NAME}.c" \
    "$UIWEB/facade.o" \
    "$UIWEB/runtime/Amalgame_UI_Web.o" \
    "$UIWEB/runtime/vendor/webview/webview.o" \
    $(pkg-config --libs webkit2gtk-4.1) \
    -lstdc++ -lgc -lm -lcurl -lz \
    -o "$APP_NAME"
```

`amc new --template ui-web-form` produit une version clé en main de ce
script avec les chemins macOS / Windows. Les applications existantes
peuvent le copier tel quel — seule la résolution du chemin de package
change d'un OS à l'autre.

Exécution :

```sh
./build.sh && ./hello
```

Vous devriez voir une fenêtre 480×320 avec le titre, un bouton et un
panneau `<pre>` vide. Cliquer sur le bouton affiche `"clicked"` (avec les
guillemets — c'est l'encodage JSON) dans le panneau.

## Ce qui se passe

- `new Window(...)` alloue un slot webview côté C du runtime. Jusqu'à
  4 fenêtres par processus.
- `Page.New().SetBody(...)` construit un arbre d'`Element` en mémoire.
  Rien n'est encore rendu.
- `Page.ApplyTo(win)` parcourt l'arbre, génère le HTML, injecte les
  bridges JS d'auto-collecte et de routage de résultat via `Window.Init`,
  charge le HTML via `Window.SetHtml`, et enregistre chaque handler
  `.OnClick` / `.OnChange` via `Window.Bind`.
- `Window.Run()` entre dans la boucle d'événements de la webview et
  bloque jusqu'à ce que la fenêtre soit fermée ou que `Window.Terminate()`
  soit appelé.

Vous écrivez de l'Amalgame pour toute l'application. Pas de HTML
manuscrit. Pas de JS à moins que vous ne le souhaitiez délibérément
(voir [`05-extending.md`](05-extending.md)).

## Par où continuer

- Le [catalogue de widgets](02-widgets.md) présente chaque builder avec
  le nom de l'outil WinForms correspondant et un exemple de code.
- Le [modèle d'événements](03-events-and-state.md) explique comment
  l'état du formulaire circule, comment `OnResult` achemine les valeurs
  de retour, et comment mettre à jour des lignes sans tout re-rendre.
- La [référence de mise en page](04-layout-and-theme.md) couvre
  `Stack` / `Row` / `Grid` / `Flow`, le mode application-shell
  `Page.FillViewport`, et comment surcharger les sept variables CSS.

## Pièges courants

- **Window.IsValid()** peut échouer si les 4 slots
  `AMALGAME_UI_WEB_MAX_WINDOWS` sont déjà occupés. Détruisez
  explicitement les fenêtres dont vous n'avez plus besoin.
- **Les DevTools nécessitent `debug=true`** au moment de la construction
  de `Window` — c'est le 4e argument. L'activer ultérieurement n'est pas
  supporté par la bibliothèque webview sous-jacente.
- **Les longues chaînes fluentes** envoyaient autrefois l'inférence de
  types d'amc dans un comportement quadratique sur des chaînes de ~24
  liens et plus. Corrigé dans amc v0.8.16 — mais décomposer en
  intermédiaires `let header: Element = …` reste le style recommandé pour
  la lisibilité. Voir
  [`04-layout-and-theme.md`](04-layout-and-theme.md#fluent-chain-limits).
- **Les chaînes `new X(...).Method()`** étaient abaissées incorrectement
  par amc avant la v0.8.16 ; si vous ciblez un compilateur plus ancien,
  décomposez via `let x = new X(...)` puis appelez `x.Method()`.
