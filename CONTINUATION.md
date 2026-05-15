# Continuation prompt — start a new chat with this

> **Last refreshed 2026-05-15 (late afternoon)** — **v0.8.12 tagged**
> + GUI ecosystem pivoted from SDL/Tk to webview. Massive day:
> released amc tags **v0.8.11** + **v0.8.12**, created the new
> [`amalgame-ui-web`](https://github.com/amalgame-lang/amalgame-ui-web)
> package and shipped it through v0.0.3, sunset `amalgame-ui-forms`
> and the never-published `amalgame-ui-tk` exploration.
>
> ### Headline: GUI toolkit pivot SDL/Tk → Webview (2026-05-15)
>
> Two sub-pivots the same day:
>
> - **Morning** — SDL retained-mode (`amalgame-ui-forms`) → Tcl/Tk
>   (`amalgame-ui-tk`). Reason: SDL drawing from scratch couldn't
>   produce rounded corners, OS native fonts, HiDPI, theming —
>   six cgen/typechecker bugs had piled up specifically around
>   `ui-forms`. Tk wraps native widgets through ttk on Linux,
>   Cocoa on macOS, Vista on Windows.
>
> - **Afternoon** — Tcl/Tk (`amalgame-ui-tk`) → Webview
>   (`amalgame-ui-web`). Reason: Tk's menubar was buggy on X11,
>   the "caméléon" promise was unconvincing in practice, and the
>   rest of the desktop ecosystem (VS Code, Slack, Discord,
>   1Password, Tauri) settled on webview years ago. ui-tk had
>   only ~6 h of investment, never pushed to GitHub.
>
> The decision matrix lives in
> [`docs/proposals/amalgame-ui-web.md`](docs/proposals/amalgame-ui-web.md)
> — industry survey across 10 modern language ecosystems,
> options matrix (IUP / libui-ng / LVGL / Slint / webview),
> v0.0.x → v1.0 capability roadmap with effort estimates,
> deployment notes per OS, implementation plan.
>
> ### v0.8.12 (cross-pkg chains + inline lambda fixes) ✅ tagged
>
> 1. **PR #456 — cgen bugs #2 + half of "inline lambda as arg"**.
>    - **Cross-package chained method calls** (ROADMAP #2). For
>      `ExtType.Static().Method()` where `ExtType` lives in an
>      `--external` facade, EmitCalleeStr's CALL-on-MEMBER branch
>      called InferTypeFromExpr which fell back to the consumer's
>      `SymName` (e.g. `App_Page` instead of `Amalgame_UI_Web_Page`).
>      `MethodRet` was empty under that key, so the fallback
>      returned the raw `EmitExprStr(callee.Left) + "_" + mname`
>      string, producing invalid `Type_Static()_Method(...)` C
>      tokens. Two-part fix: `RegisterExternalProg` now calls
>      `MethodRetSet` for every external method; InferTypeFromExpr's
>      static path tries `ExternalClassMangled` before `SymName`.
>    - **Inline lambda as argument** outside Map/Filter dispatch.
>      EmitExprStr for `__lambda__` returned a
>      `__lambda_<param>_<body>__` placeholder string. Higher-order
>      list methods intercept earlier via EmitClosureArg, so the
>      placeholder only surfaced at non-list call sites (Bind,
>      Element.OnClick, etc.) where it became garbage C. Replaced
>      with direct `EmitLambdaAsClosure(expr)` so every inline
>      lambda emits a real `AmalgameClosure_new(...)` compound
>      expression.
>
>    Discovered while bootstrapping `amalgame-ui-web` v0.0.3's
>    fluent builder. 5 of 6 ui-forms cgen bugs are now closed; only
>    **#6 `let` scope flattened** remains open (no consumer
>    retriggered it intra-package in v0.0.x).
>
> 2. **PR #459 — ROADMAP_COMPLET sunset banner** + **PR #460 —
>    proposal doc committed**. Documentation companion changes.
>
> ### v0.8.11 (cgen brace literal + typed-param lambdas + Closure) ✅ tagged
>
> 1. **PR #452 — cgen mis-parses `{` `}` `:` inside string literals**.
>    `EmitInterpolatedString` accepted any `{x...}` content starting
>    with a letter (or `this.`) as an interpolation slot. CSS rules
>    like `body{font-family:system-ui}` silently triggered
>    interpolation parsing and produced malformed C. New
>    `IsValidInterpExpr` requires the brace content to match
>    `ident(.ident)*` with optional trailing `(args)`.
>
> 2. **PR #453 — typed-param lambdas + return-type-aware closure
>    calls + Closure type**. Multi-param `(req: string) => "..."`
>    style now lowers correctly: ParseLambdaMulti consumes optional
>    `:type` annotations; `InferTypeFromExpr` stashes the lambda's
>    return type via a new `__closure_ret__` map; closure call
>    sites emit a direct cast for pointer-typed returns
>    (`code_string`, `*`) instead of the legacy hardcoded
>    `UnboxScalar("i64", …)`. New AM type `Closure` →
>    `AmalgameClosure*` so facade methods can declare
>    `handler: Closure` without per-package typedef shims.
>
>    Unblocks every C-trampoline-style binding the stdlib might
>    add next — webview Bind is the first consumer; filewatcher
>    OnChange / http handler callbacks would land the same way.
>
> ### amalgame-ui-web v0.0.3 — webview GUI shipped
>
> **Three iterations the same day.** Repo at
> [`amalgame-lang/amalgame-ui-web`](https://github.com/amalgame-lang/amalgame-ui-web)
> (public). Wraps the MIT-licensed
> [`webview/webview`](https://github.com/webview/webview) v0.12.0
> (vendored at `runtime/vendor/webview/webview.h`) and renders
> HTML/CSS/JS via WebView2 on Windows, WKWebView on macOS,
> WebKitGTK on Linux.
>
> - **v0.0.1** — single window MVP: `Window(title, w, h, debug)`,
>   `IsValid`, `SetTitle`, `SetSize(w, h, hint)`, `Navigate(url)`,
>   `SetHtml(html)`, `Init(js)` (pre-page-load inject), `Eval(js)`,
>   `Run()` (blocking event loop), `Terminate()`, `Destroy()`.
>   `WindowHint.None/Min/Max/Fixed` mirror `webview_hint_t`.
>
> - **v0.0.2** — bidirectional IPC: `Window.Bind(name, handler:
>   Closure)` + `Window.Unbind(name)`. Backed by a 64-slot
>   trampoline registry in the C glue; the fixed
>   `_amalgame_uiweb_trampoline` dispatches every JS call into
>   `AmalgameClosure_call2(req, NULL)` and forwards the returned
>   `code_string` to `webview_return` automatically. Handlers
>   must return valid JSON — `Json.EncodeString` helper ships
>   alongside.
>
> - **v0.0.3** — HTML builder API. `Element` class with static
>   builders (`Stack`, `Row`, `Label`, `Heading`, `Button`,
>   `Input`, `Pre`, `Div`) + fluent `Attr/Id/Class/Style/AddChild/
>   SetText/OnClick` chaining. `Page` class with
>   `New().SetTitle(...).SetBody(...).ApplyTo(window)`. Page.Render
>   walks the tree, allocates a `_amc_<n>` name per OnClick element,
>   and ApplyTo auto-binds each handler via Window.Bind. The fluent
>   chain pattern is what surfaced the cgen #2 bug fixed in v0.8.12.
>
> Listed on `amalgame-lang/packages-index` (PR #18), so
> `amc package add ui-web` works end-to-end. Requires amc
> >=0.8.12.
>
> ### Sunset 2026-05-15 — ui-forms + ui-tk
>
> `amalgame-ui-forms` v0.1.4 stays listed on packages-index for
> existing consumers but receives a top-of-README banner pointing
> users to `amalgame-ui-web` (PR #1 on that repo, merged). No
> further releases planned. `amalgame-ui-sdl` v0.1.0 stays as the
> foundation for a future `amalgame-gfx` package (games / 3D /
> real-time viz, separate scope).
>
> `amalgame-ui-tk` was never pushed to GitHub — local sunset only.
>
> ROADMAP_COMPLET entry (Forms toolkit) carries the sunset banner
> and points to the design proposal (PR #459, merged).
>
> ### What's next — v0.0.4 candidates
>
> Open in [`memory/project_ui_pivot.md`](https://github.com/amalgame-lang/Amalgame/blob/main/.claude/projects/-home-neitsab-D-veloppement-Amalgame/memory/project_ui_pivot.md)
> (private to author's environment, mirrored at the top of every
> session). Three feature buckets ordered by user pain:
>
> 1. **`Element.Bind(name)` — read DOM state from AM**. Currently
>    AM can push HTML/text/JS into the webview (SetHtml, Eval)
>    and react to JS calls (Bind), but can't read input values
>    declaratively. v0.0.4 candidate: an Input/Select/Textarea
>    accessor that injects bridging JS at Render time, so the
>    OnClick handler can read `(req: string)` as a JSON object
>    with form field values pre-parsed.
>
> 2. **OS theme handling** — webview default theme should match
>    the OS (light/dark), with overrideable CSS/asset files.
>    Detection already exists via the
>    `Amalgame_UI_DetectOSTheme()` weak symbol shipped with
>    `amalgame-ui-sdl` (probes macOS `defaults`, Windows registry,
>    Linux `gsettings color-scheme`). v0.0.4 ships a
>    baseline `data-theme` stylesheet that ApplyTo injects when
>    the page has no custom CSS; users override via standard
>    `<link rel=stylesheet>` or via a new
>    `Page.SetStylesheet(path)` helper. Modern webviews honor the
>    `prefers-color-scheme` media query natively, so user CSS
>    can write `@media (prefers-color-scheme: dark) { ... }`
>    and just work.
>
> 3. **Native menubar — biggest UX gap left.** Win32 + NSMenu +
>    GtkMenuBar. ~6 weeks × 3 OS. v0.0.5 or v0.1.0 candidate.
>
> Remaining capability axis toward v1.0 (per the proposal):
> custom URL scheme `am://`, native file dialogs, system tray +
> notifications, multi-window orchestration, IPC binary, auto-
> update, OS drag-drop. ~20-25 focused engineering weeks total,
> spreadable over v0.1.x → v0.3.x.
>
> ### Cgen backlog status
>
> 5 of 6 ui-forms cgen bugs closed. Open:
>
> - [ ] **#6 `let` scope flattened to function level** — two
>       `let x: T = ...` in different `if`/`while` blocks collide
>       at the C level. **Workaround**: rename. **Fix**: resolver
>       tracks block scope, cgen emits per-block C scopes.
>       Hasn't reproduced intra-package in ui-web v0.0.x — picks
>       up when a consumer hits it again.
>
> ---
>
> ### Previous push — 2026-05-14 (late night, archived below)
>
> ### v0.8.10 (close 2 cgen bugs + SDL2 in installer) ✅ tagged
>
> 1. **PR #447 — cgen bugs #4 + #5 closed**. Two of the six
>    bugs surfaced by ui-forms development:
>    - **#4 parens lost on mixed `* + /`** — `EmitExprStr`
>      BINARY branch now wraps any sub-BINARY operand in
>      parens. `(h - 2*pad - gap*(n-1)) / n` lowers correctly
>      instead of left-associating against C precedence.
>    - **#5 `return null` rejected by the typechecker** —
>      `IsAssignable` accepts null for any target that isn't a
>      primitive value type (new `IsPrimitiveValue` helper).
>      Drops the `@c { return NULL; }` workaround pattern.
>
>    4 cgen bugs remain open in ROADMAP under "Cgen/typechecker
>    bugs surfaced by amalgame-ui-forms" (#1 forward-decl
>    ordering, #2 chained calls cross-pkg, #3 field=type
>    shadow, #6 let scope flat). Intra-package smoke tests no
>    longer trigger any of them — may already be collateral-
>    fixed by #4 + #5, or need a cross-package repro.
>
> 2. **PR #446 — install.sh installs SDL2 by default**.
>    libsdl2-dev + libsdl2-ttf-dev are now pulled in alongside
>    libgc + libcurl + zlib on apt/dnf/pacman/zypper/brew/pkg.
>    ~5 MB footprint; opt out with `AMC_NO_GUI=1 ./install.sh`
>    for minimal installs. GUI users no longer have to install
>    SDL manually before `amc new --template forms`.
>
> 3. **PR #450 — register-package.sh latest-run-only**.
>    Re-tagging a package after a CI fix left the historical
>    failure in `gh run list`, blocking the script from
>    registering the freshly-green tag. Now checks `--limit 1`
>    so only the latest run gates registration.
>
> ### v0.8.9 (tag fix + install hints) ✅ tagged
>
> 1. **PR #442 — `amc new --template forms` tags fixed**.
>    Scaffolded `amalgame.toml` hardcoded `v0.0.6-dev` for both
>    packages — those never existed on GitHub. Bumped to the
>    published stable tags (ui-sdl v0.1.0 + ui-forms v0.1.1).
>
> 2. **PR #443 — SDL2 install hints**. install.sh post-success
>    prints OS-specific SDL2 install snippets; Homebrew formula
>    gains sdl2 + sdl2_ttf as runtime deps. (Superseded by
>    PR #446 in v0.8.10, which installs them by default.)
>
> ### v0.8.8 (forms scaffolder + multi-spec) ✅ tagged
>
> 1. **PR #436 — `amc new --template forms`**. Scaffolds a
>    GUI project: `src/main.am` (Form + Label + Button +
>    StackVertical), `amalgame.toml` (deps on ui-sdl + ui-forms),
>    `build.sh` (amc emit .c → gcc link facade archives + SDL2
>    pkg-config), `README.md` (apt/brew/pacman install snippets).
>    Sample opens a 320×240 window via `Application.Run`.
>
> 2. **PR #437 — `amc package add` multi-spec**. Accepts
>    several positional args; runs the existing per-package
>    install pipeline (extracted into `RunOne`) for each in
>    order. Stops on first failure so the user sees the
>    broken spec, not a cascade.
>
> 3. Plus: XDG runtime probe in PrecompileFacade + PrecompilePackage
>    (used to look only under `<bin>/runtime`, the legacy in-
>    tree path; XDG `<bin>/../share/amalgame/runtime/` is now
>    checked first).
>
> ### v0.8.7 (amc --lib cross-package facade deps) ✅ tagged
>
> 1. **PR #433** — `amc --lib facade.am` now loads every
>    LoadedPackage's facade as `--external`, not just none.
>    Pre-v0.8.7 the path skipped the whole loop, so facades
>    importing other packages (ui-forms → ui-sdl) hit
>    'Unknown symbol' at the resolver. Self-package still
>    skipped (matched on namespace) to avoid the
>    forward-decl-instead-of-definition loop.
>
> ### GUI toolkit summary (ui-sdl + ui-forms)
>
> Shipped at v0.1.0 (then patched up to v0.1.4). Two-package
> split:
>
> - **ui-sdl v0.1.0** — thin SDL2/SDL3 binding. Surface:
>   Window (create/close/size/title/event poll), Event
>   (Quit/MouseDown/MouseUp/MouseMove/KeyDown/KeyUp/Resize),
>   Surface (Clear/Present/FillRect/DrawRect/DrawLine/DrawPixel),
>   Font (LoadDefault cross-OS probe + DrawText + MeasureWidth/
>   Height), Color, Rect, OSTheme.DetectOS (returns
>   "light"/"dark" from macOS defaults / Windows registry /
>   gsettings). Backend chosen at compile time via
>   `-DAMALGAME_UI_USE_SDL3` (default SDL2).
>
> - **ui-forms v0.1.4** — retained-mode GUI toolkit on top of
>   ui-sdl. Single concrete `Widget` class with a `Kind` tag
>   (class-with-tag pattern — amc 0.8.x's typechecker rejects
>   subclass upcasts). 9 kinds: Label, Button, CheckBox,
>   RadioButton, TextBox (focus + printable-ASCII typing +
>   backspace), Panel, ListBox, ComboBox, MenuBar. Form
>   container with 4 layouts (StackVertical/StackHorizontal/
>   Grid/Absolute). Theme.Light/Dark/FromOS palette.
>   `Application.Run` blocking event loop with mouse-down
>   hit-testing, keyboard focus dispatch, layout repack on
>   resize. Label + Button text centered horizontally +
>   vertically (font.MeasureHeight + MeasureWidth).
>
> Scaffold: `amc new <name> --template forms` (lands in
> v0.8.9+). Sample app: 320×240 window with Label + Button.
>
> ### v0.8.1 (polish the debugger) ✅ tagged
> Four PRs landed on top of v0.8.0 to make the DAP usable
> without any "ahh I forgot the env var" friction.
>
> 1. **PR #395 — `amc new --vscode`** opt-in flag. Scaffolds
>    `.vscode/launch.json` (two configurations: POSIX +
>    Windows `.exe`, F5 dropdown picks) and
>    `.vscode/settings.json`. Opt-in keeps Neovim / Helix /
>    IntelliJ projects clean. Advertised in `amc --help` on
>    the `new` line.
>
> 2. **PR #396 — `Program.ResolveSelfPath()` + scaffold
>    cleanup**. Fix: `amc build` derived `runtime/` from
>    `dirname(argv[0])`. When amc is launched via PATH,
>    argv[0] is the bare `amc`, dirname collapses, and gcc
>    never gets `-I'<runtime>'` → `_runtime.h: fichier ou
>    dossier de ce type`. `ResolveSelfPath()` reads
>    `/proc/self/exe` (Linux) / `GetModuleFileNameA`
>    (Windows); macOS falls back to argv[0] until
>    `_NSGetExecutablePath` lands. Same PR rewrites
>    `BuildShExe` / `BuildShService` / `BuildPs1Service`
>    templates into two-line wrappers around `amc build`
>    that forward `"$@"` (so `./build.sh -g` propagates).
>
> 3. **PR #397 — `tasks.json` + `preLaunchTask` + `amc test`
>    path fix + READMEs**. Scaffold now drops
>    `tasks.json` ("amc: build (debug)" + "release"); both
>    launch configs carry `preLaunchTask: "amc: build
>    (debug)"`. F5 rebuilds with `-g` automatically — no more
>    "F5 runs but no breakpoint stops" when the binary on
>    disk is release. `amc test` got the same
>    `/proc/self/exe` fix BuildEntry got in PR #396 (was
>    missed). Generated READMEs gain a "Debug" section.
>
> 4. **PR #398 — XDG install layout, cross-OS**. All three
>    install paths now share the same tree:
>    `<prefix>/{bin/amc[.exe], share/amalgame/{runtime,lib,docs}}`.
>    `install/install.sh` (Linux / macOS / FreeBSD) does a
>    flat `cp -r dist/$NAME/* $PREFIX/` from the staged
>    tarball — no more partial `_runtime.h`-only copy, no
>    forced `AMC_RUNTIME` in shell rc. `install/windows/
>    amalgame.iss` got rebuilt (was pinned to `0.3.0` with
>    broken paths). `.github/workflows/release.yml` stages
>    the matching XDG tree on all 3 OS jobs. New helpers
>    `Program.ResolveRuntimeDir(amcPath)` +
>    `Program.ResolveLibAmalgameA(amcPath)` probe a stable
>    chain: `$AMC_RUNTIME / $AMC_LIB` →
>    `<bin>/../share/amalgame/{runtime, lib}` →
>    `<bin>/{runtime, lib}` (legacy). No env var required.
>    Inno Setup `[Registry]` AMC_RUNTIME write gone.
>
> ### v0.8.0 (debug adapter) ✅ tagged earlier this session
>
> Recap for context:
> - **`amc dap`** — thin DAP proxy (`src/dap.am`, ~130 lines).
>   Detects `lldb-dap` (LLVM 18+ today, gdb --dap pending
>   v0.8.2), `execvp()`s into it. Transparent — no in-amc
>   message copy ("Approche C" in ROADMAP_COMPLET.md).
> - **`amc build --debug` / `amc run --debug`** (alias `-g`)
>   swap `-O2` for `-O0 -g` on gcc/g++. Watch builds keep
>   `-O2`.
> - **`#line N "foo.am"` directives** at every statement
>   whose source line differs from the previous. gcc + clang
>   honour these → DWARF carries `.am` filenames + line
>   numbers natively. No source maps needed.
> - **VS Code extension v0.3.0** registers `amc` debug type +
>   `DebugAdapterDescriptorFactory` that spawns `amc dap`.
>
> ### Ecosystem and bundled stdlib (updated 2026-05-14)
>
> **15 official external packages**:
> - *Pure-AM facades* — math, math-vec, random, encoding, crypto,
>   datetime, logging, service, io-filewatcher, yaml,
>   **ui-forms** (new, v0.1.0)
> - *C-header bindings* — regex, compress, net-websocket,
>   **ui-sdl** (new, v0.1.0)
> - *Database / messaging legacy* — sqlite, redis, mqtt, duckdb
>
> **GUI toolkit shipped 2026-05-14**: ui-sdl + ui-forms cover
> the full Window/Event/Surface/Font + retained-mode widget
> stack (Form + 9 widget kinds + 4 layouts + theming).
> Scaffolder: `amc new <name> --template forms`. Sample app
> opens a 320×240 window with a Label + Button via
> `Application.Run`. Requires SDL2 + SDL2_ttf dev headers
> (libsdl2-dev on Debian; brew install sdl2 sdl2_ttf on macOS).
>
> **amc 0.8.7+ unblocks**: cross-package facade deps in
> `amc --lib` (PR #433) — let ui-forms's facade.am pull
> Color/Window/Event from ui-sdl via `import Amalgame.UI.SDL`.
> Without this fix the resolver bailed on cross-pkg types.
> Also on develop (next release): `amc package add` accepts
> multiple specs (PR #437) — `amc package add ui-sdl ui-forms`
> instead of two invocations.
>
> **Bundled stdlib remaining**: `runtime/_runtime.h` +
> `Amalgame_{String,Collections,Console,IO,Net,Process}.h`
> (bootstrap surface amc itself uses), and `src/stdlib/`
> `{json,toml,msgpack,path,amc_buildinfo.am.in}`. msgpack stays
> bundled until a cgen ABI bug is fixed (see "Persistent todos"
> below).
>
> **Tests**: 451/451 PASS in amc + 85/85 across the 9+ packages
> with local runners. Last tag: **`v0.8.10`** (plus ui-sdl
> v0.1.0 + ui-forms v0.1.4 on the external side).
>
> **DECISIONS recorded at top of `ROADMAP_COMPLET.md`**:
> - No new `runtime/Amalgame_*.h` after v0.7.3 — new C
>   bindings ship as standalone external packages.
> - **DAP strategy is hybride C→A** — v0.8.x stays on the
>   transparent proxy; the bridge-MI migration ("Approche A":
>   pretty-print AmalgameList*/AmalgameMap*, filter runtime
>   frames, decode closures) is explicitly tracked as future
>   work, not a nice-to-have.

## Resume here — post-v0.8.10 trajectory

The GUI ecosystem ships. Picking up next session, the
priorities in rough order:

1. **Remaining 4 cgen bugs** surfaced by ui-forms. #4 + #5
   closed in v0.8.10; #1 #2 #3 #6 still open under
   "Cgen/typechecker bugs surfaced by amalgame-ui-forms" in
   `ROADMAP_COMPLET.md`. Intra-pkg smoke tests don't repro
   them anymore — write minimal cross-package test cases
   before claiming any of them is dead.
   - **#1 forward-decl ordering** — type-of-type referenced
     before its declaration.
   - **#2 chained calls cross-pkg** — `pkg.A().B()` flattens
     to a missing intermediate cast when A returns an
     external incomplete typedef.
   - **#3 field shadows type name** — class `Foo { Foo Foo }`
     rejected; rename field workaround.
   - **#6 let scope flat** — `let x` inside `if {}` leaks
     into the enclosing block.

2. **macOS canonical-path resolution** — mirror PR #396's
   `/proc/self/exe` fix using `_NSGetExecutablePath` from
   `<mach-o/dyld.h>` in `Program.ResolveSelfPath()`. ~5 lines
   of `@c {}` block. Removes the last edge case where
   `amc build` can't find runtime/ via PATH on macOS.

3. **`gdb --dap` fallback in `src/dap.am`** — Linux + Windows
   MSYS2 users get a second backend after lldb-dap. Probe
   order: keep the existing `lldb-dap*` chain, then try
   `gdb` (parse `gdb --version` first line for ≥ 14). When
   picked, `execvp("gdb", ["gdb", "--dap", NULL])`. ~30
   lines.

4. **Facade ABI cgen fix** (blocks msgpack extraction). When
   a package's `facade.am` calls its own static methods via
   `ClassName.X()`, `EmitCalleeStr` hits
   `PkgClassMangledPrefix` (returns the namespace) before
   the `SymName` fallback (would include the class name).
   Symbol mismatch + gcc implicit-int → runtime segfault.
   Fix: check `IsLocalClass(tname)` **before**
   `PkgClassMangledPrefix` in `EmitCalleeStr` (line 3515) +
   mirror in `TypeToC` (line 3793). Then extract
   `amalgame-msgpack` as the 14th package.

5. **LSP package discovery code action** — `amc package
   suggest <namespace> --json` is already shipped (v0.7.7).
   Wire `textDocument/codeAction` to detect unresolved-import
   diagnostics, call `amc package suggest`, and offer
   "Install package X for Amalgame.Y" quickfixes. Pattern:
   Visual Studio's `using X;` lightbulb.

6. **HiDPI scaling in ui-forms** — item 6 of the original
   GUI plan, partially deferred. SDL2 gives us
   logical-vs-physical resolution; widget metrics need a
   `Theme.ScaleFactor` multiplier and Font sizes need to
   honor it. ~3-4h once we have a HiDPI test box.

7. **2D/3D graphics + WYSIWYG VS Code form designer** —
   future scope, captured in ROADMAP_COMPLET.md
   "GUI future scope" section. Don't start until v0.8.x
   compiler bugs are closed.

8. **Approche A — DAP message-rewriting bridge** (long-term).
   Pretty-print `AmalgameList*` / `AmalgameMap*`, filter
   `Amalgame_*` / `_runtime.h` frames. Swap `execvp` for
   fork+pipe+`poll()` and rewrite messages on the way
   through; the transparent proxy stays available behind
   `amc dap --raw`. ~6-10h. Explicit dette technique in
   `project_dap_strategy.md` auto-memory.

## Persistent todos (don't lose these)

- **BUG TRACKER** — pre-existing cgen bugs hit during the
  DateTime migration:
  - ~~**Parens lost on mixed `* + /`**~~ — **FIXED in v0.8.10**
    (PR #447). `EmitExprStr` BINARY branch wraps sub-BINARY
    operands. Workaround locals can be removed at leisure.
  - **Multi-line expression continuation broken** — the parser
    closes the statement at the newline even with a binary
    operator at end-of-line. Workaround: single-line expr or
    intermediate locals.
- **Facade-package ABI bug** — see "Resume here" item 2 above.
  Documented in `ROADMAP_COMPLET.md` under "Compiler open bugs".
- **WS deferred** — wss:// TLS, binary opcodes, continuation
  frames, per-message-deflate, HTTP subprotocols. The
  `amalgame-net-websocket` package owns this now; bump
  package version when a real consumer lands.
- **Registering a package on packages-index** — use
  `./tools/register-package.sh <shortname> <tag>` after the
  package's own CI is green on the tagged ref. The script
  validates the tag exists, reads the package's manifest to
  pull `required-amalgame`, and opens a PR on
  `amalgame-lang/packages-index` appending the
  `[[version]]` entry. Replaces the per-repo
  `.github/workflows/index-pr.yml` pattern (which required a
  PACKAGES_INDEX_PAT secret on every package repo) — with the
  script the credentials live in the developer's `gh auth`
  session. Don't add `.github/workflows/index-pr.yml` to new
  packages; just run the script.
- **AMM draft** — PR #370 (closed superseded) seeded
  `docs/memory-management/` with five design documents for a
  hypothetical bdwgc → automatic-lifetime replacement. Files
  live on develop tip; review/iterate when ready.

## Plan for v0.7.4 (project G — historical, shipped)
>
> 1. **Phase 1 (MVP parser + cgen)** —
>    - Lexer: tokenise `@c {` as a raw-C span marker; the body
>      bytes flow through opaque until the matching `}` (count
>      nested braces).
>    - Parser: produce `NodeKind.INLINE_C` with the raw body
>      stored in `Str`. Detect `@out = …;` syntax inside.
>    - Resolver: treat the block as opaque — return type comes
>      from the enclosing method signature; arg types from the
>      surrounding scope.
>    - CGen: splice the body verbatim. Wrap with a compound
>      statement so locals are correctly scoped. Map
>      `@out = expr;` to `return (RT) expr;` where RT is the
>      enclosing method's declared return.
>    - Tests: 3-4 fixtures (`@c { return strlen(s); }` style).
>
> 2. **Phase 2 (POC migration)** —
>    `runtime/Amalgame_BuildInfo.h` is the smallest candidate
>    (10 useful lines, 2 inline helpers + 2 defines). Migrate
>    it to `src/stdlib/amc_buildinfo.am` with `@c {}` blocks
>    for the `return AMC_GIT_REV;` parts. Verify
>    `amc --version` still works + 602 tests still green.
>
> 3. **Phase 3 (extensions)** —
>    `@c_include "<header.h>"` at file scope (for std libc
>    headers). `@c_link "name"` (passed as `-lname` to gcc).
>    Doc warning that inline-C is `unsafe`-like.
>
> ## Build state (2026-05-13)
>
> - `./amc --version`: `amc 0.7.5 (commit a623f91…)` on develop tip;
>   `feat/libamalgame-pkg` carries `d49e305` on top.
> - `./build_amc.sh`: ~2-3s end-to-end (Step 0 sed-stamps build
>   provenance into `amc_buildinfo.am`; Step 4 builds
>   `lib/libamalgame.a` ≈ 200 KB).
> - `./tests/run_all_tests.sh`: **613/613 PASS** (216 core + 351
>   stdlib + 12 fmt + 34 amc-new).
> - libgc-dev + libcurl4-openssl-dev + zlib1g-dev required for the
>   bootstrap. MSYS2 also needs `mingw-w64-x86_64-libsystre` for
>   POSIX `<regex.h>`.
>
> The block below is the v0.6.0-era prompt — kept for the
> compiler/bootstrap context which is still accurate. Read it
> after the above for the "how the codebase is laid out" view.

---

```
I'm working on Amalgame, a self-hosted programming language that
transpiles to C. I keep the project in
/home/neitsab/Développement/Amalgame.

Current state (May 2026, v0.6.0):

═══════════════════════════════════════════════════════════════
  Compiler + bootstrap
═══════════════════════════════════════════════════════════════

- The compiler `amc` is written in Amalgame in src/ and compiles
  itself in ~2 seconds via ./build_amc.sh.
- 2-rung bootstrap chain in build_amc.sh:
    ./amc                  → current self-hosted (may break in dev)
    ./snapshot/amc         → last known-good amc, captured by
                             tools/save-snapshot.sh after green tests.
                             snapshot/amc_lib.c is committed; from a
                             clean clone, rebuild with one gcc step
                             (see snapshot/INFO.md).
- The runtime headers are at runtime/. Cross-platform (POSIX +
  Windows winsock2 via #ifdef _WIN32 in Amalgame_Net.h). libcurl is
  required by the runtime for the Net module + the claude-api /
  chatgpt / gemini providers.
- Test runner (./tests/run_all_tests.sh) drives ./amc directly.
  Build artefacts go to /tmp via mktemp; the source tree stays
  clean. Currently **205 core / 258 stdlib / 12 fmt / 34 amc-new
  = 509 PASS / 0 FAIL / 0 SKIP** across the sub-suites.
- Multi-OS CI (.github/workflows/ci.yml) — Linux + macOS + Windows
  MSYS2. All three platforms gcc the snapshot/amc_lib.c then chain
  through build_amc.sh.
  CI compiles with -Wint-conversion as an error on macOS/Windows;
  pin int-typed locals via `let n: int = …` when the codegen erases
  the return type to void* across a method-call boundary.
- Releases automated on `v*` tag (.github/workflows/release.yml).
  Latest is **v0.6.0** — see CHANGELOG.md for the per-release detail.
  develop → release/vX.Y.Z → develop → main → tag is the release flow.
  Both develop and main are protected (force-push + delete blocked,
  PR required, admin bypass allowed for owner-driven release flow).
- VS Code extension in editors/vscode/ — TextMate grammar +
  language config + LSP client (vscode-languageclient over stdio).
  Configurable via `amalgame.serverPath` in user settings.
- Formatter: `amc fmt file.am` re-emits canonical source with
  comments preserved. Idempotent on every compiler source.
- **`PackageRegistry.AmcVersion()` in src/package_registry.am** is
  the single source of truth for the version string since v0.5.0.
  `main.am`'s `--version` reads it; the manual-release flow only
  edits this one constant. tools/release.sh handles everything else.

═══════════════════════════════════════════════════════════════
  Package manager — full surface
═══════════════════════════════════════════════════════════════

`amc package <action>` (alias `amc pkg`) with these verbs:

  amc package add <name|url>[@<tag>] [--no-precompile]
       Install a package. If @<tag> omitted for an INDEXED
       shortname, auto-resolve to the latest tag whose
       `required-amalgame` is satisfied by the running amc.
       Full git URLs still need an explicit @<tag>.
       --no-precompile skips install-time compile even if the
       manifest declares `[stdlib].precompile = true`.

  amc package search [keyword] [--refresh]
       Browse the curated index. Each result shows known tags +
       compat status (✓/✗ vs running amc) + a "← latest compatible"
       marker. --refresh wipes the 30-min cache to force re-fetch.

  amc package versions <name> [--refresh]
       Shortcut: `search` output filtered to one package.

  amc package list
       Show installed deps from amalgame.lock with their pinned
       tags. Format: `<ClassName> @ <tag> — <slug>`.

  amc package remove <name>[@<tag>] [...]
       Strip dep(s) from amalgame.toml + amalgame.lock. The
       optional @<tag> safety suffix refuses to remove unless
       the installed tag matches.

  amc package update <name>@<tag>
       Bump a pinned tag (delegates to add under the hood).

  amc package cache clear [--all]
       Drop cached packages and/or index file.

**Manifest format** (`amalgame.toml` in each package repo):

```toml
[package]
name              = "amalgame-database-duckdb"
version           = "0.1.1"
license           = "Apache-2.0"
description       = "DuckDB binding — vendored C++ amalgamation"
authors           = ["Bastien Mouget"]
required-amalgame = ">=0.5.4"   # or ^, ~, =, >, <, <=, bare
schema-version    = 1

[stdlib]
class      = "DuckDB"
header     = "runtime/Amalgame_Database_DuckDB.h"
namespace  = "Amalgame.Database.DuckDB"
sources    = ["runtime/Amalgame_Database/duckdb/duckdb.cpp"]
cflags     = "-O2 -DNDEBUG"           # extra flags for .c sources
cxxflags   = "-O2 -DNDEBUG -std=c++17" # extra flags for .cpp/.cc/.cxx
libs       = ["stdc++"]                # -l<name> at final link
precompile = true                       # compile at `amc package add` time

[stdlib.functions]
Open       = { returns = "AmalgameDuckDB*" }
# … etc
```

**Required-amalgame operators** (v0.6.0+): `>=`, `>`, `<=`, `<`,
`=`, `^` (caret, 0.x-aware npm flavour), `~` (tilde, locks
major.minor). Bare version treated as `>=` for back-compat.

**Precompile-on-install** (v0.5.4+): when `precompile = true`,
`amc package add` compiles each `[stdlib].sources` entry into a
persistent cache at:

```
~/.amalgame/packages/<host>/<owner>/<repo>/<tag>_<sha>/
└─ build/<platform>/<class>-<basename>.o
```

`<platform>` = `linux-x86_64` / `macos-arm64` / `windows-x86_64`
(lowercased `$(uname -s)-$(uname -m)`). Cross-OS users sharing a
$HOME don't collide.

Subsequent `amc test` / build look there first, fall back to
`/tmp/amc-pkg-<class>-<basename>.o` (v0.5.2 lazy cache), then to
fresh compile. `--no-precompile` opts out per-install.

**Calibration** (v0.5.4+): each precompile writes a sample to
`~/.amalgame/calibration.toml`:

```toml
[[sample]]
lang      = "cxx"
size_kb   = 24944
elapsed_s = 882
pkg_ver   = "amalgame-database-duckdb@v0.1.1"
```

Future installs read these and compute ETA via weighted average
on the current machine. First time on a new machine: no ETA, but
elapsed time printed after.

**Cross-platform `$HOME` resolution** (v0.5.4+):
`PackageRegistry.AmalgameHome()` walks `$AMALGAME_HOME` →
`$HOME` → `$USERPROFILE`. Fixes a pre-existing bug where amc fell
back to `/tmp` on native Windows shells (only MSYS2 was being
exercised by CI).

**Index cache TTL** (v0.5.6+): `FetchIndex` considers
`~/.amalgame/cache/packages-index.toml` fresh for 30 minutes
(mtime via `date -r <file> +%s`). Network failure during refresh
falls back to serving the stale cache with a warning.

═══════════════════════════════════════════════════════════════
  Packages-index (separate repo)
═══════════════════════════════════════════════════════════════

`github.com/amalgame-lang/packages-index` is the curated SoT for
shortname resolution + `amc package search`. Schema v2:

```toml
schema-version = 1

[[package]]
name        = "duckdb"
url         = "github.com/amalgame-lang/amalgame-database-duckdb"
description = "DuckDB binding — vendored C++ amalgamation (MIT)…"
tier        = "official"          # or "listed" for community
maintainer  = "amalgame-lang"
license     = "Apache-2.0"
category    = "database"

# One [[version]] block per (shortname, tag) — flat, linked by
# `package` field. Append newest-last; amc walks the array and
# uses the last-seen compatible tag for auto-resolve + search
# "latest compatible" marker.
[[version]]
package           = "duckdb"
tag               = "v0.1.0"
required-amalgame = ">=0.5.3"

[[version]]
package           = "duckdb"
tag               = "v0.1.1"
required-amalgame = ">=0.5.4"
```

**Automated maintenance**: each package repo has a
`.github/workflows/index-pr.yml` that fires on tag push, reads
the manifest's `required-amalgame`, and opens a PR on
packages-index adding the new `[[version]]` block. Validated
in prod on the v0.2.2 SQLite release. Needs the
`PACKAGES_INDEX_PAT` repo secret (fine-grained PAT scoped to
packages-index, Contents + PRs = write).

Current registered entries:
- **sqlite** — v0.2.0, v0.2.2 (v0.2.1 skipped, never registered)
- **redis** — v0.2.0
- **mqtt** — v0.2.0
- **duckdb** — v0.1.0, v0.1.1

═══════════════════════════════════════════════════════════════
  External packages — all 4 live
═══════════════════════════════════════════════════════════════

**Amalgame.Database.SQLite** — `amalgame-database-sqlite` v0.2.2
- Vendored SQLite 3 amalgamation (public-domain)
- `precompile = true` (v0.2.1+ requires amc ≥ 0.5.4)
- Surface: Open/Close/IsOpen/LastError/Exec/QueryAll/LastInsertId/Changes
- cflags = "-DSQLITE_THREADSAFE=1 -DSQLITE_ENABLE_FTS5"

**Amalgame.Database.NoSQL.Redis** — `amalgame-database-nosql-redis` v0.2.0
- Pure RESP2 over `Amalgame_Net.h` sockets (no vendored C)
- Surface: Open/Close/IsOpen/LastError/Ping/Set/Get/Del/Exists/Incr/Decr/Expire
- Works against Redis / KeyDB / Dragonfly / Valkey

**Amalgame.Messaging.MQTT** — `amalgame-messaging-mqtt` v0.2.0
- Pure MQTT 3.1.1 over TCP, no vendored C
- QoS 0 in v1

**Amalgame.Database.DuckDB** — `amalgame-database-duckdb` v0.1.1
- Vendored DuckDB v1.5.2 C++ amalgamation (MIT)
- `precompile = true` (requires amc ≥ 0.5.4)
- cxxflags = "-O2 -DNDEBUG -std=c++17 -Wno-unused-parameter …"
- libs = ["stdc++"]
- Two-stage link: gcc -c on the cgen-emitted .c (amc emits
  C-style void* casts g++ rejects), g++ on the resulting .o
  + duckdb.cpp.o (needs libstdc++ + C++ static-init order)
- DuckDB bare amalgamation doesn't include `core_functions`
  extension (SUM / AVG / STDDEV). `Open()` enables
  `autoinstall_known_extensions=1` + `autoload_known_extensions=1`
  so first query needing those downloads + caches the extension
  from extensions.duckdb.org. Offline-only: stick to COUNT(*) /
  MIN / MAX which are parser built-ins.

═══════════════════════════════════════════════════════════════
  Standard library (bundled with amc, no manifest needed)
═══════════════════════════════════════════════════════════════

Core (since v0.3.x): Console, File, Path (flat fn API), Math,
String, List<T> / Map<K,V> / Set<T>, Http + HttpResponse,
Tcp {Server,Client}, Udp, Args, Exit, Process (Run + RunCapture),
Env.

Namespace-facade stdlib (each under `namespace Amalgame.<Module>`):

- **Amalgame.Formats.Json** (v0.5.0) — schemaless JsonValue tree
- **Amalgame.Formats.Toml** (v0.5.0) — TOML 1.0 subset
- **Amalgame.Random** (v0.4.4) — PCG-32 + crypto entropy
- **Amalgame.Encoding** (v0.4.4) — Base64 / Hex / percent-encode
- **Amalgame.DateTime** (v0.4.4) — Instant, Duration, Stopwatch
- **Amalgame.Crypto** (v0.4.4) — SHA-256 + HMAC-SHA-256
- **Amalgame.Path** (v0.4.11) — Combine/Filename/Directory/Stem/
  IsAbsolute/Normalize/Sep
- **Amalgame.Logging** (v0.4.12) — leveled stderr + optional file
- **Amalgame.Service** (v0.4.13) — long-running daemon primitives

The core-stdlib classes that lower to flat `Class_Method` symbols
live in a small hardcoded list inside the cgen
(src/generator/c_gen.am around line 3215): Console, File, Math,
String, List, Env, Process, Log, Service. **External package
classes are not in that list** — they go through the
namespace-mangled path resolved by `PackageRegistry`.

═══════════════════════════════════════════════════════════════
  LSP — what's in (unchanged this session)
═══════════════════════════════════════════════════════════════

`amc lsp` is a workspace-aware LSP 3.x server over stdio
JSON-RPC. Capabilities advertised:

- textDocumentSync: 1 (Full), hover, completion, definition,
  declaration, typeDefinition, documentSymbol, workspaceSymbol,
  references, rename (with prepare), callHierarchy, inlayHint,
  codeAction, foldingRange.

5 slices closed (v0.3.4 → v0.4.17). Next on backlog:
tighter selectionRange via parser nameStart hook, more code
actions wired to linter/typechecker.

═══════════════════════════════════════════════════════════════
  `amc test` runner
═══════════════════════════════════════════════════════════════

Two big upgrades from this session:
1. **C/C++ dispatch** (v0.5.3) — `PreCompilePackageSources`
   switches gcc/g++ on file extension. RunTest two-stages the
   link when any package has C++ sources (gcc -c the test.c
   → g++ link with .o + libs).
2. **Persistent cache lookup** (v0.5.4) — RunTest looks at
   `<pkg-dir>/build/<platform>/` before falling back to /tmp.

`amcRuntime` is resolved once up front:
  1. `$AMC_RUNTIME` env var if set
  2. else `<dirname(amc)>/runtime`
  3. else `./runtime` (legacy in-tree path)

═══════════════════════════════════════════════════════════════
  Authorship + contribution policy
═══════════════════════════════════════════════════════════════

- **Bastien Mouget is the sole author and copyright holder**.
  All work is Apache-2.0 licensed.
- **External contributions are paused**. Bug reports open; forks
  allowed per Apache-2.0.
- Auto-close hook on PRs from forks (internal branches
  release/*, feat/*, docs/*, chore/* are skipped).
- **No `Co-Authored-By: Claude …` trailers in any new commit**.
- Third-party licence audit in NOTICE.md.

═══════════════════════════════════════════════════════════════
  Release flow (gitflow + tools/release.sh)
═══════════════════════════════════════════════════════════════

Single source of truth = `PackageRegistry.AmcVersion()` in
src/package_registry.am.

Manual flow (used 5× this session):

    git checkout -b release/vX.Y.Z
    # edit src/package_registry.am AmcVersion(), README.md,
    # ROADMAP_COMPLET.md, CHANGELOG.md (move Unreleased → [vX.Y.Z])
    ./build_amc.sh && ./tests/run_all_tests.sh && ./tools/save-snapshot.sh
    git add … && git commit -m "release: vX.Y.Z" && git push
    gh pr create --base develop … && gh pr merge --squash --admin
    gh pr create --base main --head develop … && gh pr merge --merge --admin
    git checkout main && git tag -a vX.Y.Z -m … && git push origin vX.Y.Z
    # release.yml builds + publishes the 4 artefacts
    git checkout develop && git merge origin/main --ff-only && git push

tools/release.sh automates this but has an interactive `read`
prompt (line 175) — needs `yes y | ./tools/release.sh X.Y.Z` to
work non-interactively. Manual flow is what I use in practice.

═══════════════════════════════════════════════════════════════
  Memory feedback (claude.ai/code, per-project)
═══════════════════════════════════════════════════════════════

Saved feedbacks under
~/.claude/projects/-home-neitsab-D-veloppement-Amalgame/memory/:

- **feedback_gitflow.md** — features sur develop, jamais commit
  direct sur main/develop.
- **feedback_language.md** — répondre en français dans le chat
  (code/commits restent en anglais).
- **feedback_autonomous_edits.md** — exécuter Edit/Write/Bash
  sans "veux-tu que je..." une fois le plan validé.
- **feedback_no_coauthor_trailer.md** — plus de
  `Co-Authored-By: Claude …` dans les commits (projet
  potentiellement revendable, AI = outil pas co-auteur).

═══════════════════════════════════════════════════════════════
  Known gotchas / sharp edges
═══════════════════════════════════════════════════════════════

1. **Bootstrap chicken-and-egg when adding a new runtime header**
   — snapshot's amc doesn't know new symbols. Workarounds:
   temporary `-include` flag in gen_test, or one-line shim header
   at OLD name. Both retired after first snapshot save.

2. **Nested generics in `let` annotations don't parse** —
   `let xs: List<List<string>> = …` rejected. Drop outer
   annotation (cgen infers AmalgameList*) and annotate inner:
   `let row: List<string> = xs.Get(0)`.

3. **Cross-namespace static-call return-type inference** — cgen's
   isStdlib short-circuit skips MethodRetRawSet/Get for
   short-syntax stdlib calls. So `let rows = SQLite.QueryAll(...)`
   doesn't carry `List<List<string>>` raw type. User annotates
   inner list explicitly.

4. **`amc` doesn't auto-link non-test builds** — `amc -o foo
   bar.am` emits `foo.c` only. `amc test` does the gcc step
   internally + splices vendored .o objects. For `amc -o`
   workflows, gcc by hand.

5. **MemberTable.Set silent-no-op on duplicate** — important
   resolver invariant from v0.4.7. Don't change to error.

6. **CGen-precedence bug on `(A || B) && C`** — cgen re-
   associates to `A || (B && C)` in the C output. Workaround:
   split kind+name check across multiple `if` statements.

7. **String-interpolation conflict with `${VAR:-default}`** —
   collides when emitting shell-script templates. Sentinel
   workaround: `let lb = "{"; let rb = "}"; …`.

8. **Curly braces in string templates** — escape `{` and `}` if
   literal contains them.

9. **gen_test linking and libcurl** — adding a runtime symbol
   that pulls in curl means gen_test now needs `-lcurl` too
   (build_amc.sh handles it since v0.4.0).

10. **Bump the version constant BEFORE every tag** — single
    source of truth is `PackageRegistry.AmcVersion()`.

11. **One-time post-clone setup for `merge=ours`** —
    `.gitattributes` declares `merge=ours` for
    `snapshot/amc_lib.c`, `snapshot/INFO.md`, and `src/amc_lib.c`.
    Run once: `git config merge.ours.driver true`.

12. **C++ packages use a two-stage link** — gcc -c on the
    cgen-emitted .c (amc emits C-style `void*` casts that g++
    rejects with `-fpermissive` warnings as errors), g++ on the
    resulting .o + the .o cache. Implemented in `RunTest` since
    v0.5.3.

13. **`Path.X` syntax doesn't work in non-Path namespaces** —
    pre-existing bug. `Path` is NOT in the cgen's
    `isCoreStdlib` list (because its facade method names like
    `Directory` map to runtime `Path_GetDirectory`, the rule
    "Class.Method → Class_Method" breaks). Workaround: call
    runtime symbols directly (`Path_Combine(a, b)` instead of
    `Path.Combine(a, b)`).

14. **DuckDB bare amalgamation missing aggregates** — SUM /
    AVG / STDDEV live in the `core_functions` extension. The
    wrapper enables auto-install but needs network on first
    call. Offline: stick to COUNT(*) / MIN / MAX.

15. **PAT exposure in chat** — fine-grained PAT for
    PACKAGES_INDEX_PAT must never be pasted in the chat.
    Always add via GitHub web UI directly to repo secrets.
    If exposed, revoke immediately at
    https://github.com/settings/tokens?type=beta.

═══════════════════════════════════════════════════════════════
  Roadmap snapshot (next-up after v0.6.0)
═══════════════════════════════════════════════════════════════

Backlog quick-wins (not yet picked up):

- TTL TOML scheme — switch from shell `date -r` to a
  `File_Mtime` runtime helper for true cross-platform support
  (Windows native cmd.exe doesn't have GNU date).
- `amc package outdated` — list installed deps that have newer
  indexed versions matching their constraint.
- `amc package info <pkg>` — full details on one package.
- `amc package notice` — aggregate license info for installed
  deps (commercial downstream).
- `amc package check --frozen` — CI fail-fast if lock doesn't
  match installed.
- `amc --version` enriched with git rev + build date.
- Yanking support: `yanked = true` in [[version]] blocks.
- Transitive deps + cycle detection.
- Path deps (`{ path = "../foo" }`).
- `amc package vendor` (commit cache into repo for offline
  reproducible builds).
- Help text audit pass (some subcommand --help still drift).

External-package backlog:

- **DuckDB v0.2.x** — prepared statements, typed accessors,
  Parquet helpers, transactions.
- **PostgreSQL** — dynamic-link libpq, first dynamic-dep
  package (exercises a new manifest pattern).
- **MySQL / MariaDB** — same shape as Postgres.
- **MongoDB** — libmongoc + libbson, C-API binding.
- **NATS Core** — pure-Amalgame protocol (~250 LoC).
- **DuckDB v1.x** — vendor `core_functions` extension so SUM /
  AVG work offline out of the box.

Compiler / tooling backlog:

- **Multi-version coexistence** — dual-link, package A wants
  redis@v1 + B wants redis@v2.
- **`Amalgame.Service` v2** — native Windows SCM dispatcher,
  drops NSSM dep.
- **`amc new --template service` v2** — macOS launchd.
- **LSP slice 6** — tighter selectionRange + more code actions.
- **ORM layer** — sits above the SQL backend packages.
- **`Path.X` shorthand fix** — restructure cgen's stdlib
  dispatch to allow facade-name divergence (Path.Directory →
  Path_GetDirectory), not just direct mapping.

═══════════════════════════════════════════════════════════════
  Repo layout
═══════════════════════════════════════════════════════════════

  amc                              ← built binary (gitignored)
  build_amc.sh                     ← 3-step bootstrap
  README.md                        ← project intro, install/run
  CHANGELOG.md                     ← per-release detail
  NOTICE.md                        ← authorship + 3rd-party audit
  CONTRIBUTING.md                  ← external PRs paused
  LICENSE                          ← Apache-2.0
  ROADMAP_COMPLET.md               ← what's next
  CONTINUATION.md                  ← this file
  .gitattributes                   ← merge=ours + linguist
  .github/workflows/               ← ci.yml, release.yml,
                                     release-pdf.yml,
                                     auto-close-external-prs.yml
  docs/
    guide/                         ← user-facing 8-chapter book
    language/                      ← grammar.ebnf + grammar.md
    changelog/                     ← per-version PDF builds
    proposals/                     ← design docs
    DEVELOPER_GUIDE.md
  editors/vscode/                  ← extension.js + grammar
  runtime/                         ← _runtime.h + Amalgame_*.h
  snapshot/
    amc_lib.c                      ← portable bootstrap source
    amc                            ← compiled snapshot (gitignored)
    INFO.md                        ← provenance
  src/
    main.am                        ← CLI entry, RunTest + dispatch
    amc_lib.c                      ← generated; merge=ours
    package_registry.am            ← PackageRegistry + LoadedPackage
                                     + Calibration + AmalgameHome
                                     + VersionSatisfies operators
    add_cmd.am                     ← all `amc package <action>`
                                     verbs incl. auto-resolve
    lexer/, parser/, resolver/,
    generator/, typechecker.am,
    linter.am, lsp.am, formatter/  ← compiler internals
    stdlib/                        ← facades for Json/Toml/Random/
                                     Encoding/DateTime/Crypto/Path/
                                     Logging/Service
    migrate.am / generate.am /
    explain.am / new_cmd.am
  tests/
    run_all_tests.sh
    run_tests.sh
    run_stdlib_tests.sh
    run_fmt_tests.sh
    run_amc_new_tests.sh
    fixtures/pm/                   ← package-manager test fixtures
    samples/                       ← .am test inputs
  tools/
    save-snapshot.sh
    release.sh                     ← end-to-end release flow

═══════════════════════════════════════════════════════════════
  TL;DR for the new session
═══════════════════════════════════════════════════════════════

Pick up from **v0.6.0**. develop and main both at v0.6.0, synced,
working tree clean. All five overnight tags published (v0.5.3 →
v0.6.0). `~/.local/bin/amc` is the user-installed copy.

Big shifts since the last CONTINUATION.md (v0.5.2):

1. **C++ packages pipeline** (v0.5.3) — `[stdlib].sources` accepts
   .cpp, manifest gains `cflags`/`cxxflags`/`libs`/`schema-version`.
   Auto-link in `amc test`. First user: DuckDB.

2. **Precompile-on-install + calibration** (v0.5.4) — heavy C/C++
   packages opt in via `[stdlib].precompile = true`; install
   pays the compile cost once into a persistent platform-tagged
   cache. `~/.amalgame/calibration.toml` auto-learns compile
   speed on the machine. Cross-platform `$HOME` resolution
   (POSIX + Windows native).

3. **Search/versions with compat** (v0.5.5) — `amc package search`
   and `amc package versions <pkg>` show all indexed tags with
   ✓/✗ compat marker. `list` shows pinned version. `remove`
   accepts `@<tag>` safety suffix.

4. **Index cache TTL + runner fixes** (v0.5.6) — 30-min auto-
   refresh on the index cache (was forever-cached). Redis +
   MQTT runners fixed (were using obsolete `amc add` syntax,
   silently SKIPping every test since v0.5.1).

5. **Auto-resolve + semver operators** (v0.6.0) — `amc package
   add <shortname>` (no tag) picks the latest compatible from
   the index. `required-amalgame` accepts ^/~/>=/>/<=/</= as
   well as bare versions.

6. **packages-index schema v2** — top-level `[[version]]` array
   with `required-amalgame` per tag. Auto-update via
   `.github/workflows/index-pr.yml` in each package repo on tag
   push (validated in prod on sqlite v0.2.2).

Memory feedbacks still apply by default: répondre en français /
pas de Co-Authored-By trailer / édits autonomes après plan
validé / features sur develop.

The most natural next directions:

1. **5e external package** — PostgreSQL (libpq dynamic-link,
   first to exercise the dynamic-dep manifest pattern) or
   DuckDB v0.2 (prepared statements + typed accessors).

2. **`amc package outdated`** — quick-win observability verb.
   Reads lock + index, lists deps where a newer compatible
   version is available.

3. **`File_Mtime` in runtime** — replaces shell `date -r` in
   `IndexCacheIsFresh`, makes TTL truly cross-platform
   (Windows native cmd.exe lacks GNU date).

4. **Yanking** — `yanked = true` in [[version]] blocks, search
   surfaces a ⚠ marker, `add` refuses unless `--allow-yanked`.

5. **CI automation refinement** — first auto-PR shipped, but
   the workflow could become more idiomatic (e.g. group
   multiple version pushes if they happen close together,
   add a `--dry-run` mode).

Ask me which direction before diving in.
```
