# 02 — Widget catalogue

Every `Element.<Name>(...)` static builder, grouped by purpose.
WinForms-toolbox cross-reference lives at
[`../winforms-mapping.md`](../winforms-mapping.md).

Convention: every builder returns an `Element` so the fluent
chain composes — `.AddChild(...)`, `.Attr(...)`, `.Style(...)`,
`.On<Event>(...)` all work on every widget.

## Text & headings

| Builder | HTML | Notes |
|---|---|---|
| `Element.Heading(text)`        | `<h1>` | Big page heading. Use one per page. |
| `Element.Label(text)`          | `<p>`  | Read-only paragraph. |
| `Element.Pre(text)`            | `<pre>`| Monospace output panel. Often paired with `.Id(...)` so `OnResult` can route into it. |
| `Element.Link(text, url)`      | `<a href>` | OS-browser hyperlink. See [LinkLabel notes](#linklabel) below. |

```amalgame
Element.Stack()
    .AddChild(Element.Heading("Hello"))
    .AddChild(Element.Label("Welcome to the demo."))
    .AddChild(Element.Pre("(submit to see the payload)").Id("out"))
```

### LinkLabel

`Element.Link(text, url)` emits an `<a href>` whose `onclick`
attribute routes through the OS browser (xdg-open / open /
ShellExecute). The user never lands inside the webview.

To navigate inside the same window instead, build a plain
`new Element("a")` and attach your own `.OnClick(h)`.

## Buttons

| Builder | HTML | Notes |
|---|---|---|
| `Element.Button(caption)`      | `<button>` | Wire via `.OnClick(handler)`. |

```amalgame
Element.Button("Save")
    .OnClick((req: string) => req)        // echo the form
    .OnResult("out")                       // dump into #out
```

`Enabled(false)` produces a dimmed disabled button. `Tooltip(t)`
adds a native OS tooltip via the HTML `title` attribute.

## Single-line inputs

| Builder | HTML | WinForms |
|---|---|---|
| `Element.Input(name)`                    | `<input type=text>`   | TextBox |
| `Element.Password(name)`                 | `<input type=password>` | TextBox (masked) |
| `Element.Number(name, min, max, step)`   | `<input type=number>` | NumericUpDown |
| `Element.Slider(name, min, max, step)`   | `<input type=range>`  | TrackBar |
| `Element.DatePicker(name)`               | `<input type=date>`   | DateTimePicker (date) |
| `Element.TimePicker(name)`               | `<input type=time>`   | DateTimePicker (time) |
| `Element.ColorPicker(name)`              | `<input type=color>`  | ColorDialog inline |
| `Element.MaskedTextBox(name, pattern, inputmode)` | `<input pattern=… inputmode=…>` | MaskedTextBox |

Every `name` is auto-collected by the form-state bridge — handlers
receive a JSON object whose keys are these names. See
[`03-events-and-state.md`](03-events-and-state.md#form-payload).

```amalgame
Element.Row()
    .AddChild(Element.Label("Quantity:").Size(120, 0))
    .AddChild(Element.Number("qty", 1, 100, 1))
```

`Number`'s min/max default to 0 — pass `(name, 0, 0, 0)` to leave
all three attributes unset.

## Multi-line text

| Builder | HTML |
|---|---|
| `Element.Textarea(name)` | `<textarea>` |

```amalgame
Element.Textarea("message")
    .Attr("placeholder", "Type a message…")
    .Size(0, 80)        // height in px; 0 = auto width
```

`textarea` is intentionally not resizable by default — the
drag-grip destabilizes declarative layouts. Re-enable via
`.Style("resize:vertical")` if you need it.

## Boolean / choice widgets

| Builder | HTML | WinForms |
|---|---|---|
| `Element.CheckBox(name)`               | `<input type=checkbox>` | CheckBox (bare) |
| `Element.CheckBoxLabel(name, caption)` | `<label><input type=checkbox> caption</label>` | CheckBox (.Text) |
| `Element.Radio(name, value)`           | `<input type=radio>` | RadioButton (bare) |
| `Element.RadioLabel(name, value, caption)` | `<label><input type=radio> caption</label>` | RadioButton (.Text) |

Use the `*Label` variant whenever you want the caption clickable
— matches WinForms `Text` property and what users expect.

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

Radio buttons in the same `name` group are mutually exclusive.

## Lists

| Builder | HTML | WinForms |
|---|---|---|
| `Element.Select(name)` + `Element.Option(value, label)` | `<select><option>` | ComboBox |
| `Element.ListBox(name, size)`                            | `<select multiple>` | ListBox |
| `Element.CheckedListBox(name)` + `Element.CheckedItem(name, value, label)` | `<ul>` of `<input type=checkbox>` | CheckedListBox |
| `Element.ListView(headers, bodyId)` + `Element.ListViewRow(values)` | `<table>` | ListView (details mode) |

```amalgame
let cols: List<string> = ["Name", "Size", "Modified"]
let r1:  List<string> = ["readme.md", "2 KB", "today"]

Element.ListView(cols, "files-body")
    .AddChild(Element.ListViewRow(r1))
```

The `bodyId` parameter is the `<tbody>` id — pass it when you'll
use `Page.AppendInner(win, bodyId, …)` to grow the table at
runtime (see [`03-events-and-state.md`](03-events-and-state.md#partial-dom-updates)).
Pass `""` to skip the id.

## Display widgets

| Builder | HTML |
|---|---|
| `Element.ProgressBar(value, max)` | `<progress>` — pass `value < 0` for an indeterminate bar. |
| `Element.Image(src)`              | `<img>` — accepts `file://`, `https://`, `data:image/...`. |
| `Element.PictureBox(src)`         | Alias of `Image` — matches the WinForms toolbox name. |
| `Element.Iframe(url)`             | `<iframe>` — embed a webpage inside your app. |

```amalgame
Element.Row()
    .AddChild(Element.Label("Loading:").Size(80, 0))
    .AddChild(Element.ProgressBar(42, 100))
```

## Containers & layout

These don't carry data, they organize children. Full reference in
[`04-layout-and-theme.md`](04-layout-and-theme.md).

| Builder | What it does |
|---|---|
| `Element.Div()`                          | Generic block container. |
| `Element.Panel()`                        | Alias of `Div` (WinForms name). |
| `Element.Stack()`                        | Flex column with 8 px gap. |
| `Element.Row()`                          | Flex row with 8 px gap. |
| `Element.Flow(direction)`                | Flex with wrap — `"row"` or `"column"`. |
| `Element.Grid(rows, cols, gap)`          | CSS Grid. Pass `rows=0` for implicit row count. |
| `Element.AbsoluteContainer()`            | `position:relative` parent for `.Position(x, y)` children. |
| `Element.GroupBox(title)`                | `<fieldset><legend>` — captioned section. |
| `Element.TabControl(group)` + `Element.Tab(group, id, label, body)` | Pure-CSS tabs (radio + sibling selector). |
| `Element.ToolStrip()`                    | Themed horizontal button row. |
| `Element.StatusStrip()`                  | Fixed-bottom status bar. |

```amalgame
Element.GroupBox("Personal info")
    .AddChild(Element.Input("name").Attr("placeholder", "Name"))
    .AddChild(Element.Input("email").Attr("placeholder", "Email"))
```

## Escape hatches

When no builder fits:

```amalgame
// 1. Use a raw HTML tag — same fluent API as the builders.
new Element("aside")
    .Class("my-side-panel")
    .AddChild(Element.Label("Custom widget"))

// 2. Inject raw HTML inside an element.
Element.Div().Id("plot").Raw("<svg width=200 height=200>...</svg>")
```

Both forms produce regular HTML; the auto-collect bridge still
works for any `[name]` input inside.

For deeper escape hatches — JS libraries, custom C primitives,
themed CSS overrides — see [`05-extending.md`](05-extending.md).

## Property setters (on every Element)

Mirror the WinForms designer Properties pane. Each returns `this`
so they chain.

| Setter | What it does |
|---|---|
| `.SetText(t)`           | Inner text content. |
| `.Id(id)`               | HTML id. Required when targeted by `OnResult` / `AppendInner`. |
| `.Class(cls)`           | CSS class. Multiple classes allowed (space-separated). |
| `.Style(css)`           | Append inline CSS. Cumulative — multiple calls layer. |
| `.Attr(name, value)`    | Set any HTML attribute (escape hatch). |
| `.Size(w, h)`           | Pixel size; 0 on either axis leaves it unset. |
| `.Position(x, y)`       | Absolute placement (use inside `AbsoluteContainer`). |
| `.Visible(b)`           | False → `display:none`. |
| `.Enabled(b)`           | False → disabled + dimmed. |
| `.Tooltip(t)`           | Native OS tooltip (`title` attribute). |
| `.TabIndex(n)`          | Keyboard focus order. |
| `.ForeColor(css)`       | Shorthand for `color:` (accepts `#hex`, `var(--amc-...)`, named). |
| `.BackColor(css)`       | Shorthand for `background:`. |
| `.Font(family, sizePx)` | Shorthand for `font-family` + `font-size`. |
| `.DataTag(payload)`     | Round-trip arbitrary string via `data-tag` attribute. |
| `.Fill()`               | Mark this child as the flex-grow / scroll target inside its parent. |
| `.Bind(name)`           | Declarative form-key (same as `.Attr("name", name)`). |
| `.Raw(html)`            | Append a raw HTML chunk as a child. |

For events (`.OnClick`, `.OnChange`, `.OnFocus`, …) see
[`03-events-and-state.md`](03-events-and-state.md).
