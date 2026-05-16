# `amalgame-ui-web` — developer guide

Build native-feeling desktop GUIs in Amalgame, rendered by the OS
webview engine (WebView2 on Windows, WKWebView on macOS, WebKitGTK
on Linux).

This guide is the entry point for everything you need to ship an
app — installation, the widget catalogue, event model, layout,
theming, partial-DOM updates, and the escape hatches for custom
HTML/CSS/JS.

For the WinForms widget cross-reference and the architectural
overview, see the sibling docs:

- [`../winforms-mapping.md`](../winforms-mapping.md) — every
  WinForms toolbox control, where it stands in `ui-web`, and
  what's deferred / out of scope.
- [`../architecture.md`](../architecture.md) — the three-level
  Element / Component / Form model and the runtime injection layout.

## Reading order

| File | Topic |
|---|---|
| [`01-getting-started.md`](01-getting-started.md) | Install, prereqs, scaffold the first window, build script. |
| [`02-widgets.md`](02-widgets.md) | Catalogue of every `Element.*` widget builder with examples. |
| [`03-events-and-state.md`](03-events-and-state.md) | OnClick, OnChange, OnResult, custom events, JSON form payload, partial DOM patching. |
| [`04-layout-and-theme.md`](04-layout-and-theme.md) | Stack / Row / Grid / Flow / Absolute / TabControl, FillViewport, the seven CSS variables. |
| [`05-extending.md`](05-extending.md) | `Element.Raw`, embedding a JS lib (Plotly / Tabulator / CodeMirror), C-side primitives, native escape hatches. |

## Minimal app

Two equivalent shapes — the explicit `Window + Page` and the
`Form + Application.Run` sugar (v0.0.7).

### Explicit shape

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

### Form sugar

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

Build (Linux/Ubuntu, see [`01-getting-started.md`](01-getting-started.md)
for macOS / Windows):

```sh
amc package add ui-web
./build.sh && ./hello
```

OS-themed (light / dark follows the desktop), Click-handler bound
to the AM closure, return JSON pretty-printed into `#out`. No HTML
authored by hand.

## What this package is and isn't

**Is:**
- A typed Amalgame surface over an OS webview.
- A WinForms-aligned widget builder (`Element.Button`,
  `Element.Input`, …) covering the bulk of the WinForms toolbox
  plus modal dialogs, menus, tree views, split panes, rich-text,
  and a month calendar.
- Form-state auto-collection — every named input round-trips
  through a JSON object delivered to your handlers.
- A baseline stylesheet themed via 7 CSS variables that flip
  with the OS color scheme — live with `Page.AutoTheme(true)`
  (v0.0.8) or once-at-load by default.
- Partial-DOM update primitives so DataGrid-style apps can
  refresh rows without re-rendering the page.

**Isn't:**
- A retained-mode toolkit (use `Element` to build trees, not
  `Button.Visible = false` mutations).
- A *native* WinForms — the MenuBar / Dialog / context menu
  render as HTML/CSS, not OS chrome. An opt-in native MenuBar
  is planned for v0.1.0 (the AM API won't change). DataGridView,
  system tray, ToolTip-as-widget, NotifyIcon and the printing
  stack are not in scope before v0.1+. See
  [`../winforms-mapping.md`](../winforms-mapping.md) for the
  per-control status.
- A browser. URLs in `<a href>` route through the OS browser via
  `Element.Link`; the webview only renders your app's own HTML.

## Mental model

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

If you've used Tauri or Electron, this is the same idea — webview
front-end, native back-end — but the front-end is *generated* from
typed Amalgame code instead of hand-written HTML.
