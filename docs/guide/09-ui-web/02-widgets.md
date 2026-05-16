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
| `Element.Number(name, min, max, step)`   | composite `<input type=number>` + ▲/▼ | NumericUpDown |
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

Since v0.0.5 `Number` is a composite (the `<input type=number>`
plus a vertical `▲` / `▼` button column). WebKitGTK doesn't paint
the native `::-webkit-inner-spin-button`, so the buttons are
always visible and behave identically on the three OS engines.

## Multi-line text

| Builder | HTML | WinForms |
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

`textarea` is intentionally not resizable by default — the
drag-grip destabilizes declarative layouts. Re-enable via
`.Style("resize:vertical")` if you need it.

`RichTextBox` (v0.0.9) accepts inline formatting via the built-in
browser shortcuts (`Ctrl-B` / `Ctrl-I` / `Ctrl-U`) and pasted rich
content. The form payload reports the rich content as the inner
HTML under the widget's `name` key.

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

## Tree (v0.0.7)

| Builder | What it does | WinForms |
|---|---|---|
| `Element.TreeView()`               | Root container.                              | TreeView |
| `Element.TreeNode(caption)`        | Expandable folder-like node (HTML5 `<details>`). | TreeNode |
| `Element.TreeLeaf(caption)`        | Terminal item.                               | TreeNode (leaf) |

```amalgame
Element.TreeView()
    .AddChild(Element.TreeNode("src")
        .AddChild(Element.TreeNode("parser")
            .AddChild(Element.TreeLeaf("ast.am"))
            .AddChild(Element.TreeLeaf("parser.am")))
        .AddChild(Element.TreeLeaf("main.am")))
    .AddChild(Element.TreeLeaf("README.md"))
```

Built on `<details>` / `<summary>` so expand/collapse + keyboard
focus work without any JS. Pass `.Attr("open", "open")` on a
`TreeNode` to render it expanded by default. The baseline themes
the rotating ▶ caret, hover background, and 18 px nested indent
through the `--amc-*` variables.

## Display widgets

| Builder | HTML | WinForms |
|---|---|---|
| `Element.ProgressBar(value, max)` | `<progress>`               | ProgressBar (`value < 0` = indeterminate) |
| `Element.Image(src)`              | `<img>`                    | PictureBox / Image |
| `Element.PictureBox(src)`         | `<img>` (alias)            | PictureBox |
| `Element.Iframe(url)`             | `<iframe>`                 | WebBrowser |
| `Element.MonthCalendar(name, year, month)` | grid (`<table>`) + ◀ / month / year / ▶ header | MonthCalendar (v0.0.9 + v0.0.10 navigator) |

```amalgame
Element.Row()
    .AddChild(Element.Label("Loading:").Size(80, 0))
    .AddChild(Element.ProgressBar(42, 100))

Element.MonthCalendar("birthday", 2026, 5)
```

`MonthCalendar` (v0.0.9) renders an inline month grid. The header
(v0.0.10) carries ◀ / month dropdown / year dropdown / ▶ so the
user can jump to any month or year directly. Clicking a day
highlights it and reports the selection as an ISO `YYYY-MM-DD`
string under the widget's `name` key in the form payload (the
key is absent when no day is selected).

## Menus (v0.0.8)

The cross-OS, HTML-rendered MenuBar — same shape as WinForms'
`MenuStrip`, themed with `--amc-*`. A native OS variant (Win32
`HMENU` / NSMenu / GtkMenuBar) is planned as opt-in via
`data-mode="native"` in v0.1.0; today the API is forward-compatible.

| Builder | What it does | WinForms |
|---|---|---|
| `Element.MenuBar()`                           | `<nav>` container — top-of-window menu strip. | MenuStrip |
| `Element.MenuItem(label)`                     | Top-level item with a dropdown panel — internally a `<details>` whose `<summary>` is the label. | ToolStripMenuItem |
| `Element.MenuOption(label, actionName)`       | `<button>` inside a dropdown that calls `window.<actionName>('')` on click. | ToolStripMenuItem (leaf) |
| `Element.MenuSeparator()`                     | Themed `<hr>` between options. | ToolStripSeparator |
| `Element.ContextMenu(targetId)`               | Right-click menu — reuses MenuOption / MenuSeparator. | ContextMenuStrip |

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

