# 04 — Layout & theme

How children are arranged on the page, how to make the layout
fill the viewport like a desktop app, and how the OS color
scheme drives the visuals.

## Containers cheat-sheet

| Container | Children flow | Use when |
|---|---|---|
| `Element.Stack()`     | Vertical, 8 px gap                | Vertically stacked sections, form fields, page root. |
| `Element.Row()`       | Horizontal, 8 px gap, vertical center | Side-by-side widgets, button bars. |
| `Element.Flow(dir)`   | dir ("row" / "column") with wrap  | Tag cloud, button toolbar that wraps on narrow windows. |
| `Element.Grid(r,c,g)` | CSS Grid — `r × c` cells, gap `g`px | Tabular layout, dashboard cards. Pass `r=0` for implicit rows. |
| `Element.AbsoluteContainer()` | `position:relative` parent | Diagram / Designer-style layouts. Children use `.Position(x, y)`. |
| `Element.Div()`       | Block, no internal layout         | When you need a styled wrapper. |
| `Element.Panel()`     | Alias of `Div`                    | When the WinForms name reads better. |
| `Element.GroupBox(title)` | Block with `<legend>` caption | Visually grouped subsections of a form. |
| `Element.TabControl(g) / Tab(g, id, label, body)` | Tab strip + active panel below | Categorize many widgets without overwhelming the user. |
| `Element.ToolStrip()` | Themed horizontal button row      | Toolbar above content. |
| `Element.StatusStrip()` | Fixed-bottom footer             | Status / version / connection state line. |

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

For pixel-precise placement inside an `AbsoluteContainer`:

```amalgame
Element.AbsoluteContainer()
    .Size(0, 200)        // 200 px tall, fluid width
    .AddChild(Element.Label("X").Position(40, 10).Size(80, 24))
    .AddChild(Element.Label("Y").Position(40, 60).Size(80, 24))
```

`Position` sets `position:absolute;left:Xpx;top:Ypx`. It only
takes effect inside a parent with `position:relative|absolute|fixed`,
which is what `AbsoluteContainer` provides.

## Filling the viewport

By default since v0.0.5, `Page` lays out the body like a desktop
app:

- The body is pinned to `100vh` height.
- Window-level vscroll is disabled.
- The first child of the body grows to take the remaining space.
- Inner widgets handle their own scrolling — TabControl panels,
  `<pre>` output, ListView body all `overflow:auto` when their
  content exceeds the visible region.

For long-form documents that should scroll naturally, opt out:

```amalgame
Page.New().NaturalFlow().SetBody(...)
```

For app-shell behavior (the default), no extra call needed —
just structure your tree so the first child of the body is a
container that organizes the rest:

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

`TabControl` automatically gets `flex:1; min-height:0` from its
baseline class, so it grows to fill its parent. The active tab
panel has `overflow:auto`. `StatusStrip` is `position:fixed` so
it doesn't take layout space — it floats at the viewport bottom.

## `.Fill()` — mark a child as the flex-grow target

When your root layout isn't a TabControl but some other tall
content (a `Stack` of fields, a long ListView), mark the child
that should take the remaining space and scroll:

```amalgame
Element.Stack()
    .AddChild(Element.Heading("Inbox"))             // natural height
    .AddChild(Element.ToolStrip()                   // natural height
        .AddChild(Element.Button("Refresh")))
    .AddChild(longListViewContainer.Fill())         // takes the rest
    .AddChild(Element.StatusStrip()                  // fixed footer
        .AddChild(Element.Label("Connected")))
```

`.Fill()` adds the `amc-fill-child` class, which the baseline
maps to `flex:1; min-height:0; overflow:auto`.

## TabControl mechanics

Pure-CSS — no JavaScript needed for tab switching.

```amalgame
Element.TabControl("settings")
    .AddChild(Element.Tab("settings", "general",  "General",  generalBody)
        .Attr("checked", "checked"))    // ← default active tab
    .AddChild(Element.Tab("settings", "advanced", "Advanced", advancedBody))
    .AddChild(Element.Tab("settings", "about",    "About",    aboutBody))
```

