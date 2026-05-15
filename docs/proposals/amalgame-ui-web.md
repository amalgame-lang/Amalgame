# Proposal: `Amalgame.UI.Web` — Webview-based UI toolkit strategy

**Status:** v0.0.3 shipped 2026-05-15 — package at
[`amalgame-lang/amalgame-ui-web`](https://github.com/amalgame-lang/amalgame-ui-web).
Supersedes the SDL-based `ui-forms` and the never-published Tk-based
`amalgame-ui-tk` exploration. Both sunsets carry a banner pointing
here. Requires amc ≥ 0.8.12 (typed-param lambdas + cross-package
chains + Closure type).

**Author:** v0.8.x cycle, post `ui-forms` cgen bug-bash.

**Tracking PRs:**
- amc v0.8.11 — typed-param lambdas + Closure type + cgen brace
  literal (#452, #453).
- amc v0.8.12 — cross-package chained calls + inline lambda as
  argument outside Map/Filter (#456).
- ui-web v0.0.1 (window MVP) → v0.0.2 (Bind IPC) → v0.0.3 (Element /
  Page builder API). PR #1 on the package repo merged the
  cumulative branch to main; tagged `v0.0.3`.

**Shipped milestones**

| Version | Surface | Notes |
|---------|---------|-------|
| v0.0.1  | Window MVP — IsValid / SetTitle / SetSize / Navigate / SetHtml / Init / Eval / Run / Terminate / Destroy + WindowHint | Wraps `webview/webview` v0.12.0 vendored at `runtime/vendor/webview/`. |
| v0.0.2  | + Window.Bind(name, handler: Closure) + Unbind | C trampoline + 64-slot registry. Handler signature `(req: string) => "..."` returning JSON. |
| v0.0.3  | + Element / Page builder + Html.Escape + Json.EncodeString | Fluent chain API, auto-Bind via Page.ApplyTo, layout primitives (Stack / Row / Heading / Label / Button / Input / Pre). |

## Problem

Amalgame needs a UI toolkit for "productive" applications — forms,
dashboards, editors, internal tools. We tried two paths and rejected
both:

1. **SDL-based `ui-forms`** (v0.7.x → v0.8.10). Too time-consuming:
   every widget, every theme, every input behaviour had to be
   reimplemented from scratch. Six known cgen/typechecker bugs
   accumulated specifically around `ui-forms` codegen.
2. **Tk-based `amalgame-ui-tk`** (started 2026-05-15, same day as this
   pivot). Looked promising on paper — native widgets via Tcl/Tk — but
   in practice the menubar implementation is buggy and the result is
   not as "caméléon" (true OS-native look) as advertised. Forest theme
   on Linux feels foreign, Sun Valley on Windows is acceptable, aqua on
   macOS is fine but inconsistent.

The deeper question: **what approach do modern language ecosystems
actually use for desktop GUI?**

## Industry survey (2026)

| Language | Main GUI solution | Approach | Truly native? | Notable apps |
|---|---|---|---|---|
| Rust | egui, iced, Slint, Tauri, GPUI, Floem | Custom render or webview | No | Zed (GPUI), Lapce (Floem), Warp |
| Go | Fyne, Gio, Wails | Custom render or webview | No | syncthing-tray, Wails apps |
| Swift | SwiftUI, AppKit/UIKit | Native Apple | Yes (Apple-only) | Every modern Mac/iOS app |
| Kotlin | Compose Multiplatform, Swing | Custom render (Skia) | No | IntelliJ (Swing) |
| Dart | Flutter | Custom render (Skia/Impeller) | No (simulated Material/Cupertino) | BMW, Toyota infotainment |
| C#/.NET | MAUI, Avalonia, Eto.Forms | Mixed — Eto/MAUI wrap native | Yes for MAUI/Eto | Visual Studio, Pinta |
| Java | Swing, JavaFX, **SWT** | Custom render — *except SWT* | No (Swing/JFX), yes (SWT) | IntelliJ, Eclipse (SWT) |
| Python | Tkinter, PyQt, wxPython, Kivy | Native wrap or Qt | Partial | Calibre, Anki |
| Lua | **IUP** | Native wrap | Yes | Tecgraf engineering tools |
| JS/TS | Electron, **Tauri** | Webview / Chromium | No | VS Code, Slack, Discord, 1Password |

Three takeaways:

- **True "caméléon" native is a vanishing approach** in modern languages.
  Pixel-perfect OS-native rendering across 3 OSes costs more than any
  small team can sustain.
- **The dominant pragmatic choice for productive apps is webview** —
  Electron, Tauri, Wails, plus VS Code, Slack, Discord, 1Password.
- **The dominant choice for games / real-time / creative tools is
  custom render** — wgpu, Skia, SDL, raylib.

These are different problem spaces and there is no toolkit that wins
both. Amalgame should follow the same split.

## Options surveyed for Amalgame

We evaluated five concrete candidates against the constraints:
permissive license (project may be commercialized), C-friendly API
(Amalgame compiles to C), small binary footprint, modern look,
manageable maintenance burden.

| Library | License | API | Look | Stability | Verdict |
|---|---|---|---|---|---|
| **IUP** | MIT | C | Native widgets on each OS | Rock solid (30+ years) | Backup — usable for native-look apps later |
| **libui-ng** | MIT | C | Native widgets | Alpha, single maintainer | Risky long-term |
| **LVGL** | MIT | C | Custom (Material/iOS-themed) | Active, embedded focus | Possible alternative — see appendix |
| **Slint** | GPL / royalty-free / commercial | C/C++ | Custom, modern | Active | **Rejected** — GPL viral, royalty-free has strings |
| **`webview/webview`** | MIT | C | HTML/CSS rendered by OS webview | Active since 2017, used in prod | **Chosen** |

The `webview/webview` library (Serge Zaitsev) is what Wails uses for
Go, what many Rust apps used before Tauri matured, and what countless
indie cross-platform tools ship today. It is small (~1500 lines of
C/C++), MIT-licensed, and exposes a minimal API: create window,
navigate to URL, bind native functions, eval JavaScript.

## Decision

**`Amalgame.UI.Web` is the v1 UI toolkit.** It uses `webview/webview`
as the backend and exposes a thin Amalgame API on top.

The previous toolkit packages are sunset:

- `amalgame-ui-forms` (SDL) → frozen at v0.8.10. No further development.
- `amalgame-ui-tk` (Tcl/Tk) → development halted; package will not ship.
- A future `amalgame-gfx` package will cover the games / real-time /
  creative use cases. **Not part of this proposal.**

## Architecture

```
+--------------------------------------------------+
|  Amalgame app code                               |
|  (business logic, IO, SQLite, types, etc.)       |
+--------------------------------------------------+
                      ↕  bridge IPC
+--------------------------------------------------+
|  Amalgame.UI.Web runtime                         |
|  - window/lifecycle management                   |
|  - JSON-serialized message channel               |
|  - asset resolver (file:// in v1, am:// in v2)   |
+--------------------------------------------------+
                      ↕  C FFI
+--------------------------------------------------+
|  webview/webview C library                       |
+--------------------------------------------------+
                      ↕
+----------------+ +------------------+ +----------+
|  WebView2      | |  WKWebView       | | WebKitGTK |
|  (Windows)     | |  (macOS)         | | (Linux)   |
+----------------+ +------------------+ +----------+
                      ↕
+--------------------------------------------------+
|  HTML / CSS / JS frontend                        |
|  (in v1: hand-written; in v3: optionally         |
|   generated from a higher-level Amalgame DSL)    |
+--------------------------------------------------+
```

App developers write Amalgame for the backend and HTML/CSS/JS for the
frontend. Communication goes through a typed IPC channel:
`window.bind("command_name", handler)` exposes an Amalgame function to
the JS side, and `window.eval("...")` injects JavaScript into the page.

## v1 scope — what we ship

v1 is a **production-viable subset** for ~80% of productive desktop
apps: forms, dashboards, viewers, editors with HTML-based menus, tools.

### Capability matrix — v1

| Capability | v1 status | Notes |
|---|---|---|
| Native window with OS titlebar, resizable | ✓ | All 3 OSes |
| HTML rendering via OS webview | ✓ | WebView2 / WKWebView / WebKitGTK |
| Configurable initial size, title, min/max | ✓ | |
| Load HTML from local file, embedded resource, or HTTP | ✓ | |
| IPC: Amalgame ↔ JS (JSON-serialized args) | ✓ | `webview_bind` / `webview_eval` |
| DevTools | ✓ | Built-in on Windows/Linux; macOS requires a runtime flag |
| CSS animations at 60 fps | ✓ | Native webview perf |
| Hot reload during dev | ✓ | File watcher + reload IPC |
| Native menubar (File / Edit / View / …) | ✗ | Done in HTML for v1 |
| Native dialogs (open/save file, message box) | ✗ | Call OS dialog separately or use HTML `<input type=file>` |
| System tray / notifications | ✗ | Deferred to v2 |
| Multi-window with controlled lifecycles | ⚠ Limited | Single primary window recommended |
| Custom URL scheme (`am://`) | ⚠ Partial | `file://` works; `am://` deferred |
| Borderless / custom titlebar | ✗ | Deferred to v2 |
| Drag-and-drop integration with OS | ✗ | HTML drag-and-drop works inside the webview |

**What v1 covers in practice:**
forms apps, admin dashboards, internal tools, content viewers, simple
editors, settings/config UIs, onboarding wizards, log viewers,
data-entry apps, calculators, REPL frontends.

**What v1 does *not* cover well:**
apps with rich native menubars (Photoshop-style), tray-based daemons,
multi-window orchestration (IDE-style), heavily-customized window
chrome (VS Code-style titlebar with embedded menus).

For the menubar concern specifically — which was a pain point in the Tk
exploration — note that v1 ships menus rendered in HTML (CSS-styled
dropdowns). This is acceptable for most productive apps but is not OS
native. If true native menus are a v1 requirement, scope `1.1` to add
Win32 + NSMenu + GtkMenuBar — see "Menubar option" below.

## Theme strategy

**Decision (2026-05-15):** the default theme is the **OS theme** —
light or dark, auto-detected. Users override by shipping their own
stylesheets / images / assets next to the app; nothing more
prescriptive than that.

### Three layers, simplest first

1. **OS default, zero config.** Modern webview engines (WebView2,
   WKWebView, WebKitGTK) honor the `prefers-color-scheme` CSS media
   query natively from the OS — when the user toggles dark mode at
   the system level, the webview reflects it without app code.
   `Amalgame.UI.Web` ships a tiny baseline stylesheet that the
   builder API (`Page.ApplyTo`) injects when the page has no custom
   CSS attached. The baseline uses `@media (prefers-color-scheme:
   dark) { ... }` so all four cases (Windows light/dark, macOS
   light/dark, GNOME light/dark) just work.
   For backends needing more than a CSS hint, the runtime weak
   symbol `Amalgame_UI_DetectOSTheme()` (shipped via
   `amalgame-ui-sdl`) probes macOS `defaults`, the Windows registry,
   and Linux `gsettings color-scheme` — usable from `@c { … }`
   blocks if a non-webview signal is needed.

2. **External stylesheet override.** Two facade helpers shipped in
   v0.0.4:

   ```amalgame
   page.SetStylesheet("file:///abs/path/style.css")  // replaces baseline
   page.AddCss("file:///abs/path/extra.css")          // augments baseline
   ```

   The user keeps everything (colors, typography, layout polish) in
   normal CSS files alongside the app binary. Webview's same-origin
   rules mean `file://` URLs work without a custom protocol — for
   complex asset trees, switch to `am://` (v0.1.0, see native bundle
   below).

3. **External assets — images, fonts, icons.** Layered on stylesheet
   loading. `Page.AddAsset(path)` registers a file to be served
   alongside the page (either via `file://` URL or, post-v0.1.0, via
   the `am://` custom scheme handler). Referenced from the user's
   CSS / HTML normally.

### What the baseline stylesheet covers

System fonts via the `system-ui` / `-apple-system` / `Segoe UI` /
sans-serif stack. Light/dark color swap for body / text / links.
Reasonable margins, button padding, input borders that match the
OS look on each platform. ~100 lines of CSS, no opinions beyond
"don't look terrible on first load".

Users who want a designed app override everything via
`SetStylesheet`; the baseline isn't trying to be a design system.

## v0.1.0 scope — bundled native OS integration

v0.1.0 is the **"v1 MVP"** of this proposal: the milestone at which
`Amalgame.UI.Web` covers ~95% of productive desktop apps and reaches
feature parity with Tauri / Wails. The previous draft of this doc
spread these additions across v0.1.x → v0.3.x mini-releases; current
plan (decided 2026-05-15) is to **bundle them as one large milestone**
because they share infrastructure (per-OS C glue per feature, no
useful subset of them on its own).

The non-webview surface — i.e. every native OS object the user
expects from a productive desktop app, *not* the document content
(which stays HTML/CSS):

### Bundled native OS additions and effort estimates

| Feature | Estimated effort | Cumulative |
|---|---|---|
| Native menubar (Win32 + NSMenu + GtkMenuBar) | ~2 weeks × 3 OSes = 6 weeks | 6 |
| Native dialogs (file open/save, message box, color picker) | ~3 days × 3 OSes = 2 weeks | 8 |
| Custom URL scheme (`am://` asset resolver) | ~1 week × 3 OSes = 3 weeks | 11 |
| System tray + native notifications | ~1 week × 3 OSes = 3 weeks | 14 |
| Multi-window orchestration | ~2 weeks | 16 |
| Typed IPC channel (binary MessagePack via amalgame-msgpack) | ~1 week | 17 |
| Auto-update integration | ~1 week × 3 OSes = 3 weeks | 20 |
| OS-native drag-and-drop | ~1 week | 21 |
| Window chrome (borderless, custom titlebar / traffic lights) | ~2 weeks | 23 |

**Total v0.1.0 effort: ~20-23 focused engineering weeks** (~5-6
months part-time). Can be sequenced in 3 PRs per OS-touching item
to merge incrementally without holding the whole milestone hostage
to a Windows-only blocker.

**None of v0.1.0 breaks the v0.0.x API.** New surface, no existing
method signature changes — see the "Migration" section below.

### Menubar option pulled forward (v0.0.5)

If menu ergonomics block a real user before the full v0.1.0 bundle
is ready, the menubar can ship in v0.0.5 alone. It's the single
most-requested OS-native feature for users coming from Tk / native
toolkits. Cost: ~6 weeks for all 3 platforms.

## Migration v0.0.x → v0.1.0 — non-breaking, additive

This is the central guarantee of the proposal: **no Amalgame app
written against the v0.0.x API will break in v0.1.0.**

### v0.0.x API surface (shipped)

```
module Amalgame.UI.Web

# Window lifecycle
fn Window.new(title: Str, w: Int, h: Int) -> Window
fn Window.show(window: Window) -> Unit
fn Window.close(window: Window) -> Unit
fn Window.set_title(window: Window, title: Str) -> Unit
fn Window.set_size(window: Window, w: Int, h: Int) -> Unit
fn Window.set_min_size(window: Window, w: Int, h: Int) -> Unit

# Content loading
fn Window.load_url(window: Window, url: Str) -> Unit
fn Window.load_html(window: Window, html: Str) -> Unit

# IPC
fn Window.bind(window: Window, name: Str, handler: fn(Json) -> Json) -> Unit
fn Window.eval(window: Window, js: Str) -> Unit

# Event loop
fn App.run(window: Window) -> Unit
```

### v0.1.0 additive surface

```
# Menus (v1.1 or v2)
fn Window.set_menu(window: Window, menu: Menu) -> Unit
fn Menu.new() -> Menu
fn Menu.add_item(menu: Menu, label: Str, handler: fn() -> Unit) -> Unit
fn Menu.add_submenu(menu: Menu, label: Str, submenu: Menu) -> Unit

# Dialogs (v2)
fn Dialog.open_file(window: Window, filters: List<FileFilter>) -> Option<Str>
fn Dialog.save_file(window: Window, default_name: Str) -> Option<Str>
fn Dialog.message(window: Window, kind: DialogKind, text: Str) -> Unit

# Tray (v2)
fn App.tray(icon: Str, menu: Menu) -> Tray

# Multi-window (v2 — extension, single-window API stays valid)
fn App.add_window(window: Window) -> Unit
```

All v2 additions are **net-new functions**. Existing v1 function
signatures do not change. App developers who never call `set_menu` or
`Dialog.open_file` get exactly the same behaviour in v2 as in v1.

## Trade-offs

**Pros**
- Ships a viable v1 in ~3-4 weeks.
- Aligns with the de-facto industry standard for productive apps.
- HTML/CSS gives app developers the largest design ecosystem in
  existence (any CSS framework, any icon set, any font).
- DevTools come free.
- Binary footprint stays tiny (the OS webview is already installed).
- Forward-compatible API — no breaking changes in v2.

**Cons**
- Not OS-native look. Apps will feel like Electron apps (because they
  effectively are, minus the bundled Chromium).
- App developers need basic HTML/CSS/JS literacy. Mitigation: a v3
  Amalgame UI DSL could compile to HTML, hiding the web nature
  (deferred, not in this proposal).
- IPC latency adds ~1-5 ms per round-trip vs. native. Imperceptible
  for productive apps; problematic for real-time tools — but those
  belong in `amalgame-gfx`.
- WebView2 on Windows 10 < 1803 is not guaranteed present. The
  installer (`amalgame.iss`) must ship the WebView2 evergreen
  bootstrapper. Windows 11 has it built in.
- WebKitGTK availability varies across Linux distros. Mitigation:
  document the apt/dnf/pacman package names in the build guide.

## Deployment notes

- **Windows**: `amalgame.iss` must include the WebView2 evergreen
  bootstrapper for pre-1803 systems. Modern Windows 10/11 systems have
  it pre-installed.
- **macOS**: WKWebView is part of the OS since 10.10. No runtime to
  ship.
- **Linux**: WebKitGTK 2.40+ is required. Document the install
  command per distro family:
  - Debian/Ubuntu: `apt install libwebkit2gtk-4.1-dev`
  - Fedora: `dnf install webkit2gtk4.1-devel`
  - Arch: `pacman -S webkit2gtk-4.1`

## Implementation status

| Step | Status |
|---|---|
| Vendor `webview/webview` v0.12.0 under `runtime/vendor/webview/` (single C++ TU `webview.cc` triggering the C++ impl) | ✅ shipped v0.0.1 |
| C glue `runtime/Amalgame_UI_Web.{h,c}` — 64-slot table + minimal API (~200 LOC) | ✅ shipped v0.0.1 |
| Amalgame facade — Window class (10 methods) + WindowHint constants (~250 LOC) | ✅ shipped v0.0.1 |
| Sample apps `tests/spike.am` + `tests/spike_bind.am` + `tests/spike_builder.am` + reusable `tests/build_spike.sh` | ✅ shipped v0.0.1 → v0.0.3 |
| Linux deployment notes in `README.md` (WebKitGTK install per distro family) | ✅ shipped v0.0.1 |
| Bind IPC: trampoline + `Window.Bind` / `Unbind` (~150 LOC C + 30 LOC AM) | ✅ shipped v0.0.2 |
| Builder API: Element + Page + Html.Escape + Json.EncodeString (~320 LOC AM) | ✅ shipped v0.0.3 |
| Listed on `amalgame-lang/packages-index` so `amc package add ui-web` works | ✅ PR #18 merged |
| `amalgame.iss` WebView2 bootstrapper for Win10 < 1803 | ⏳ deferred — checking deploy on real machine first |
| CI smoke test on Linux (headless WebKitGTK via Xvfb) | ⏳ v0.0.4 — repo is fresh, no Actions wired yet |
| `Element.Bind(name)` — read DOM state from AM | ⏳ v0.0.4 candidate |
| Theme strategy implementation (baseline CSS + SetStylesheet) | ⏳ v0.0.4 candidate (this doc just landed the spec) |
| `amc new --template ui-web-form` scaffold | ⏳ v0.0.4 candidate |
| Bundled native OS additions (menubar, dialogs, tray, …) | ⏳ v0.1.0 milestone |

## Appendix — LVGL as a fallback

If, after v1 ships, native look becomes a strong user demand and
neither IUP nor a custom menubar layer satisfies it, **LVGL** (MIT,
pure C, themed) is the recommended fallback. It can coexist with
`Amalgame.UI.Web` as a second package (`amalgame-ui-native`) sharing
the same higher-level Amalgame DSL when that DSL eventually lands.

LVGL trade-offs vs. webview:
- Pro: no JS, no HTML; pure C runtime, smaller (~300-500 KB).
- Pro: built-in animations, Material/iOS-themed widgets, gesture
  handling.
- Con: not actually OS-native (custom-rendered); designed for
  embedded so desktop ergonomics (clipboard, IME, accessibility) need
  bridging.
- Con: another package to maintain.

For now, LVGL is **listed as an option, not a commitment.**

## Open questions

1. Should `Amalgame.UI.Web` ship a default CSS framework
   (Pico.css? Tailwind? a custom mini-stylesheet?) or stay
   framework-agnostic? Recommendation: ship a tiny opinionated
   stylesheet (~5 KB) that gives sane defaults; users can replace it.
2. Should the v1 IPC channel use JSON only, or accept binary
   (MessagePack via `amalgame-msgpack`) from the start? Recommendation:
   JSON only for v1; binary in v2 if profiling shows a need.
3. Should hot reload be a v1 feature or a dev-only tool shipped
   separately? Recommendation: v1, behind a `--dev` flag.
4. Should the proposed Amalgame UI DSL (v3) be sketched in a separate
   proposal, or be left fully open? Recommendation: separate proposal
   after v1 ships and real-world feedback arrives.