Bind every action name via `win.Bind("amc_new", handler)`. A
global `click` listener closes any open menu when the user
clicks outside it; `Escape` closes both menubar dropdowns and
the context menu.

`ContextMenu` attaches to the host element whose id you pass
and listens for the `contextmenu` DOM event:

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
| `Element.SplitContainer(orientation, ratio)` | Two-pane resizable container (v0.0.8). |
| `Element.ToolStrip()`                    | Themed horizontal button row. |
| `Element.StatusStrip()`                  | Fixed-bottom status bar. |

```amalgame
Element.GroupBox("Personal info")
    .AddChild(Element.Input("name").Attr("placeholder", "Name"))
    .AddChild(Element.Input("email").Attr("placeholder", "Email"))
```

`SplitContainer` (v0.0.8) takes an orientation (`"row"` →
left/right with a vertical divider, `"column"` → top/bottom)
and an initial ratio percentage 5..95 (e.g. `30` for a 30/70
split). The divider is draggable with the mouse / pen / touch:

```amalgame
Element.SplitContainer("row", 30)
    .AddChild(Element.Stack().AddChild( … left pane … ))
    .AddChild(Element.Stack().AddChild( … right pane … ))
```

## Dialogs (v0.0.6 → v0.0.8)

Modal message boxes and file pickers. All entry points are
static methods on `Dialog`, not Element builders — they don't
go in the page tree; they pop on demand.

### Message boxes (v0.0.6)

| Call | Buttons | WinForms equivalent |
|---|---|---|
| `Dialog.Info(win, title, message, handler)`         | OK         | `MessageBox.Show(... Information)` |
| `Dialog.Warning(win, title, message, handler)`      | OK         | `MessageBox.Show(... Warning)` |
| `Dialog.Error(win, title, message, handler)`        | OK         | `MessageBox.Show(... Error)` |
| `Dialog.Confirm(win, title, message, handler)`      | OK/Cancel  | `MessageBox.Show(... OKCancel)` |
| `Dialog.YesNoCancel(win, title, message, handler)`  | Yes/No/Cancel | `MessageBox.Show(... YesNoCancel)` |
| `Dialog.Show(win, kind, title, message, buttons, handler)` | mix & match | low-level |

The handler receives the clicked button id as `req` —
`"ok"` / `"cancel"` / `"yes"` / `"no"` — or `"cancel"` when the
user dismisses with Esc / backdrop click.

```amalgame
Dialog.Confirm(win, "Quit?", "Discard unsaved changes?",
    (req: string) => {
        if (req == "ok") { win.Terminate() }
        return ""
    })
```

Implementation uses the HTML `<dialog>` element — focus trap,
Esc-dismissal and backdrop scrim come from the browser for free.
The header carries a colored accent (blue / orange / red) based on
the kind.

### File pickers (v0.0.7 + v0.0.8)

| Call | Returns to handler | Notes |
|---|---|---|
| `Dialog.OpenFile(win, accept, handler)`                       | filename string (or `""` on cancel) | Browser sandbox: filename only, no path. |
| `Dialog.OpenFileContent(win, accept, binary, handler)` (v0.0.8) | JSON `{"name":"…","content":"…"}` (text or base64) | `binary=true` → `readAsDataURL` then strip prefix. |
| `Dialog.SaveFile(win, filename, content, mimeType, handler)`  | `"ok"` once download is initiated | Sandbox: no way to know if the user accepted Save-As. |

```amalgame
Dialog.OpenFileContent(win, ".txt,.json", false,
    (payload: string) => {
        // payload = ""  on cancel
        // payload = {"name":"notes.txt","content":"Hello, file…"} on success
        return ""
    })
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
| `.Attr(name, value)`    | Set any HTML attribute (escape hatch). Replaces on duplicate key (since v0.0.8). |
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
