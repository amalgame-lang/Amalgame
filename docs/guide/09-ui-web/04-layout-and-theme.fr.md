# 04 — Mise en page & thème

Comment les enfants sont disposés sur la page, comment faire en sorte que
la mise en page occupe tout le viewport comme une application de bureau,
et comment le thème de couleur du système d'exploitation pilote l'apparence visuelle.

## Aide-mémoire des conteneurs

| Conteneur | Flux des enfants | Utiliser quand |
|---|---|---|
| `Element.Stack()`     | Vertical, écart de 8 px                | Sections empilées verticalement, champs de formulaire, racine de page. |
| `Element.Row()`       | Horizontal, écart de 8 px, centrage vertical | Widgets côte à côte, barres de boutons. |
| `Element.Flow(dir)`   | dir (« row » / « column ») avec retour à la ligne | Nuage de tags, barre d'outils de boutons qui se replie sur les fenêtres étroites. |
| `Element.Grid(r,c,g)` | CSS Grid — `r × c` cellules, écart `g`px | Mise en page tabulaire, cartes de tableau de bord. Passez `r=0` pour des lignes implicites. |
| `Element.AbsoluteContainer()` | Parent `position:relative` | Mises en page de type diagramme / designer. Les enfants utilisent `.Position(x, y)`. |
| `Element.Div()`       | Block, sans mise en page interne         | Quand vous avez besoin d'un wrapper stylé. |
| `Element.Panel()`     | Alias de `Div`                    | Quand le nom WinForms se lit mieux. |
| `Element.GroupBox(title)` | Block avec légende `<legend>` | Sous-sections visuellement groupées d'un formulaire. |
| `Element.TabControl(g) / Tab(g, id, label, body)` | Barre d'onglets + panneau actif en dessous | Catégoriser de nombreux widgets sans surcharger l'utilisateur. |
| `Element.SplitContainer(or, ratio)` (v0.0.8) | Deux volets redimensionnables (`"row"` ou `"column"`) | Shell de type IDE, interface maître/détail. |
| `Element.MenuBar()`   | Barre de menu en haut de la fenêtre (v0.0.8) | Menus Fichier / Édition / Affichage au niveau de l'application. |
| `Element.ToolStrip()` | Rangée de boutons horizontale avec thème      | Barre d'outils au-dessus du contenu. |
| `Element.StatusStrip()` | Pied de page fixé en bas             | Ligne d'état / version / état de connexion. |

```amalgame
Element.Stack()
    .AddChild(Element.GroupBox("Contact")
        .AddChild(Element.Input("name").Attr("placeholder", "Name"))
        .AddChild(Element.Input("email").Attr("placeholder", "Email")))
    .AddChild(Element.Row()
        .AddChild(Element.Button("Save"))
        .AddChild(Element.Button("Cancel")))
```

## `Element.Position(x, y)` + `.Size(w, h)`

Pour un placement au pixel près à l'intérieur d'un `AbsoluteContainer` :

```amalgame
Element.AbsoluteContainer()
    .Size(0, 200)        // 200 px de hauteur, largeur fluide
    .AddChild(Element.Label("X").Position(40, 10).Size(80, 24))
    .AddChild(Element.Label("Y").Position(40, 60).Size(80, 24))
```

`Position` applique `position:absolute;left:Xpx;top:Ypx`. Cela ne prend
effet qu'à l'intérieur d'un parent avec `position:relative|absolute|fixed`,
ce que fournit précisément `AbsoluteContainer`.

## Occupation du viewport

Par défaut depuis la v0.0.5, `Page` dispose le corps comme une application
de bureau (mode `FillViewport`) :

- Le corps est épinglé à une hauteur de `100vh`.
- Le défilement vertical au niveau de la fenêtre est désactivé.
- Le premier enfant du corps grandit pour occuper l'espace restant.
- Les widgets internes gèrent leur propre défilement — les panneaux de
  TabControl, la sortie `<pre>`, le corps du ListView passent tous en
  `overflow:auto` quand leur contenu dépasse la zone visible.

