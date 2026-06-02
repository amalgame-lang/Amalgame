# 03 — Événements et état

Comment les actions de l'utilisateur dans la page rendue remontent jusqu'à
votre code Amalgame, comment l'état des formulaires circule, et comment
rafraîchir des parties du DOM sans re-rendre la page entière.

## Le modèle d'événements

Chaque événement DOM est acheminé via un mécanisme unique et uniforme :

```amalgame
Element.<widget>(...).On(eventName, handler)
```

`On("click", h)`, `On("change", h)`, `On("wheel", h)`, les événements
personnalisés comme `On("my-app-event", h)` — tous utilisent le même câblage.

Pour les événements courants alignés sur WinForms, des raccourcis existent :

| Raccourci                   | Événement DOM                  | Équivalent WinForms             |
|-----------------------------|----------------------------|---------------------------------|
| `.OnClick(h)`               | `click`                    | `Click`                         |
| `.OnDblClick(h)`            | `dblclick`                 | `DoubleClick`                   |
| `.OnChange(h)`              | `change` + `input` (pour les champs texte / textareas) | `TextChanged` / `CheckedChanged` / `SelectedIndexChanged` |
| `.OnFocus(h)`               | `focus`                    | `GotFocus` / `Enter`            |
| `.OnBlur(h)`                | `blur`                     | `LostFocus` / `Leave`           |
| `.OnMouseEnter(h)`          | `mouseenter`               | `MouseEnter`                    |
| `.OnMouseLeave(h)`          | `mouseleave`               | `MouseLeave`                    |
| `.OnKeyDown(h)`             | `keydown`                  | `KeyDown`                       |
| `.OnKeyUp(h)`               | `keyup`                    | `KeyUp`                         |

Un même Element peut porter plusieurs événements indépendamment :

```amalgame
Element.Input("user")
    .OnFocus((req: string) => req)
    .OnBlur((req: string) => req)
    .OnChange((req: string) => req)
```

## Signature du handler

Chaque handler est une closure de cette forme :

```amalgame
(req: string) => string
```

- `req` est un instantané JSON-encodé de chaque champ de formulaire nommé sur
  la page au moment où l'événement se déclenche.
- La valeur de retour doit être du JSON valide — soit `"true"`, une
  chaîne comme `"\"ok\""`, un nombre `"42"`, ou un objet plus riche.

Le handler le plus simple renvoie le formulaire tel quel :

```amalgame
Element.Button("Submit").OnClick((req: string) => req)
```

Pour retourner un texte libre, enveloppez-le avec `Json.EncodeString` :

```amalgame
Element.Button("Greet").OnClick((req: string) => Json.EncodeString("hello"))
```

## Charge utile du formulaire

Chaque élément possédant un attribut `name` (construit via les builders `Input`,
`Password`, `Select`, etc., ou via `.Bind(name)` sur un élément brut) est
auto-collecté par le bridge `window.__amc_collect` injecté par `Page.ApplyTo`.

Exemple de `req` pour un handler :

```json
{
  "user": "alice",
  "message": "Hi!",
  "newsletter": true,
  "priority": "normal",
  "theme": "dark"
}
```

- Cases à cocher → booléen.
- Boutons radio → la valeur de celui qui est coché dans le groupe ; clé absente
  si aucun n'est coché.
- Listboxes à sélection multiple → non auto-collectées en v0.0.5 — lisez
  `.selectedOptions` via `Window.Eval` jusqu'à la v0.0.6.
