# 05 — Étendre ui-web

Les trois échappatoires : HTML brut, bibliothèques JS et primitives
natives de l'OS.

## HTML brut

`Element.Raw(html)` ajoute un fragment HTML verbatim comme enfant de
n'importe quel élément. Utile pour injecter du markup déjà existant ou
des snippets spécialisés que les builders ne couvrent pas :

```amalgame
Element.Div().Id("logo-host").Raw("<svg width='80' height='80' viewBox='0 0 100 100'>"
    + "<circle cx='50' cy='50' r='40' fill='#4a9eff' stroke='#1a1a1a' stroke-width='2'/>"
    + "<text x='50' y='58' text-anchor='middle' fill='white' font-size='32'>A</text>"
    + "</svg>")
```

Le développeur est responsable de l'échappement. `Page.RenderElement`
émet la chaîne telle quelle ; les caractères spéciaux HTML (`<`, `&`,
`"`) doivent donc constituer du markup valide. Pour une interpolation
sûre de chaînes fournies par l'utilisateur, utilisez `Html.Escape(...)`
avant de les insérer.

## Balises d'éléments personnalisées

N'importe quelle balise HTML (y compris les éléments personnalisés) est
accessible via `new Element(tagName)` :

```amalgame
// HTML5 <details> / <summary>
new Element("details")
    .AddChild(new Element("summary").SetText("Plus d'infos"))
    .AddChild(Element.Label("Ce bloc se déploie au clic de l'utilisateur."))

// Nom d'un élément personnalisé (web components)
new Element("my-spinner").Attr("size", "32")
```

La même API fluente (`.Attr`, `.Style`, `.On(event, h)`,
`.AddChild`) fonctionne sur toutes les balises. La feuille de style de
base ne thème pas les balises inconnues — fournissez votre propre CSS
via `Page.AddCss` si vous souhaitez les styliser.

## Intégrer une bibliothèque JS

Les moteurs de webview sont de vrais navigateurs, donc toute bibliothèque
front-end distribuée en ESM / UMD / `<script>` fonctionne. Trois
patterns :

### 1. Inclure depuis un CDN ou un fichier local

```amalgame
Page.New()
    .AddCss("https://unpkg.com/tabulator-tables@5/dist/css/tabulator.min.css")
    .SetBody(Element.Stack()
        .AddChild(Element.Div().Id("table-host")))
    .ApplyTo(win)

// Monte la lib après le chargement de la page.
win.Eval("(function(){var s=document.createElement('script');s.src='https://unpkg.com/tabulator-tables@5/dist/js/tabulator.min.js';s.onload=function(){new Tabulator('#table-host',{data:[{n:1},{n:2}],columns:[{title:'N',field:'n'}]});};document.head.appendChild(s);})();")
```

### 2. Lier un callback JS de retour vers AM

```amalgame
win.Bind("amc_select_row", (req: string) => {
    Console.WriteLine("row selected: " + req)
    return "true"
})

win.Eval(
    "table.on('rowClick', function(e, row){"
    + "  window.amc_select_row(JSON.stringify(row.getData()));"
    + "});"
)
```

Le pattern est symétrique : les éléments rendus par AM émettent
`onclick="window._amc_N(...)"`, vous pouvez tout aussi bien `win.Bind`
n'importe quel nom et l'appeler depuis votre JS personnalisé.

### 3. Poser un point de montage que la lib remplit

```amalgame
Element.Div().Id("plot").Style("height:300px")
```

Après `ApplyTo`, évaluez l'initialisation de la lib :

```amalgame
win.Eval("Plotly.newPlot('plot', [{x:[1,2,3],y:[2,4,1],type:'bar'}], {});")
```

Utilisez `Page.AppendInner` / `Window.SetInnerHtml` si vous devez
remplacer le contenu du point de montage ultérieurement — mais gardez à
l'esprit que la plupart des bibliothèques JS allouent un état en dehors
du DOM et laisseront des fuites mémoire si vous n'appelez pas leur
méthode `destroy()` au préalable.