Pour les documents longs qui doivent défiler naturellement, désactivez ce comportement :

```amalgame
Page.New().NaturalFlow().SetBody(...)
```

Pour le comportement de shell d'application (le défaut), aucun appel
supplémentaire n'est nécessaire — structurez simplement votre arbre de
sorte que le premier enfant du corps soit un conteneur qui organise le
reste :

```amalgame
Page.New().SetBody(
    Element.Stack()
        .AddChild(Element.Heading("My App"))
        .AddChild(Element.TabControl("main")
            .AddChild(Element.Tab("main", "a", "A", panelA))
            .AddChild(Element.Tab("main", "b", "B", panelB)))
        .AddChild(Element.StatusStrip()
            .AddChild(Element.Label("Ready")))
)
```

`TabControl` reçoit automatiquement `flex:1; min-height:0` depuis sa
classe de base, il grandit donc pour remplir son parent. Le panneau de
l'onglet actif a `overflow:auto`. `StatusStrip` est en `position:fixed` et
ne consomme pas d'espace dans la mise en page — il flotte en bas du viewport.

## `Page.FullBleed()` (v0.0.8) — supprimer le padding du corps

`FillViewport` conserve le padding de corps par défaut de 16 px pour que
les formulaires simples aient de l'air. Les mises en page de type shell IDE
où la `MenuBar` touche le bord supérieur et où un `SplitContainer` remplit
le reste nécessitent un corps sans padding :

```amalgame
Page.New()
    .FullBleed()                            // body padding:0
    .SetBody(
        Element.Stack()
            .AddChild(Element.MenuBar()...) // touche le bord supérieur
            .AddChild(Element.SplitContainer("row", 25)
                .AddChild(treePane)
                .AddChild(editorPane)
                .Fill())
            .AddChild(Element.StatusStrip()...))
```

`FullBleed` et `FillViewport` sont indépendants — `FullBleed` ne modifie
que le padding du corps ; `FillViewport` contrôle la hauteur de la mise en
page. La combinaison des deux est le shell IDE typique.

## `.Fill()` — désigner un enfant comme cible de flex-grow

Quand votre mise en page racine n'est pas un `TabControl` mais un autre
contenu de grande hauteur (un `Stack` de champs, un long `ListView`, un
`SplitContainer`), marquez l'enfant qui doit occuper l'espace restant et
défiler :

```amalgame
Element.Stack()
    .AddChild(Element.Heading("Inbox"))             // hauteur naturelle
    .AddChild(Element.ToolStrip()                   // hauteur naturelle
        .AddChild(Element.Button("Refresh")))
    .AddChild(longListViewContainer.Fill())         // prend le reste
    .AddChild(Element.StatusStrip()                  // pied de page fixe
        .AddChild(Element.Label("Connected")))
```

`.Fill()` ajoute la classe `amc-fill-child`, que la feuille de style de
base traduit par `flex:1; min-height:0; overflow:auto`.

## Mécanique du TabControl

CSS pur — aucun JavaScript requis pour la commutation d'onglets.

```amalgame
Element.TabControl("settings")
    .AddChild(Element.Tab("settings", "general",  "General",  generalBody)
        .Attr("checked", "checked"))    // ← onglet actif par défaut
    .AddChild(Element.Tab("settings", "advanced", "Advanced", advancedBody))
    .AddChild(Element.Tab("settings", "about",    "About",    aboutBody))
```

Sous le capot : chaque Tab est un `<label>` enveloppant un
`<input type=radio>` masqué, plus le corps du panneau. Tous les radios
partagent le `name` (le groupe), de sorte que cocher l'un décoche les
autres. Un sélecteur CSS de type sibling révèle uniquement le panneau de
l'onglet `:checked`.

- `groupName` doit être unique sur la page si vous avez plusieurs
  `TabControl`. Réutilisez-le entre les onglets d'un même contrôle.