- `Element.RichTextBox(name)` (v0.0.9) → le `.innerHTML` de l'élément contenteditable
  (conserve le gras / l'italique / les listes).
- `Element.MonthCalendar(name, …)` (v0.0.9) → le jour sélectionné au format
  ISO `YYYY-MM-DD` ; clé absente si aucun jour n'est sélectionné.
- Autres champs → leur `.value` (toujours une chaîne en HTML, même
  pour `type=number` / `type=date`).

La même charge utile parvient à **chaque** événement de chaque Element — vous
n'avez pas à brancher chaque champ sur un handler spécifique. Lisez uniquement
les champs qui vous intéressent.

## OnResult — acheminer la valeur de retour

Sans `OnResult`, la valeur de retour du handler est ignorée. Avec lui,
le bridge écrit le résultat formaté en JSON dans un élément cible par id :

```amalgame
Element.Stack()
    .AddChild(Element.Button("Submit")
        .OnClick((req: string) => req)
        .OnResult("out"))            // ← écrit la valeur de retour du handler
                                     //   dans #out
    .AddChild(Element.Pre("").Id("out"))
```

- Le routeur appelle d'abord `JSON.stringify(JSON.parse(r), null, 2)`
  — le JSON valide s'affiche formaté. Les chaînes brutes passent telles quelles.
- `OnResult(targetId)` est persistant vis-à-vis des setters d'événements : tout
  appel `On*` placé AVANT lui capture la route. Tout appel `On*` APRÈS ne la
  capture pas, sauf à redéfinir `OnResult`.
- Passez `""` pour réinitialiser.

Pour traiter une chaîne brute et éviter le formattage JSON, retournez-la
sans guillemets depuis le handler — `Json.EncodeString` et le routeur
coopèrent de sorte qu'un retour de type chaîne s'affiche comme texte cité dans
le `<pre>`.

## Collisions de noms de bind

Si deux Elements ont le même `id`, `OnResult` ne met à jour que le premier
résultat (`document.getElementById`). Idem pour les cibles de `Page.PatchInner` /
`AppendInner` — gardez des ids uniques.

L'auto-collect traite les valeurs `name` comme des clés de formulaire : deux
widgets `<input name="x">` s'écrasent mutuellement dans la charge utile. Le
partage de groupe radio est intentionnel, mais choisissez sinon des noms
distincts.

## Mises à jour partielles du DOM

Re-rendre la page entière à chaque changement d'état est coûteux et fait perdre
le focus, la position de défilement, etc. Utilisez l'API de patch pour les
changements incrémentaux.

### Opérations DOM brutes (`Window.*`)

| Appel | Ce qu'il fait |
|---|---|
| `Window.SetInnerHtml(id, html)` | Remplace le HTML interne de `#id`. |
| `Window.AppendHtml(id, html)`   | Ajoute du HTML comme dernier enfant de `#id`. |
| `Window.RemoveElement(id)`      | Supprime l'élément avec l'id donné. |

```amalgame
win.SetInnerHtml("status", "<span>Saved.</span>")
```

Ce sont des échappatoires — aucun binding d'événement n'est câblé
automatiquement, donc tout élément possédant un `onclick` n'aura pas de
handler AM à moins de le `Window.Bind` séparément.

### Patch typé par Element (`Page.*`)

Les wrappers typés rendent un sous-arbre `Element`, câblent tout nouveau
`OnClick` / `OnChange` qu'il contient, et injectent le résultat via
les opérations brutes ci-dessus :

| Appel | Ce qu'il fait |
|---|---|
| `Page.PatchInner(win, id, element)`  | Rend `element` et remplace les enfants de `#id`. |
| `Page.AppendInner(win, id, element)` | Rend `element` et l'ajoute aux enfants de `#id`. |

```amalgame
let row: Element = Element.ListViewRow(["new.txt", "0 KB", "today"])
page.AppendInner(win, "files-body", row)
```

Les événements dans le nouveau sous-arbre reçoivent des noms de bind
`_amc_N` fraîchement alloués et sont auto-enregistrés. Le `Page.Counter`
continue de croître entre les patches — les noms restent uniques.

### Motif : mise à jour en direct d'une DataGrid

```amalgame
public static void Main() {
    let win: Window = new Window("Files", 600, 400, false)
    if (!win.IsValid()) { return }

    let cols: List<string> = ["Name", "Size", "Modified"]
    let page: Page = Page.New().SetTitle("Files").SetBody(
        Element.Stack()
            .AddChild(Element.Heading("Files"))
            .AddChild(Element.ListView(cols, "files-body"))
            .AddChild(Element.Button("Refresh")
                .Attr("onclick", "window.amc_refresh('');"))
    )

    var counter: int = 0
    win.Bind("amc_refresh", (req: string) => {
        counter = counter + 1
        let r: List<string> = [
            "row-" + String_FromInt(counter) + ".txt",
            "0 KB",
            "now"
        ]
        page.AppendInner(win, "files-body", Element.ListViewRow(r))
        return "true"
    })

    page.ApplyTo(win)
    win.Run()
    win.Destroy()
}
```

Chaque clic sur Refresh ajoute une nouvelle ligne au tableau sans re-rendre
l'en-tête, le bouton, ni les lignes existantes.

## Handlers de Dialog (v0.0.6+)

`Dialog.Info` / `Dialog.Confirm` / `Dialog.OpenFile` etc.
**ne** livrent **pas** la charge utile du formulaire de la page. Ce sont des
bridges à usage unique : le handler reçoit le choix de l'utilisateur sous forme
de chaîne brute.

| Appel | Forme de `req` |
|---|---|
| `Dialog.Info / Warning / Error`                 | `"ok"` (bouton) ou `"cancel"` (Échap / fond) |
| `Dialog.Confirm`                                | `"ok"` ou `"cancel"` |
| `Dialog.YesNoCancel`                            | `"yes"`, `"no"`, ou `"cancel"` |
| `Dialog.OpenFile`                               | chaîne nom de fichier (`""` en cas d'annulation) |
| `Dialog.OpenFileContent` (v0.0.8)               | `""` en cas d'annulation, sinon JSON `{"name":"…","content":"…"}` |
| `Dialog.SaveFile`                               | `"ok"` une fois le téléchargement lancé |

```amalgame
Dialog.Confirm(win, "Quit?", "Discard unsaved changes?",
    (req: string) => {
        if (req == "ok") { win.Terminate() }
        return ""
    })
```

Si vous avez besoin de la charge utile du formulaire en plus du choix de
l'utilisateur, capturez-la une fois via `Window.Eval` et un champ caché, puis
lisez-la depuis l'intérieur du handler de dialog — le bridge d'auto-collect
par événement n'est pas câblé à travers les appels `Dialog.*`.

## Handlers d'action MenuBar (v0.0.8)

`Element.MenuOption(label, actionName)` appelle
`window.<actionName>('')` au clic. L'argument chaîne vide signifie que le
handler reçoit toujours `""` comme `req` — même mise en garde que pour les
Dialogs : bindez d'abord un collecteur `Window.Eval` séparé si vous avez besoin
de l'état du formulaire quand l'option de menu se déclenche.

```amalgame
win.Bind("amc_save", (req: string) => {
    // req est "" — collectez le formulaire via le bridge habituel :
    win.Eval("window._amc_save_payload = JSON.stringify(window.__amc_collect());")
    // … puis lisez window._amc_save_payload depuis un Eval suivant.
    return ""
})
```

La plupart des applications n'en ont pas besoin — les actions de menu ont
tendance à piloter le flux de haut niveau (Ouvrir / Enregistrer / Quitter) et
à lire l'état via des dialogs dédiés. Quand elles ont besoin du formulaire, un
`Window.Bind` déclenché par un `Button` dans la page est plus simple qu'une
MenuOption.

## Événements JS personnalisés

Tout ce que le navigateur émet peut être bindé :

```amalgame
Element.Div().Id("drop-zone")
    .On("drop",      (req: string) => req)
    .On("dragover",  (req: string) => req)
    .On("dragleave", (req: string) => req)
```

La charge utile JSON est toujours l'instantané du formulaire — pour lire
l'événement lui-même (ex. coordonnées de glisser, fichiers déposés), injectez
un petit shim JS via `Window.Eval` qui capture les données pertinentes de
l'événement dans un champ caché, puis lisez ce champ caché au prochain appel
bindé.

## Opt-out `data-amc-internal`

`Element.Link(text, url)` et l'intercepteur global `<a>` acheminent
les URLs http(s) vers le navigateur de l'OS. Si vous souhaitez qu'une ancre
spécifique reste dans la webview (navigation interne à l'app), marquez-la :

```amalgame
new Element("a")
    .Attr("href", "#page2")
    .Attr("data-amc-internal", "true")
    .SetText("Section 2")
```

Le routage par défaut vers le navigateur l'ignore alors.

## Ce que `Window.Bind` fait réellement

Le mécanisme sous-jacent. `Window.Bind(name, closure)` fait
deux choses au niveau de la webview :

1. Enregistre `name` dans la table de binding interne (capacité :
   `AMALGAME_UI_WEB_MAX_BINDINGS = 64`).
2. Injecte un shim JS : chaque fois que `window.<name>(args...)` est
   appelé depuis la page, l'appel est mis en file d'attente, le trampoline C
   réveille la closure AM, la closure s'exécute, et sa valeur de retour
   parvient au côté JS sous forme de résolution de `Promise`.

C'est pourquoi la signature de chaque handler d'événement est `(req: string) => string`
— bind ne connaît que les chaînes JSON.