Under the hood: each Tab is a `<label>` wrapping a hidden
`<input type=radio>`, plus the panel body. All radios share the
`name` (the group), so checking one unchecks the rest. CSS
sibling-selector reveals only the `:checked` tab's panel.

- `groupName` must be unique on the page if you have multiple
  TabControls. Reuse across tabs of the same control.
- `id` is appended to the radio's `id` so the `<label for=...>`
  works.
- Up to 32 tabs per TabControl (baseline grid template). Beyond
  that, override the `.amc-tabs` `grid-template-columns` from
  your own stylesheet.

## Theming — the seven CSS variables

The baseline stylesheet exposes seven variables that flip with
the OS theme. Override any of them from your own stylesheet:

| Variable        | Light default | Dark default | Used by |
|-----------------|---------------|--------------|---------|
| `--amc-bg`      | `#fff`        | `#1e1e1e`    | Body bg, input bg |
| `--amc-fg`      | `#1a1a1a`     | `#e8e8e8`    | Body text, control text |
| `--amc-muted`   | `#6a6a6a`     | `#9a9a9a`    | Legend text, status strip |
| `--amc-border`  | `#d0d0d0`     | `#404040`    | Input borders, table cell borders |
| `--amc-surface` | `#f5f5f5`     | `#2a2a2a`    | Button bg, `<pre>` bg, toolbar bg |
| `--amc-accent`  | `#0066cc`     | `#4a9eff`    | Focus rings, slider thumb, active tab border, links |
| `--amc-radius`  | `4px`         | `4px`        | Input + button border-radius |

The OS theme is detected at render time and written to
`<html data-theme="dark|light">`. Override per-page:

```amalgame
Page.New().SetTheme("dark")    // force dark
Page.New().SetTheme("light")   // force light
Page.New().SetTheme("auto")    // default — follow OS
```

You can also override via env var: `AMALGAME_UI_THEME=dark` wins
over OS detection at runtime.

### Customizing variables

```amalgame
Page.New()
    .AddCss("data:text/css,:root{--amc-accent:%23ff6b00;--amc-radius:8px}")
```

(URL-encoded `#` as `%23` because the URL is a `data:` URL.) Or
ship a real stylesheet file:

```amalgame
Page.New()
    .AddCss("file:///abs/path/to/my-theme.css")
```

Three ways to bring CSS:

```amalgame
// 1. Baseline + your overrides (layered on top)
Page.New().AddCss("file:///abs/app/overrides.css")

// 2. Skip the baseline, ship your own complete stylesheet
Page.New().SetStylesheet("file:///abs/app/style.css")

// 3. Skip the baseline, layer multiple sheets
Page.New().NoBaseline()
    .AddCss("https://unpkg.com/@picocss/pico@2/css/pico.min.css")
    .AddCss("file:///abs/app/app.css")
```

## Chrome lockdown

By default the webview's right-click context menu and reload
hotkeys (F5, Ctrl+R / Cmd+R) are blocked. They'd land the user
at `about:blank` because the document is loaded via `SetHtml`
without a backing URL.

Apps that navigate via `Window.Navigate(url)` to a real URL and
want the standard browser chrome opt back in:

```amalgame
Page.New().AllowBrowserDefaults()
```

DevTools (`Ctrl+Shift+I` in debug builds) is never blocked.

## Fluent-chain limits

Amc's type inference goes non-linear above ~24 chained `.AddChild(...)`
calls in a single fluent expression. Workaround: split into named
intermediates.

```amalgame
// AVOID — long chain that may hang amc
Element.Stack()
    .AddChild(...)         // ×30+
    .AddChild(...)
    ...

// PREFER — broken into blocks
let header: Element = Element.Stack()
    .AddChild(...)         // ×8 max per chain
let main:   Element = Element.Stack()
    .AddChild(...)         // ×8 max
let footer: Element = Element.Stack()
    .AddChild(...)         // ×8 max

Element.Stack()
    .AddChild(header)
    .AddChild(main)
    .AddChild(footer)
```

This is an amc bug (memoization missing on `InferTypeFromExpr`)
that will be fixed; the workaround is harmless and reads better
anyway.
