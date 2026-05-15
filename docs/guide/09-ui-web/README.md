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
- A WinForms-aligned widget builder (`Element.Button`, `Element.Input`, …).
- Form-state auto-collection — every named input round-trips through
  a JSON object delivered to your handlers.
- A baseline stylesheet themed via 7 CSS variables that flip with
  the OS color scheme.
- Partial-DOM update primitives so DataGrid-style apps can refresh
  rows without re-rendering the page.

**Isn't:**
- A retained-mode toolkit (use `Element` to build trees, not
  `Button.Visible = false` mutations).
- A full WPF / Qt replacement — native menubar, system tray,
  DataGridView and OS dialogs are planned for v0.1+ (see
  [`../winforms-mapping.md`](../winforms-mapping.md)).
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
