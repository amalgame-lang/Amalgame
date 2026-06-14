# 02 — Catalogue de widgets

Tous les constructeurs statiques `Element.<Nom>(...)`, regroupés par
catégorie.

Convention : chaque constructeur retourne un `Element`, ce qui permet
de composer des chaînes fluentes — `.AddChild(...)`, `.Attr(...)`, `.Style(...)`,
`.On<Event>(...)` fonctionnent sur tous les widgets.

## Texte et titres

| Constructeur | HTML | Remarques |
|---|---|---|
| `Element.Heading(text)`        | `<h1>` | Grand titre de page. Un seul par page. |
| `Element.Label(text)`          | `<p>`  | Paragraphe en lecture seule. |
| `Element.Pre(text)`            | `<pre>`| Panneau de sortie en police à chasse fixe. Souvent associé à `.Id(...)` pour qu'`OnResult` puisse y router le contenu. |
| `Element.Link(text, url)`      | `<a href>` | Lien hypertexte vers le navigateur de l'OS. Voir les [remarques sur LinkLabel](#linklabel) ci-dessous. |

```amalgame
Element.Stack()
    .AddChild(Element.Heading("Hello"))
    .AddChild(Element.Label("Welcome to the demo."))
    .AddChild(Element.Pre("(submit to see the payload)").Id("out"))
```

### LinkLabel

`Element.Link(text, url)` émet un `<a href>` dont l'attribut `onclick`
route via le navigateur de l'OS (xdg-open / open /
ShellExecute). L'utilisateur n'atterrit jamais dans la webview.

Pour naviguer dans la même fenêtre, créez un
`new Element("a")` et attachez votre propre `.OnClick(h)`.

## Boutons

| Constructeur | HTML | Remarques |
|---|---|---|
| `Element.Button(caption)`      | `<button>` | À câbler via `.OnClick(handler)`. |

```amalgame
Element.Button("Save")
    .OnClick((req: string) => req)        // echo the form
    .OnResult("out")                       // dump into #out
```

`Enabled(false)` produit un bouton désactivé grisé. `Tooltip(t)`
ajoute une infobulle native de l'OS via l'attribut HTML `title`.

## Saisies sur une ligne

| Constructeur | HTML | WinForms |
|---|---|---|
| `Element.Input(name)`                    | `<input type=text>`   | TextBox |
| `Element.Password(name)`                 | `<input type=password>` | TextBox (masqué) |
| `Element.Number(name, min, max, step)`   | `<input type=number>` composite + ▲/▼ | NumericUpDown |
| `Element.Slider(name, min, max, step)`   | `<input type=range>`  | TrackBar |
| `Element.DatePicker(name)`               | `<input type=date>`   | DateTimePicker (date) |
| `Element.TimePicker(name)`               | `<input type=time>`   | DateTimePicker (heure) |
| `Element.ColorPicker(name)`              | `<input type=color>`  | ColorDialog intégré |
| `Element.MaskedTextBox(name, pattern, inputmode)` | `<input pattern=… inputmode=…>` | MaskedTextBox |