- `id` est ajouté à l'`id` du radio pour que le `<label for=...>`
  fonctionne.
- Jusqu'à 32 onglets par `TabControl` (grille du modèle de base). Au-delà,
  surchargez le `grid-template-columns` de `.amc-tabs` depuis votre propre
  feuille de style.

## Mécanique du SplitContainer (v0.0.8)

```amalgame
Element.SplitContainer("row", 30)   // "row" → séparateur vertical
    .AddChild(leftPane)
    .AddChild(rightPane)
```

Deux volets plus un séparateur fin. Le séparateur est déplaçable : un
petit pont JS câble `pointerdown` → `pointermove` pour ajuster les valeurs
`flex-grow` des volets → `pointerup` termine le glisser. Le ratio de
division est limité à 5..95 pour qu'aucun volet ne puisse se réduire
complètement.

L'orientation `"column"` donne un séparateur horizontal (volets
haut/bas). Le conteneur gère son propre thème ; vous n'avez rien à
styliser pour obtenir un séparateur fonctionnel en thème clair ou sombre.

## MenuBar dans une mise en page (v0.0.8)

`MenuBar` est simplement un élément — placez-le là où vous mettriez
n'importe quel autre conteneur. L'emplacement habituel est le premier
enfant du corps de la page :

```amalgame
Page.New().FullBleed().SetBody(
    Element.Stack()
        .AddChild(Element.MenuBar()
            .AddChild(Element.MenuItem("File")
                .AddChild(Element.MenuOption("New",  "amc_new"))
                .AddChild(Element.MenuOption("Quit", "amc_quit"))))
        .AddChild(mainContent.Fill())
        .AddChild(Element.StatusStrip()
            .AddChild(Element.Label("Ready"))))
```

Liez les noms d'action avec `win.Bind("amc_new", handler)`. La `MenuBar`
porte `data-mode="common"` aujourd'hui ; la v0.1.0 ajoutera un
`data-mode="native"` optionnel qui bascule vers les menus natifs du
système d'exploitation sans modifier votre code AM.

## Thème — les sept variables CSS

La feuille de style de base expose sept variables qui basculent avec le
thème du système d'exploitation. Surchargez-en une ou plusieurs depuis
votre propre feuille de style :

| Variable        | Défaut clair  | Défaut sombre | Utilisée par |
|-----------------|---------------|--------------|---------|
| `--amc-bg`      | `#fff`        | `#1e1e1e`    | Fond du corps, fond des inputs |
| `--amc-fg`      | `#1a1a1a`     | `#e8e8e8`    | Texte du corps, texte des contrôles |
| `--amc-muted`   | `#6a6a6a`     | `#9a9a9a`    | Texte des légendes, barre d'état |
| `--amc-border`  | `#d0d0d0`     | `#404040`    | Bordures des inputs, bordures des cellules de tableau |
| `--amc-surface` | `#f5f5f5`     | `#2a2a2a`    | Fond des boutons, fond `<pre>`, fond des barres d'outils, fond de la MenuBar |
| `--amc-accent`  | `#0066cc`     | `#4a9eff`    | Anneaux de focus, curseur du slider, bordure de l'onglet actif, contour du jour courant du MonthCalendar, liens |
| `--amc-radius`  | `4px`         | `4px`        | Border-radius des inputs et boutons |

Le thème du système d'exploitation est détecté au moment du rendu et
écrit dans `<html data-theme="dark|light">`. Surchargez par page :

```amalgame
Page.New().SetTheme("dark")    // forcer le thème sombre
Page.New().SetTheme("light")   // forcer le thème clair
Page.New().SetTheme("auto")    // défaut — suivre le système d'exploitation
```

Vous pouvez également surcharger via une variable d'environnement :
`AMALGAME_UI_THEME=dark` l'emporte sur la détection du système
d'exploitation au moment de l'exécution.

### Basculement de thème en direct — `Page.AutoTheme(true)` (v0.0.8)