## Primitives côté C

Quand vous avez besoin d'une capacité native de l'OS que la webview
n'expose pas (barre d'état système, menus natifs, boîtes de dialogue de
fichiers au-delà de `<input type=file>`, accès au registre / au
trousseau de clés), ajoutez une primitive C dans
`runtime/Amalgame_UI_Web.c` et exposez-la à travers la façade.

Le pattern issu de la v0.0.5 elle-même — `Amalgame_UI_Web_OpenUrl` —
sert de modèle :

1. **Déclaration dans l'en-tête** `runtime/Amalgame_UI_Web.h` :

    ```c
    int Amalgame_UI_Web_OpenUrl(const char* url);
    ```

2. **Implémentation** dans `runtime/Amalgame_UI_Web.c` avec des
   `#ifdef` par plateforme :

    ```c
    int Amalgame_UI_Web_OpenUrl(const char* url) {
    #if defined(_WIN32)
        HINSTANCE r = ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
        return ((INT_PTR)r > 32) ? 0 : 1;
    #elif defined(__APPLE__)
        pid_t pid = fork();
        if (pid == 0) { execlp("open", "open", url, (char*)NULL); _exit(127); }
        return (pid > 0) ? 0 : 1;
    #else
        pid_t pid = fork();
        if (pid == 0) { execlp("xdg-open", "xdg-open", url, (char*)NULL); _exit(127); }
        return (pid > 0) ? 0 : 1;
    #endif
    }
    ```

3. **Wrapper dans la façade** `facade.am` :

    ```amalgame
    public static bool OpenExternalUrl(url: string) {
        var rc: int = 1
        @c {
            rc = (i64)Amalgame_UI_Web_OpenUrl(url);
        }
        return rc == 0
    }
    ```

4. **Injection automatique** dans `Page.ApplyTo` si la primitive doit
   être liée automatiquement pour chaque page (`_amc_openurl` y est
   lié pour que `Element.Link` fonctionne sans code applicatif).

La bibliothèque webview elle-même expose le handle de fenêtre natif via
`webview_get_window(w)` — HWND sous Windows, NSWindow* sous macOS,
GtkWindow* sous Linux. Castez et appelez les API natives à partir de là
pour une intégration plus profonde.

La surface C actuelle (voir `runtime/Amalgame_UI_Web.h`) est la suivante :

- `Amalgame_UI_Web_Create / SetTitle / SetSize / Navigate /
  SetHtml / Init / Eval / Run / Terminate / Destroy / Bind /
  Unbind` — l'API webview encapsulée.
- `Amalgame_UI_Web_OpenUrl(url)` — pont vers le navigateur de l'OS,
  utilisé par `Element.Link`.
- `Amalgame_UI_Web_DetectOSTheme()` — `"light"` / `"dark"` depuis
  gsettings / `defaults` / le registre, en respectant
  `AMALGAME_UI_THEME=…`.

Ajoutez de nouvelles primitives en suivant la même forme. Les widgets
MenuBar / SplitContainer / Dialog sont aujourd'hui livrés en pur
HTML/CSS/JS — un futur MenuBar natif optionnel (v0.1.0) viendra côtoyer
les primitives existantes sans casser l'API côté AM.

## Widgets personnalisés — le pattern Component

Quand un arbre d'éléments est réutilisé sur plusieurs écrans, promouvez-
le en classe Amalgame ordinaire dotée d'une méthode `Render()` :

```amalgame
public class LabeledInput {
    public Caption: string
    public Name:    string

    public LabeledInput(caption: string, name: string) {
        this.Caption = caption
        this.Name    = name
    }

    public Element Render() {
        return Element.Row()
            .AddChild(Element.Label(this.Caption).Size(120, 0))
            .AddChild(Element.Input(this.Name))
    }
}

// utilisation
let form: Element = Element.Stack()
form.AddChild(new LabeledInput("Name:",  "name").Render())
form.AddChild(new LabeledInput("Email:", "email").Render())
```

