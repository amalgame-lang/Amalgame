# 05 — Extending ui-web

The three escape hatches: raw HTML, JS libraries, and OS-native
primitives.

## Raw HTML

`Element.Raw(html)` appends a verbatim HTML chunk as a child of
any element. Useful for shipping markup you already have or for
specialized snippets the builders don't cover:

```amalgame
Element.Div().Id("logo-host").Raw("<svg width='80' height='80' viewBox='0 0 100 100'>"
    + "<circle cx='50' cy='50' r='40' fill='#4a9eff' stroke='#1a1a1a' stroke-width='2'/>"
    + "<text x='50' y='58' text-anchor='middle' fill='white' font-size='32'>A</text>"
    + "</svg>")
```

The dev is responsible for escaping. `Page.RenderElement` emits
the string as-is, so HTML special characters (`<`, `&`, `"`) must
be valid markup. For safe interpolation of user-provided strings,
use `Html.Escape(...)` before splicing.

## Custom Element tags

Any HTML tag (custom elements included) is reachable via
`new Element(tagName)`:

```amalgame
// HTML5 <details> / <summary>
new Element("details")
    .AddChild(new Element("summary").SetText("More info"))
    .AddChild(Element.Label("This expands when the user clicks."))

// Custom-element name (web components)
new Element("my-spinner").Attr("size", "32")
```

The same fluent API (`.Attr`, `.Style`, `.On(event, h)`,
`.AddChild`) works on every tag. The baseline stylesheet won't
theme unknown tags — supply your own CSS via `Page.AddCss` if
you want them styled.

## Embedding a JS library

Webview engines are full browsers, so any front-end library that
ships as ESM / UMD / `<script>` works. Three patterns:

### 1. Include from a CDN or local file

```amalgame
Page.New()
    .AddCss("https://unpkg.com/tabulator-tables@5/dist/css/tabulator.min.css")
    .SetBody(Element.Stack()
        .AddChild(Element.Div().Id("table-host")))
    .ApplyTo(win)

// Mount the lib after the page is loaded.
win.Eval("(function(){var s=document.createElement('script');s.src='https://unpkg.com/tabulator-tables@5/dist/js/tabulator.min.js';s.onload=function(){new Tabulator('#table-host',{data:[{n:1},{n:2}],columns:[{title:'N',field:'n'}]});};document.head.appendChild(s);})();")
```

### 2. Bind a JS callback back into AM

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

The pattern is symmetrical: AM-rendered Elements emit
`onclick="window._amc_N(...)"`, you can equally `win.Bind` any
name and call it from your custom JS.

### 3. Drop in a mount-point that the lib populates

```amalgame
Element.Div().Id("plot").Style("height:300px")
```

After `ApplyTo`, evaluate the lib's setup:

```amalgame
win.Eval("Plotly.newPlot('plot', [{x:[1,2,3],y:[2,4,1],type:'bar'}], {});")
```

Use `Page.AppendInner` / `Window.SetInnerHtml` if you need to
swap the mount-point's contents later — but be aware most JS
libs allocate state outside the DOM and will leak unless you
call their `destroy()` method first.

## C-side primitives

When you need an OS-native capability the webview doesn't
expose (system tray, native menus, file dialogs beyond
`<input type=file>`, registry / keychain access), add a C
primitive to `runtime/Amalgame_UI_Web.c` and expose it through
the facade.

The pattern from v0.0.5 itself — `Amalgame_UI_Web_OpenUrl` — is
the template:

1. **Header declaration** in `runtime/Amalgame_UI_Web.h`:

    ```c
    int Amalgame_UI_Web_OpenUrl(const char* url);
    ```

2. **Implementation** in `runtime/Amalgame_UI_Web.c` with
   platform `#ifdef`s:

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

3. **Facade wrapper** in `facade.am`:

    ```amalgame
    public static bool OpenExternalUrl(url: string) {
        var rc: int = 1
        @c {
            rc = (i64)Amalgame_UI_Web_OpenUrl(url);
        }
        return rc == 0
    }
    ```

4. **Auto-injection** in `Page.ApplyTo` if the primitive should
   be auto-bound for every page (`_amc_openurl` is bound here so
   `Element.Link` works without app code).

The webview library itself exposes the native window handle via
`webview_get_window(w)` — HWND on Windows, NSWindow* on macOS,
GtkWindow* on Linux. Cast and call native APIs from there for
deeper integration.

## Custom widgets — the Component pattern

When a tree of Elements is reused across screens, promote it to
a regular Amalgame class with a `Render()` method:

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

// usage
let form: Element = Element.Stack()
form.AddChild(new LabeledInput("Name:",  "name").Render())
form.AddChild(new LabeledInput("Email:", "email").Render())
```

Ship reusable widgets as an Amalgame package — `amc package add
my-team-widgets` pulls them in, and consumers compose them via
`Render()` like any other Element source. No engine support
needed; this is pure language reuse.

A formal `abstract class Component { Element Render() }`
convention is planned for v0.0.6 (see
[`../architecture.md`](../architecture.md)).

## Replacing the baseline stylesheet

For radical re-skinning (using Pico, Tailwind, Bulma, or a
hand-authored design system), skip the baseline:

```amalgame
Page.New()
    .NoBaseline()
    .AddCss("https://unpkg.com/@picocss/pico@2/css/pico.min.css")
    .AddCss("file:///abs/app/app.css")
```

The form-collect / chrome-lockdown / link-routing bridges still
inject. Only the visual styles are replaced. You'll need to
re-style the `.amc-statusstrip`, `.amc-tabs`, `.amc-listview`,
etc. classes yourself if you use those builders.

## Debugging tips

- **`new Window(..., true)`** enables DevTools. Right-click in
  the window or `Ctrl+Shift+I` to open the inspector. Indispensable
  for CSS / JS issues.
- **`Console.WriteLine` from your AM closures** prints to the
  terminal that launched the binary. The simplest way to trace
  what's reaching a handler.
- **The internal bridges log nothing by default.** When debugging
  a wiring issue, instrument `Page.ApplyTo`'s `_amc_openurl`
  closure or your own `Window.Bind` callbacks with
  `Console.WriteLine` to see what `req` value actually arrives.
- **`./build/dump_html`-style scripts** — wrap a Page in a
  one-shot binary that calls `Page.Render()` and prints the
  resulting HTML to stdout. Faster than spinning up the webview
  to verify the markup. See `tests/dump_html.am` in this package.

## What's intentionally NOT in ui-web

These belong elsewhere; reach for them via the regular Amalgame
ecosystem:

- **HTTP server / client** — `Amalgame.Net` stdlib.
- **Filesystem watching** — `amalgame-io-filewatcher` package.
- **Background processes** — `Process.Run` stdlib, or the
  `Service` package for long-running daemons.
- **Database** — `amalgame-database-sqlite` package.
- **Logging** — `amalgame-logging` package.

`ui-web` is the front-end binding; combine it with the rest of
the ecosystem to build full apps.