Par défaut, le thème du système d'exploitation est lu une seule fois au
moment de `Page.Render`. Si l'utilisateur bascule entre le mode sombre et
le mode clair pendant que l'application tourne, la page reste sur le thème
qu'elle avait au chargement.

Activez `AutoTheme` pour que la page suive le système en direct :

```amalgame
Page.New()
    .AutoTheme(true)
    .SetBody( … )
    .ApplyTo(win)
```

Sous le capot, le pont JS écoute
`matchMedia('(prefers-color-scheme: dark)').onchange` et bascule
`<html data-theme>` en conséquence — les variables `--amc-*` suivent.

`AutoTheme` est optionnel pour des raisons de compatibilité ascendante —
la v0.0.5 était livrée sans lui. Les versions futures pourraient en faire
le comportement par défaut.

### `Page.OnThemeChange(handler)` (v0.0.8)

Branchez-vous sur le même pont de thème en direct quand votre application
a besoin de réagir au-delà du CSS (re-rendre une icône SVG, mettre à jour
un indicateur `Pre`, journaliser le changement) :

```amalgame
Page.New()
    .OnThemeChange((req: string) => {
        // req est "light" ou "dark"
        Console.WriteLine("theme: " + req)
        return ""
    })
    .SetBody( … )
```

`OnThemeChange` implique `AutoTheme(true)`. Le handler se déclenche
également une fois au chargement initial de la page pour pouvoir
synchroniser les widgets avec le thème courant.

### Personnaliser les variables

```amalgame
Page.New()
    .AddCss("data:text/css,:root{--amc-accent:%23ff6b00;--amc-radius:8px}")
```

(`#` encodé en URL sous la forme `%23` car l'URL est une `data:` URL.) Ou
fournissez un vrai fichier de feuille de style :

```amalgame
Page.New()
    .AddCss("file:///abs/path/to/my-theme.css")
```

Trois façons d'intégrer du CSS :

```amalgame
// 1. Feuille de base + vos surcharges (superposées par-dessus)
Page.New().AddCss("file:///abs/app/overrides.css")

// 2. Ignorer la feuille de base, fournir votre propre feuille complète
Page.New().SetStylesheet("file:///abs/app/style.css")

// 3. Ignorer la feuille de base, superposer plusieurs feuilles
Page.New().NoBaseline()
    .AddCss("https://unpkg.com/@picocss/pico@2/css/pico.min.css")
    .AddCss("file:///abs/app/app.css")
```

## Verrouillage du chrome navigateur

Par défaut, le menu contextuel du clic droit et les raccourcis de
rechargement (F5, Ctrl+R / Cmd+R) de la webview sont bloqués. Ils
amèneraient l'utilisateur sur `about:blank` car le document est chargé
via `SetHtml` sans URL de support.

Les applications qui naviguent via `Window.Navigate(url)` vers une vraie
URL et souhaitent retrouver le chrome navigateur standard peuvent le
réactiver :

```amalgame
Page.New().AllowBrowserDefaults()
```

Les DevTools (`Ctrl+Shift+I` dans les builds de débogage) ne sont jamais
bloqués.

## Limites des chaînes fluentes

L'inférence de type d'amc avait un comportement quadratique sur les
chaînes `.AddChild(...)` au-delà de ~24 liens. Corrigé dans amc v0.8.16
(mémoïsation sur `InferTypeFromExpr` indexée par l'identité du pointeur
AST), les chaînes de n'importe quelle longueur sont donc maintenant
linéaires en temps. Le pattern avec `let` intermédiaires reste le style
recommandé de toute façon, car il se lit mieux :

```amalgame
let header: Element = Element.Stack()
    .AddChild(...)
let main:   Element = Element.Stack()
    .AddChild(...)
let footer: Element = Element.Stack()
    .AddChild(...)

Element.Stack()
    .AddChild(header)
    .AddChild(main)
    .AddChild(footer)
```

Si vous utilisez amc < 0.8.16, découpez les longues chaînes de la même
façon comme solution de contournement.