Distribuez des widgets réutilisables sous forme de package Amalgame —
`amc package add my-team-widgets` les intègre, et les consommateurs les
composent via `Render()` comme n'importe quelle autre source d'éléments.
Aucun support moteur requis ; c'est de la réutilisation pure du langage.

Livré en v0.0.9 — le pattern Component est désormais le point d'extension
documenté. Nous avons délibérément choisi une classe plate avec une
méthode `Render()` plutôt qu'une classe de base `abstract class
Component` : la résolution statique d'AM rend les surcharges virtuelles
sur un parent peu fiables ; une convention se lit mieux et s'exécute de
façon plus prévisible. Le modèle complet a trois niveaux (Element / Component / Form).

## Remplacer la feuille de style de base

Pour un re-skinning radical (utilisation de Pico, Tailwind, Bulma ou un
système de design fait maison), désactivez la feuille de base :

```amalgame
Page.New()
    .NoBaseline()
    .AddCss("https://unpkg.com/@picocss/pico@2/css/pico.min.css")
    .AddCss("file:///abs/app/app.css")
```

Les bridges form-collect / chrome-lockdown / link-routing continuent
d'être injectés. Seuls les styles visuels sont remplacés. Vous devrez
re-styliser vous-même les classes `.amc-statusstrip`, `.amc-tabs`,
`.amc-listview`, etc. si vous utilisez ces builders.

## La façade Form comme template rapide (v0.0.7)

Si votre application est une fenêtre principale unique, `Form` +
`Application.Run` supprime le boilerplate `Window + Page + ApplyTo +
Run + Destroy`. C'est du sucre syntaxique — sous le capot, c'est le même
chemin de code :

```amalgame
let f: Form = new Form("My App", 800, 600)
f.SetTheme("auto")
f.SetDebug(false)
f.OnLoad((req: string) => {
    // les appels Window.Bind différés vont ici
    return ""
})
f.SetBody( … )
Application.Run(f)
```

Champs publics et mutables plutôt que classe de base abstraite avec
surcharges, car la résolution statique d'AM ne résout pas de façon
fiable la méthode virtuelle d'un parent via une référence de sous-
classe — maintenir `Form` plate évite le piège. Voir
[`01-getting-started.md`](01-getting-started.md).

## Conseils de débogage

- **`new Window(..., true)`** active les DevTools. Clic droit dans la
  fenêtre ou `Ctrl+Shift+I` pour ouvrir l'inspecteur. Indispensable pour
  les problèmes CSS / JS.
- **`Console.WriteLine` depuis vos closures AM** affiche dans le terminal
  qui a lancé le binaire. La façon la plus simple de tracer ce qui
  arrive à un handler.
- **Les bridges internes ne journalisent rien par défaut.** Pour déboguer
  un problème de câblage, instrumentez la closure `_amc_openurl` de
  `Page.ApplyTo` ou vos propres callbacks `Window.Bind` avec
  `Console.WriteLine` pour voir la valeur de `req` qui arrive
  effectivement.
- **Les scripts de style `./build/dump_html`** — enveloppez une `Page`
  dans un binaire one-shot qui appelle `Page.Render()` et imprime le
  HTML résultant sur stdout. Plus rapide que de lancer la webview pour
  vérifier le markup. Voir `tests/dump_html.am` dans ce package.

## Ce qui est intentionnellement ABSENT de ui-web

Ces fonctionnalités appartiennent ailleurs ; faites-y appel via
l'écosystème Amalgame standard :

- **Serveur / client HTTP** — stdlib `Amalgame.Net`.
- **Surveillance du système de fichiers** — package `amalgame-io-filewatcher`.
- **Processus en arrière-plan** — stdlib `Process.Run`, ou le package
  `Service` pour les démons de longue durée.
- **Base de données** — package `amalgame-database-sqlite`.
- **Journalisation** — package `amalgame-logging`.

`ui-web` est le binding front-end ; combinez-le avec le reste de
l'écosystème pour construire des applications complètes.
