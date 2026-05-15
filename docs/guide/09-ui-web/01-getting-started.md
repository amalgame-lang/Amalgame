# 01 — Getting started

## Prerequisites

- **`amc` 0.8.13 or later.** v0.0.5+ of ui-web uses chained method
  calls across packages — older amc binaries trip type inference.
- **An OS webview engine** matching the build target:

  | OS                 | Engine     | Install                                      |
  |--------------------|------------|----------------------------------------------|
  | Debian / Ubuntu    | WebKitGTK  | `sudo apt install libwebkit2gtk-4.1-dev pkg-config` |
  | Fedora             | WebKitGTK  | `sudo dnf install webkit2gtk4.1-devel pkgconf` |
  | Arch               | WebKitGTK  | `sudo pacman -S webkit2gtk-4.1 pkgconf`      |
  | macOS              | WKWebView  | ships with the OS since 10.10                |
  | Windows 11         | WebView2   | part of the OS                               |
  | Windows 10 ≥ 1803  | WebView2   | Microsoft auto-installs the evergreen runtime|
  | Windows 10 < 1803  | WebView2   | bundle the bootstrapper (see main README)    |

## Install

From the project where you'll write the app:

```sh
amc package add ui-web
```

This fetches `amalgame-ui-web` into `~/.amalgame/packages/`, pins
the version in `amalgame.lock`, and lets `amc` wire the `import
Amalgame.UI.Web` statement.

If you prefer scaffolding, `amc new --template ui-web-form` lays
out a complete project (`amalgame.toml`, `src/main.am`,
`build.sh`, `.gitignore`) — see the main README for the flags.

## A minimal window

`src/hello.am`:

```amalgame
import Amalgame.UI.Web

class Program {
    public static void Main() {
        let win: Window = new Window("Hello", 480, 320, false)
        if (!win.IsValid()) {
            Console.WriteError("ui-web: failed to create webview slot")
            return
        }

        Page.New()
            .SetTitle("Hello, Amalgame!")
            .SetBody(
                Element.Stack()
                    .AddChild(Element.Heading("Hello, Amalgame"))
                    .AddChild(Element.Label("Welcome to ui-web v0.0.5."))
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

The `Window` constructor takes `(title, width, height, debug)`.
`debug=true` exposes DevTools (`Ctrl+Shift+I`); leave it `false`
in shipped builds.

## Build script

A typical `build.sh` on Linux (mirrors what `amc new` generates):

```sh
#!/bin/sh
set -eu

AMC="${AMC:-amc}"
APP_NAME="hello"
PKG_DIR="${AMALGAME_PACKAGES_DIR:-$HOME/.amalgame/packages}"
UIWEB="$PKG_DIR/amalgame-ui-web"

# 1. Compile the webview C++ implementation once.
test -f "$UIWEB/runtime/vendor/webview/webview.o" || \
    g++ -c -O2 -DWEBVIEW_GTK \
        "$UIWEB/runtime/vendor/webview/webview.cc" \
        -o "$UIWEB/runtime/vendor/webview/webview.o"

# 2. Compile the C glue layer.
test -f "$UIWEB/runtime/Amalgame_UI_Web.o" || \
    gcc -c -O2 -I"$UIWEB/runtime" \
        "$UIWEB/runtime/Amalgame_UI_Web.c" \
        -o "$UIWEB/runtime/Amalgame_UI_Web.o"

# 3. Compile facade.am into a static lib once per ui-web version.
(cd "$UIWEB" && test -f facade.o || ("$AMC" --lib facade.am --quiet && \
    gcc -c -O2 -Iruntime a.out.c -o facade.o))

# 4. Compile our app + link.
"$AMC" -o "$APP_NAME" "src/main.am" --external "$UIWEB/facade.am" --quiet
gcc -O2 \
    -I"$UIWEB/runtime" -I"$UIWEB/runtime/vendor/webview" \
    "${APP_NAME}.c" \
    "$UIWEB/facade.o" \
    "$UIWEB/runtime/Amalgame_UI_Web.o" \
    "$UIWEB/runtime/vendor/webview/webview.o" \
    $(pkg-config --libs webkit2gtk-4.1) \
    -lstdc++ -lgc -lm -lcurl -lz \
    -o "$APP_NAME"
```

`amc new --template ui-web-form` produces a turnkey version of
this with macOS / Windows paths. Existing apps can copy it
verbatim — only the package path lookup changes between OSes.

Run:

```sh
./build.sh && ./hello
```

You should see a 480×320 window with the heading, a button, and an
empty `<pre>` panel. Clicking the button prints `"clicked"` (with
quotes — that's the JSON encoding) into the panel.

## What's happening

- `new Window(...)` allocates a webview slot in the runtime's C
  side. Up to 4 windows per process.
- `Page.New().SetBody(...)` builds an `Element` tree in memory.
  Nothing is rendered yet.
- `Page.ApplyTo(win)` walks the tree, generates the HTML, injects
  the auto-collect + result-routing JS bridges via `Window.Init`,
  loads the HTML via `Window.SetHtml`, and registers each
  `.OnClick` / `.OnChange` handler via `Window.Bind`.
- `Window.Run()` enters the webview's event loop and blocks until
  the window is closed or `Window.Terminate()` is called.

You write Amalgame for the entire app. No hand-written HTML. No
JS unless you reach for it deliberately (see
[`05-extending.md`](05-extending.md)).

## Where to look next

- The [widget catalogue](02-widgets.md) shows every builder with
  the WinForms toolbox name + a snippet.
- The [event model](03-events-and-state.md) explains how form
  state flows, how `OnResult` routes return values, and how to
  patch rows without re-rendering everything.
- The [layout reference](04-layout-and-theme.md) covers
  `Stack` / `Row` / `Grid` / `Flow`, the `Page.FillViewport`
  app-shell mode, and how to override the seven CSS variables.

## Common pitfalls

- **Window.IsValid()** can fail if `AMALGAME_UI_WEB_MAX_WINDOWS`
  (4) slots are already taken. Destroy windows explicitly when
  you're done with them.
- **DevTools needs `debug=true`** at `Window` construction time —
  the 4th argument. Toggling later isn't supported by the
  underlying webview library.
- **Long fluent chains hang amc's type inference**: chains of
  ~24+ `.AddChild(...)` calls go non-linear. Split into named
  intermediates (`let header: Element = …`) — see
  [`04-layout-and-theme.md`](04-layout-and-theme.md#fluent-chain-limits).