Chaque `name` est collecté automatiquement par le pont d'état du formulaire —
les handlers reçoivent un objet JSON dont les clés sont ces noms. Voir
[`03-events-and-state.md`](03-events-and-state.md#charge-utile-du-formulaire).

```amalgame
Element.Row()
    .AddChild(Element.Label("Quantity:").Size(120, 0))
    .AddChild(Element.Number("qty", 1, 100, 1))
```

Les valeurs min/max de `Number` sont 0 par défaut — passez `(name, 0, 0, 0)` pour
laisser les trois attributs non définis.

Depuis la v0.0.5, `Number` est un composant composite (le `<input type=number>`
plus une colonne de boutons verticaux `▲` / `▼`). WebKitGTK ne dessine pas
le `::-webkit-inner-spin-button` natif, donc les boutons sont
toujours visibles et se comportent de manière identique sur les trois moteurs OS.

## Texte multi-lignes

| Constructeur | HTML | WinForms |
|---|---|---|
| `Element.Textarea(name)`        | `<textarea>` | TextBox (Multiline) |
| `Element.RichTextBox(name)`     | `<div contenteditable=true>` | RichTextBox (v0.0.9) |

```amalgame
Element.Textarea("message")
    .Attr("placeholder", "Type a message…")
    .Size(0, 80)        // height in px; 0 = auto width

Element.RichTextBox("notes")
    .Attr("placeholder-text", "Write notes here…")
    .Style("min-height:120px")
```

`textarea` n'est intentionnellement pas redimensionnable par défaut —
la poignée de glissement déstabilise les mises en page déclaratives. Réactivez via
`.Style("resize:vertical")` si nécessaire.

`RichTextBox` (v0.0.9) accepte le formatage en ligne via les raccourcis
clavier intégrés du navigateur (`Ctrl-B` / `Ctrl-I` / `Ctrl-U`) et le contenu
riche collé. Le payload du formulaire rapporte le contenu enrichi sous forme de HTML
interne sous la clé `name` du widget.

## Widgets booléens / de choix

| Constructeur | HTML | WinForms |
|---|---|---|
| `Element.CheckBox(name)`               | `<input type=checkbox>` | CheckBox (nu) |
| `Element.CheckBoxLabel(name, caption)` | `<label><input type=checkbox> caption</label>` | CheckBox (.Text) |
| `Element.Radio(name, value)`           | `<input type=radio>` | RadioButton (nu) |
| `Element.RadioLabel(name, value, caption)` | `<label><input type=radio> caption</label>` | RadioButton (.Text) |

Utilisez la variante `*Label` lorsque vous souhaitez que la légende soit
cliquable — correspond à la propriété `Text` de WinForms et aux attentes des utilisateurs.

```amalgame
Element.Stack()
    .AddChild(Element.CheckBoxLabel("agree", "I agree to the terms"))
    .AddChild(Element.Row()
        .AddChild(Element.Label("Priority:").Size(120, 0))
        .AddChild(Element.RadioLabel("priority", "low",    "Low"))
        .AddChild(Element.RadioLabel("priority", "normal", "Normal")
            .Attr("checked", "checked"))
        .AddChild(Element.RadioLabel("priority", "high",   "High")))
```

Les boutons radio d'un même groupe `name` sont mutuellement exclusifs.

## Listes

| Constructeur | HTML | WinForms |
|---|---|---|
| `Element.Select(name)` + `Element.Option(value, label)` | `<select><option>` | ComboBox |
| `Element.ListBox(name, size)`                            | `<select multiple>` | ListBox |
| `Element.CheckedListBox(name)` + `Element.CheckedItem(name, value, label)` | `<ul>` de `<input type=checkbox>` | CheckedListBox |
| `Element.ListView(headers, bodyId)` + `Element.ListViewRow(values)` | `<table>` | ListView (mode détails) |

```amalgame
let cols: List<string> = ["Name", "Size", "Modified"]
let r1:  List<string> = ["readme.md", "2 KB", "today"]

Element.ListView(cols, "files-body")
    .AddChild(Element.ListViewRow(r1))
```

Le paramètre `bodyId` est l'id du `<tbody>` — passez-le lorsque vous utiliserez
`Page.AppendInner(win, bodyId, …)` pour faire grandir le tableau au
moment de l'exécution (voir [`03-events-and-state.md`](03-events-and-state.md#mises-à-jour-partielles-du-dom)).
Passez `""` pour omettre l'id.

## Arborescence (v0.0.7)

| Constructeur | Rôle | WinForms |
|---|---|---|
| `Element.TreeView()`               | Conteneur racine.                              | TreeView |
| `Element.TreeNode(caption)`        | Nœud extensible façon dossier (HTML5 `<details>`). | TreeNode |
| `Element.TreeLeaf(caption)`        | Élément terminal.                              | TreeNode (feuille) |

```amalgame
Element.TreeView()
    .AddChild(Element.TreeNode("src")
        .AddChild(Element.TreeNode("parser")
            .AddChild(Element.TreeLeaf("ast.am"))
            .AddChild(Element.TreeLeaf("parser.am")))
        .AddChild(Element.TreeLeaf("main.am")))
    .AddChild(Element.TreeLeaf("README.md"))
```

Construit sur `<details>` / `<summary>`, donc le développement/réduction et la
navigation clavier fonctionnent sans aucun JS. Passez `.Attr("open", "open")` sur un
`TreeNode` pour le rendre développé par défaut. Le thème de base gère
le caret `▶` rotatif, le fond au survol et l'indentation de 18 px
via les variables `--amc-*`.

## Widgets d'affichage

| Constructeur | HTML | WinForms |
|---|---|---|
| `Element.ProgressBar(value, max)` | `<progress>`               | ProgressBar (`value < 0` = indéterminé) |
| `Element.Image(src)`              | `<img>`                    | PictureBox / Image |
| `Element.PictureBox(src)`         | `<img>` (alias)            | PictureBox |
| `Element.Iframe(url)`             | `<iframe>`                 | WebBrowser |
| `Element.MonthCalendar(name, year, month)` | grille (`<table>`) + en-tête ◀ / mois / année / ▶ | MonthCalendar (v0.0.9 + navigateur v0.0.10) |

```amalgame
Element.Row()
    .AddChild(Element.Label("Loading:").Size(80, 0))
    .AddChild(Element.ProgressBar(42, 100))

Element.MonthCalendar("birthday", 2026, 5)
```

`MonthCalendar` (v0.0.9) affiche une grille mensuelle intégrée. L'en-tête
(v0.0.10) porte ◀ / menu déroulant mois / menu déroulant année / ▶ pour permettre
à l'utilisateur de naviguer directement vers n'importe quel mois ou année. Cliquer sur un jour
le met en surbrillance et transmet la sélection sous forme de chaîne ISO `YYYY-MM-DD`
sous la clé `name` du widget dans le payload du formulaire (la
clé est absente si aucun jour n'est sélectionné).

## Menus (v0.0.8)

La MenuBar rendue en HTML, compatible tous OS — même forme que le
`MenuStrip` de WinForms, thématisée avec `--amc-*`. Une variante native OS (Win32
`HMENU` / NSMenu / GtkMenuBar) est prévue en opt-in via
`data-mode="native"` en v0.1.0 ; l'API est d'ores et déjà compatible vers l'avant.

| Constructeur | Rôle | WinForms |
|---|---|---|
| `Element.MenuBar()`                           | Conteneur `<nav>` — barre de menus en haut de fenêtre. | MenuStrip |
| `Element.MenuItem(label)`                     | Élément de premier niveau avec un panneau déroulant — en interne un `<details>` dont le `<summary>` est le libellé. | ToolStripMenuItem |
| `Element.MenuOption(label, actionName)`       | `<button>` dans un menu déroulant qui appelle `window.<actionName>('')` au clic. | ToolStripMenuItem (feuille) |
| `Element.MenuSeparator()`                     | `<hr>` thématisé entre les options. | ToolStripSeparator |
| `Element.ContextMenu(targetId)`               | Menu contextuel au clic droit — réutilise MenuOption / MenuSeparator. | ContextMenuStrip |

```amalgame
Element.MenuBar()
    .AddChild(Element.MenuItem("File")
        .AddChild(Element.MenuOption("New",   "amc_new"))
        .AddChild(Element.MenuOption("Open…", "amc_open"))
        .AddChild(Element.MenuSeparator())
        .AddChild(Element.MenuOption("Quit",  "amc_quit")))
    .AddChild(Element.MenuItem("Edit")
        .AddChild(Element.MenuOption("Undo",  "amc_undo")))
```

Liez chaque nom d'action via `win.Bind("amc_new", handler)`. Un
écouteur `click` global ferme tout menu ouvert lorsque l'utilisateur
clique en dehors ; `Escape` ferme à la fois les menus déroulants de la barre de menus et
le menu contextuel.

`ContextMenu` se rattache à l'élément hôte dont vous passez l'id
et écoute l'événement DOM `contextmenu` :

```amalgame
let cm = Element.ContextMenu("workspace")
    .AddChild(Element.MenuOption("Cut",   "amc_cut"))
    .AddChild(Element.MenuOption("Copy",  "amc_copy"))
    .AddChild(Element.MenuSeparator())
    .AddChild(Element.MenuOption("Paste", "amc_paste"))

Element.Div().Id("workspace").Class("amc-ctx-target")
    .AddChild(cm)
    .AddChild( … your actual content … )
```

## Conteneurs et mise en page

Ces éléments ne portent pas de données, ils organisent les enfants. Référence
complète dans [`04-layout-and-theme.md`](04-layout-and-theme.md).

| Constructeur | Rôle |
|---|---|
| `Element.Div()`                          | Conteneur bloc générique. |
| `Element.Panel()`                        | Alias de `Div` (nom WinForms). |
| `Element.Stack()`                        | Colonne flex avec un espacement de 8 px. |
| `Element.Row()`                          | Ligne flex avec un espacement de 8 px. |
| `Element.Flow(direction)`                | Flex avec retour à la ligne — `"row"` ou `"column"`. |
| `Element.Grid(rows, cols, gap)`          | CSS Grid. Passez `rows=0` pour un nombre de lignes implicite. |
| `Element.AbsoluteContainer()`            | Parent `position:relative` pour les enfants avec `.Position(x, y)`. |
| `Element.GroupBox(title)`                | `<fieldset><legend>` — section avec titre. |
| `Element.TabControl(group)` + `Element.Tab(group, id, label, body)` | Onglets CSS purs (radio + sélecteur de frère). |
| `Element.SplitContainer(orientation, ratio)` | Conteneur redimensionnable à deux volets (v0.0.8). |
| `Element.ToolStrip()`                    | Rangée de boutons horizontale thématisée. |
| `Element.StatusStrip()`                  | Barre d'état fixée en bas. |

```amalgame
Element.GroupBox("Personal info")
    .AddChild(Element.Input("name").Attr("placeholder", "Name"))
    .AddChild(Element.Input("email").Attr("placeholder", "Email"))
```

`SplitContainer` (v0.0.8) prend une orientation (`"row"` →
gauche/droite avec un séparateur vertical, `"column"` → haut/bas)
et un ratio initial en pourcentage de 5 à 95 (par ex. `30` pour un
découpage 30/70). Le séparateur est déplaçable à la souris, au stylet ou au toucher :

```amalgame
Element.SplitContainer("row", 30)
    .AddChild(Element.Stack().AddChild( … left pane … ))
    .AddChild(Element.Stack().AddChild( … right pane … ))
```

## Boîtes de dialogue (v0.0.6 → v0.0.8)

Boîtes de message modales et sélecteurs de fichiers. Tous les points d'entrée sont
des méthodes statiques sur `Dialog`, pas des constructeurs Element — ils ne
s'insèrent pas dans l'arbre de la page ; ils s'affichent à la demande.

### Boîtes de message (v0.0.6)

| Appel | Boutons | Équivalent WinForms |
|---|---|---|
| `Dialog.Info(win, title, message, handler)`         | OK         | `MessageBox.Show(... Information)` |
| `Dialog.Warning(win, title, message, handler)`      | OK         | `MessageBox.Show(... Warning)` |
| `Dialog.Error(win, title, message, handler)`        | OK         | `MessageBox.Show(... Error)` |
| `Dialog.Confirm(win, title, message, handler)`      | OK/Cancel  | `MessageBox.Show(... OKCancel)` |
| `Dialog.YesNoCancel(win, title, message, handler)`  | Yes/No/Cancel | `MessageBox.Show(... YesNoCancel)` |
| `Dialog.Show(win, kind, title, message, buttons, handler)` | au choix | bas niveau |

Le handler reçoit l'id du bouton cliqué dans `req` —
`"ok"` / `"cancel"` / `"yes"` / `"no"` — ou `"cancel"` si
l'utilisateur ferme avec Echap ou un clic sur l'arrière-plan.

```amalgame
Dialog.Confirm(win, "Quit?", "Discard unsaved changes?",
    (req: string) => {
        if (req == "ok") { win.Terminate() }
        return ""
    })
```

L'implémentation utilise l'élément HTML `<dialog>` — le piège de focus, la
fermeture par Echap et le voile d'arrière-plan sont fournis gratuitement par le navigateur.
L'en-tête porte un accent coloré (bleu / orange / rouge) selon le type.

### Sélecteurs de fichiers (v0.0.7 + v0.0.8)

| Appel | Ce que reçoit le handler | Remarques |
|---|---|---|
| `Dialog.OpenFile(win, accept, handler)`                       | chaîne nom de fichier (ou `""` en cas d'annulation) | Bac à sable navigateur : nom de fichier uniquement, sans chemin. |
| `Dialog.OpenFileContent(win, accept, binary, handler)` (v0.0.8) | JSON `{"name":"…","content":"…"}` (texte ou base64) | `binary=true` → `readAsDataURL` puis suppression du préfixe. |
| `Dialog.SaveFile(win, filename, content, mimeType, handler)`  | `"ok"` une fois le téléchargement initié | Bac à sable : impossible de savoir si l'utilisateur a accepté Enregistrer sous. |

```amalgame
Dialog.OpenFileContent(win, ".txt,.json", false,
    (payload: string) => {
        // payload = ""  on cancel
        // payload = {"name":"notes.txt","content":"Hello, file…"} on success
        return ""
    })
```

## Sorties de secours

Lorsqu'aucun constructeur ne convient :

```amalgame
// 1. Use a raw HTML tag — same fluent API as the builders.
new Element("aside")
    .Class("my-side-panel")
    .AddChild(Element.Label("Custom widget"))

// 2. Inject raw HTML inside an element.
Element.Div().Id("plot").Raw("<svg width=200 height=200>...</svg>")
```

Les deux formes produisent du HTML ordinaire ; le pont de collecte automatique fonctionne
toujours pour tout `[name]` input à l'intérieur.

Pour des sorties de secours plus avancées — bibliothèques JS, primitives C personnalisées,
surcharges CSS thématisées — voir [`05-extending.md`](05-extending.md).

## Accesseurs de propriétés (sur tout Element)

Miroir du panneau Propriétés du concepteur WinForms. Chacun retourne `this`
pour permettre l'enchaînement.

| Accesseur | Rôle |
|---|---|
| `.SetText(t)`           | Contenu texte interne. |
| `.Id(id)`               | Id HTML. Obligatoire lorsque ciblé par `OnResult` / `AppendInner`. |
| `.Class(cls)`           | Classe CSS. Plusieurs classes autorisées (séparées par des espaces). |
| `.Style(css)`           | Ajoute du CSS en ligne. Cumulatif — plusieurs appels se superposent. |
| `.Attr(name, value)`    | Définit n'importe quel attribut HTML (sortie de secours). Remplace en cas de clé dupliquée (depuis v0.0.8). |
| `.Size(w, h)`           | Taille en pixels ; 0 sur un axe laisse la valeur non définie. |
| `.Position(x, y)`       | Placement absolu (à utiliser dans `AbsoluteContainer`). |
| `.Visible(b)`           | False → `display:none`. |
| `.Enabled(b)`           | False → désactivé + grisé. |
| `.Tooltip(t)`           | Infobulle native de l'OS (attribut `title`). |
| `.TabIndex(n)`          | Ordre de focus clavier. |
| `.ForeColor(css)`       | Raccourci pour `color:` (accepte `#hex`, `var(--amc-...)`, nommé). |
| `.BackColor(css)`       | Raccourci pour `background:`. |
| `.Font(family, sizePx)` | Raccourci pour `font-family` + `font-size`. |
| `.DataTag(payload)`     | Aller-retour d'une chaîne arbitraire via l'attribut `data-tag`. |
| `.Fill()`               | Marque cet enfant comme cible flex-grow / défilement dans son parent. |
| `.Bind(name)`           | Clé de formulaire déclarative (équivalent à `.Attr("name", name)`). |
| `.Raw(html)`            | Ajoute un fragment HTML brut comme enfant. |

Pour les événements (`.OnClick`, `.OnChange`, `.OnFocus`, …) voir
[`03-events-and-state.md`](03-events-and-state.md).
