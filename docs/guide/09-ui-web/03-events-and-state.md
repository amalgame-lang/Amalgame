# 03 — Events & state

How user actions in the rendered page reach back into your
Amalgame code, how form state flows, and how to refresh parts of
the DOM without re-rendering the whole page.

## The event model

Every DOM event is routed through a single uniform mechanism:

```amalgame
Element.<widget>(...).On(eventName, handler)
```

`On("click", h)`, `On("change", h)`, `On("wheel", h)`, custom
events like `On("my-app-event", h)` — all use the same wiring.

For the common WinForms-aligned events, sugar setters exist:

| Sugar setter                | DOM event                  | WinForms equivalent             |
|-----------------------------|----------------------------|---------------------------------|
| `.OnClick(h)`               | `click`                    | `Click`                         |
| `.OnDblClick(h)`            | `dblclick`                 | `DoubleClick`                   |
| `.OnChange(h)`              | `change` + `input` (for text inputs / textareas) | `TextChanged` / `CheckedChanged` / `SelectedIndexChanged` |
| `.OnFocus(h)`               | `focus`                    | `GotFocus` / `Enter`            |
| `.OnBlur(h)`                | `blur`                     | `LostFocus` / `Leave`           |
| `.OnMouseEnter(h)`          | `mouseenter`               | `MouseEnter`                    |
| `.OnMouseLeave(h)`          | `mouseleave`               | `MouseLeave`                    |
| `.OnKeyDown(h)`             | `keydown`                  | `KeyDown`                       |
| `.OnKeyUp(h)`               | `keyup`                    | `KeyUp`                         |

A single Element can carry multiple events independently:

```amalgame
Element.Input("user")
    .OnFocus((req: string) => req)
    .OnBlur((req: string) => req)
    .OnChange((req: string) => req)
```

## Handler signature

Every handler is a closure with this shape:

```amalgame
(req: string) => string
```

- `req` is a JSON-encoded snapshot of every named form field on
  the page at the moment the event fires.
- The return value must be valid JSON — either `"true"`, a
  string like `"\"ok\""`, a number `"42"`, or a richer object.

The simplest handler echoes the form back:

```amalgame
Element.Button("Submit").OnClick((req: string) => req)
```

For a free-form text return, wrap with `Json.EncodeString`:

```amalgame
Element.Button("Greet").OnClick((req: string) => Json.EncodeString("hello"))
```

## Form payload

Every element with a `name` attribute (built via the `Input`,
`Password`, `Select`, etc. builders, or by `.Bind(name)` on a raw
element) is auto-collected by the bridge `window.__amc_collect`
injected by `Page.ApplyTo`.

Example handler `req`:

```json
{
  "user": "alice",
  "message": "Hi!",
  "newsletter": true,
  "priority": "normal",
  "theme": "dark"
}
```

- Checkboxes → boolean.
- Radios → the value of the checked one in the group; key absent
  when none is checked.
- Multi-select listboxes → not auto-collected in v0.0.5 — read
  `.selectedOptions` via `Window.Eval` until v0.0.6.
- Other inputs → their `.value` (always a string in HTML, even
  for `type=number` / `type=date`).

The same payload reaches **every** event on every Element — you
don't have to wire each input into a specific handler. Read just
the fields you care about.

## OnResult — routing the return value

Without `OnResult`, the handler's return is discarded. With it,
the bridge writes the JSON-pretty-printed result into a target
element by id:

```amalgame
Element.Stack()
    .AddChild(Element.Button("Submit")
        .OnClick((req: string) => req)
        .OnResult("out"))            // ← writes the handler's
                                     //   return into #out
    .AddChild(Element.Pre("").Id("out"))
```

- The router calls `JSON.stringify(JSON.parse(r), null, 2)` first
  — valid JSON renders pretty. Plain strings fall through
  verbatim.
- `OnResult(targetId)` is sticky across event setters: any `On*`
  call that comes BEFORE it captures the route. Any `On*` AFTER
  doesn't, unless you set it again.
- Pass `""` to reset.

To handle a plain string and avoid the JSON pretty-print, return
it unquoted from the handler — `Json.EncodeString` and the
router co-operate so a string return prints as quoted text in the
`<pre>`.

## Bind name collisions

If two Elements have the same `id`, `OnResult` only updates the
first match (`document.getElementById`). Same for `Page.PatchInner` /
`AppendInner` targets — keep ids unique.

Auto-collect treats `name` values like form keys: two `<input
name="x">` widgets clobber each other in the payload. Use
radio-group sharing intentionally, but otherwise pick distinct
names.

## Partial DOM updates

Re-rendering the whole page on every state change is wasteful and
loses focus, scroll position, etc. Use the patching API for
incremental changes.

### Raw DOM ops (`Window.*`)

| Call | What it does |
|---|---|
| `Window.SetInnerHtml(id, html)` | Replace `#id`'s inner HTML. |
| `Window.AppendHtml(id, html)`   | Append HTML as the last child of `#id`. |
| `Window.RemoveElement(id)`      | Drop the element with the given id. |

```amalgame
win.SetInnerHtml("status", "<span>Saved.</span>")
```

These are escape hatches — no event bindings are wired
automatically, so anything with an `onclick` won't have an AM
handler unless you `Window.Bind` it separately.

### Element-typed patching (`Page.*`)

The typed wrappers render an `Element` subtree, wire any new
`OnClick` / `OnChange` it contains, and inject the result via
the raw ops above:

| Call | What it does |
|---|---|
| `Page.PatchInner(win, id, element)`  | Render `element` and replace `#id`'s children. |
| `Page.AppendInner(win, id, element)` | Render `element` and append to `#id`'s children. |

```amalgame
let row: Element = Element.ListViewRow(["new.txt", "0 KB", "today"])
page.AppendInner(win, "files-body", row)
```

Events in the new subtree get freshly-allocated `_amc_N` bind
names and are auto-registered. The `Page.Counter` keeps growing
across patches — names stay unique.

### Pattern: DataGrid live update

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

Each click on Refresh appends a new row to the table without
re-rendering the heading, the button, or the existing rows.

## Custom JS events

Anything the browser fires can be bound:

```amalgame
Element.Div().Id("drop-zone")
    .On("drop",      (req: string) => req)
    .On("dragover",  (req: string) => req)
    .On("dragleave", (req: string) => req)
```

The JSON payload is still the form snapshot — to read the event
itself (e.g. drag coordinates, dropped files), inject a small JS
shim via `Window.Eval` that captures the relevant event data into
a hidden input, and read that hidden input on the next bound
call.

## `data-amc-internal` opt-out

`Element.Link(text, url)` and the global `<a>` interceptor route
http(s) URLs to the OS browser. If you want a specific anchor to
stay inside the webview (in-app navigation), mark it:

```amalgame
new Element("a")
    .Attr("href", "#page2")
    .Attr("data-amc-internal", "true")
    .SetText("Section 2")
```

The default browser routing then ignores it.

## What `Window.Bind` actually does

The plumbing under the hood. `Window.Bind(name, closure)` does
two things at the webview level:

1. Registers `name` in the internal binding table (capacity:
   `AMALGAME_UI_WEB_MAX_BINDINGS = 64`).
2. Injects a JS shim: every time `window.<name>(args...)` is
   called from the page, the call is queued, the C trampoline
   wakes the AM closure, the closure runs, and its string return
   reaches the JS side as a `Promise` resolution.

That's why every event handler signature is `(req: string) => string`
— bind only knows about JSON strings.
