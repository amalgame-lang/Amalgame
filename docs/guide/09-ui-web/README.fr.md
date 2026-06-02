# `amalgame-ui-web` — guide du développeur

Créez des interfaces graphiques de bureau à l'apparence native dans
Amalgame, rendues par le moteur webview du système d'exploitation
(WebView2 sur Windows, WKWebView sur macOS, WebKitGTK sur Linux).

Ce guide est le point d'entrée pour tout ce dont vous avez besoin pour
livrer une application — installation, catalogue de widgets, modèle
d'événements, mise en page, thématisation, mises à jour partielles du
DOM, et les échappatoires pour HTML/CSS/JS personnalisés.

Pour la correspondance des contrôles WinForms et la vue d'ensemble
architecturale, consultez les docs associées :

- [`../winforms-mapping.md`](../winforms-mapping.md) — chaque
  contrôle de la boîte à outils WinForms, son état dans `ui-web`, et
  ce qui est différé / hors périmètre.
- [`../architecture.md`](../architecture.md) — le modèle à trois
  niveaux Element / Component / Form et la disposition de l'injection
  au runtime.

## Ordre de lecture

| Fichier | Sujet |
|---|---|
| [`01-getting-started.md`](01-getting-started.md) | Installation, prérequis, création de la première fenêtre, script de build. |
| [`02-widgets.md`](02-widgets.md) | Catalogue de chaque builder de widget `Element.*` avec des exemples. |
| [`03-events-and-state.md`](03-events-and-state.md) | OnClick, OnChange, OnResult, événements personnalisés, payload JSON de formulaire, patching partiel du DOM. |
| [`04-layout-and-theme.md`](04-layout-and-theme.md) | Stack / Row / Grid / Flow / Absolute / TabControl, FillViewport, les sept variables CSS. |
| [`05-extending.md`](05-extending.md) | `Element.Raw`, intégration d'une lib JS (Plotly / Tabulator / CodeMirror), primitives côté C, échappatoires natives. |

## Application minimale

Deux formes équivalentes — l'explicite `Window + Page` et le sucre
syntaxique `Form + Application.Run` (v0.0.7).

### Forme explicite

```amalgame
import Amalgame.UI.Web

class Program {
    public static void Main() {
        let win: Window = new Window("Hello", 480, 320, false)
        if (!win.IsValid()) { return }

        Page.New()
            .SetTitle("Hello")
            .SetBody(
                Element.Stack()
                    .AddChild(Element.Heading("Hello, Amalgame"))
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

### Sucre syntaxique Form

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

Build (Linux/Ubuntu, voir [`01-getting-started.md`](01-getting-started.md)
pour macOS / Windows) :

```sh
amc package add ui-web
./build.sh && ./hello
```

Thème système (clair / sombre suit le bureau), handler de clic lié à
la closure AM, retour JSON affiché formaté dans `#out`. Aucun HTML
écrit à la main.

## Ce que ce package est et n'est pas

**Est :**
- Une surface Amalgame typée au-dessus d'une webview du système
  d'exploitation.
- Un builder de widgets aligné sur WinForms (`Element.Button`,
  `Element.Input`, …) couvrant l'essentiel de la boîte à outils
  WinForms, plus les boîtes de dialogue modales, les menus, les
  arborescences, les panneaux divisés, le texte enrichi et un
  calendrier mensuel.
- Auto-collecte de l'état du formulaire — chaque input nommé effectue
  un aller-retour via un objet JSON livré à vos handlers.
- Une feuille de style de base thématisée via 7 variables CSS qui
  basculent avec le schéma de couleurs du système — en direct avec
  `Page.AutoTheme(true)` (v0.0.8) ou une seule fois au chargement par
  défaut.
- Des primitives de mise à jour partielle du DOM pour que les
  applications de type DataGrid puissent rafraîchir des lignes sans
  re-rendre la page.

**N'est pas :**
- Un toolkit en mode conservé (utilisez `Element` pour construire des
  arbres, pas des mutations `Button.Visible = false`).
- Un *vrai* WinForms natif — la MenuBar / Dialog / le menu contextuel
  se rendent en HTML/CSS, pas en chrome du système. Une MenuBar native
  opt-in est prévue pour la v0.1.0 (l'API AM ne changera pas).
  DataGridView, la zone de notification système, ToolTip en tant que
  widget, NotifyIcon et la pile d'impression ne sont pas dans le
  périmètre avant la v0.1+. Voir
  [`../winforms-mapping.md`](../winforms-mapping.md) pour le statut
  par contrôle.
- Un navigateur. Les URLs dans `<a href>` sont routées vers le
  navigateur du système via `Element.Link` ; la webview ne rend que
  le propre HTML de votre application.

## Modèle mental

```
┌──────────────────────────────────────────────────┐
│ Amalgame code                                    │
│   - builds an `Element` tree via fluent calls    │
│   - attaches OnClick / OnChange handlers (AM)    │
│   - calls `Page.New().ApplyTo(win)` once         │
│   - reacts to events with handlers + AppendInner │
└─────────────────────┬────────────────────────────┘
                      │ Page.Render → HTML string
                      │ Window.Bind → JS shim per handler
                      ▼
┌──────────────────────────────────────────────────┐
│ Webview (WebView2 / WKWebView / WebKitGTK)       │
│   - renders HTML + CSS + JS                      │
│   - user clicks → JS shim → Amalgame closure     │
│   - closure returns a value → routed via         │
│     OnResult or discarded                        │
└──────────────────────────────────────────────────┘
```

Si vous avez utilisé Tauri ou Electron, l'idée est identique —
interface webview côté front, back-end natif — mais le front est
*généré* à partir de code Amalgame typé plutôt qu'écrit en HTML à la
main.
