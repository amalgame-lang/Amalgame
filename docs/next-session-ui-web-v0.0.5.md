# Next session — kick off `amalgame-ui-web` v0.0.5

> Copy-paste this into a fresh chat in `/home/neitsab/Développement/Amalgame`
> to start the v0.0.5 work. Self-contained context — don't depend on
> the prior chat's history.

## State at end of 2026-05-15 (evening session)

- **amc v0.8.13** tagged on `main` + GitHub Release published. Includes
  the `--template ui-web-form` scaffolder and three post-v0.8.12 fixes
  (manifest `-dev` tolerance, macOS canonical path via
  `_NSGetExecutablePath`+`realpath`, `gdb --dap` fallback after
  `lldb-dap`). 480/480 tests green.
- **amalgame-ui-web v0.0.4** tagged + on packages-index. Form reading
  (Element.Textarea/Select/CheckBox/Radio/Bind + auto-collect bridge),
  OS theming (baseline CSS + 7 variables + `[data-theme=dark]` driven
  by OS detection + Linux `gtk-application-prefer-dark-theme` flip),
  chrome lockdown (no right-click reload, no F5/Ctrl+R), HTML void-tag
  fix, cumulative `Style()`, `Element.Size(w, h)`.
- **`amc package add ui-web`** auto-resolves to v0.0.4 (cache refreshed).
- **15-package ecosystem**, ui-forms sunset, ui-web is the canonical
  desktop-GUI binding going forward.

## What v0.0.5 is

The proposal in `docs/proposals/amalgame-ui-web.md` describes
incremental v0.0.x improvements before the v0.1.0 "native chrome"
milestone. The end-of-session decision (project_ui_pivot.md memory)
was **not** to jump straight to v0.1.0 menubar work — pick the
highest-pain ergonomic gap from real-user feedback and ship that as
v0.0.5.

Three candidates, ordered by impact:

### 1. `Element.OnResult(targetId)` — declarative result wiring  *(~1-2 days, biggest UX gain)*

Today the scaffolded `main.am` ships a verbose in-page JS bridge that
intercepts button clicks, awaits the bound handler, and dumps the
return into `<pre id=out>`. It's ugly and copy-paste prone. Solution:
bake the pattern into the builder.

```amalgame
Element.Button("Submit")
    .OnClick((req: string) => Json.EncodeString("hello, " + req))
    .OnResult("out")   // ← new: handler's return goes into #out
```

Implementation sketch:

- New `Element` field `ResultTargetId: string` (default "").
- `Element.OnResult(id)` setter; returns `this`.
- `Page.RenderElement`: when `e.HasClick && String_Length(e.ResultTargetId) > 0`,
  emit `onclick="window._amc_N(...).then(r => document.getElementById('out').textContent = r)"`
  (with the target id substituted). The existing form-collect call
  stays in place.
- Update `tests/spike_form.am` and the `amc new --template
  ui-web-form` scaffold to drop the manual `win.Init` bridge.

### 2. `Page.AddAsset(path)` — bundle images/fonts alongside CSS  *(~2 days)*

Right now you can pass `Page.AddCss("file:///abs/path/style.css")`
but there's no idiomatic way to ship images, fonts, or icons that
the CSS references. v0.0.4 deferred this. Likely shape:

```amalgame
Page.New()
    .AddAsset("file:///abs/assets/logo.png")    // copied / linked into the webview
```

Open design: do we copy assets into a runtime-managed dir or just
let the user pass `file://` URLs directly? The latter is what we
ship today; AddAsset would be sugar that pre-resolves the path.

### 3. HiDPI sanity + `Window.SetIcon(path)`  *(~1-2 days)*

Today the spike windows look fine on regular DPI but no smoke test
on 2x scaling. Add:
- A `Window.SetContentScale(f: float)` (or equivalent) that propagates
  to webview's underlying scale config.
- `Window.SetIcon(path: string)` — wires through webview's native
  icon API on each OS.

This is small but high "polish per hour" — apps without a taskbar
icon look unfinished.

## Recommended scope for this session

**Pick #1 (`OnResult`)** unless there's been new user feedback pushing
toward another item. It's the smallest change with the largest
visible-UX win, and it makes the scaffold cleaner — which gets the
biggest leverage on every future ui-web app.

Total effort estimate: ~3-5 hours of focused work including:
- Element/Page edits in `facade.am`.
- Update `tests/spike_form.am` and `tests/dump_html.am`.
- Update the `amc new --template ui-web-form` generator in
  `src/new_cmd.am` so freshly-scaffolded projects use the new API.
- README + CHANGELOG bump.
- Release ui-web v0.0.5 (tag + packages-index PR).
- Then release amc v0.8.14 with the updated scaffolder (delta: just
  the new_cmd.am change).

If time runs low, ship `OnResult` in v0.0.5 and defer the scaffold
update + amc release to a follow-up session.

## Git workflow

ui-web has no `develop` branch — feature-branch → PR to `main` → tag.
amc uses gitflow (develop → release/x.y.z → develop+main → tag).

```sh
# ui-web side
cd ~/Développement/amalgame-ui-web
git checkout main && git pull
git checkout -b feat/v0.0.5-onresult
# … work, commit, push, PR, merge, tag v0.0.5 …
./tools/register-package.sh ui-web v0.0.5   # PR on packages-index, then merge

# amc side (only if shipping the scaffold update)
cd ~/Développement/Amalgame
git checkout develop && git pull
git checkout -b feat/scaffolder-onresult
# … edit src/new_cmd.am, commit, push, PR to develop, merge …
# … then release flow for v0.8.14 (see CONTINUATION.md "Release flow") …
```

## Files to skim before starting

- `~/Développement/amalgame-ui-web/facade.am` (lines ~280-475 — `Element`
  + `Page` classes, where the new method lives).
- `~/Développement/amalgame-ui-web/tests/spike_form.am` — current manual
  click-bridge JS lives here. Replace with `.OnResult("out")`.
- `~/Développement/Amalgame/src/new_cmd.am` — `MainAmUiWebForm` generator
  (around line 1265 after the v0.8.13 changes).
- `~/Développement/Amalgame/docs/proposals/amalgame-ui-web.md` — v0.0.x →
  v1.0 capability roadmap, for context on where this fits.
- `CONTINUATION.md` (top section, "evening 2026-05-15") — fresh snapshot
  of session state.

## Standardization deferred

End of last session the user surfaced inconsistencies in the API
verbs (`SetText` vs `Style` vs `Bind` vs `Size` etc.) and asked to
stay on the current pattern for now — full standardization is a v0.1.0
breaking-change candidate. Don't refactor in v0.0.5.

## Bigger picture (don't lose sight)

The headline v0.1.0 work is native menubar on the three OSes
(Win32 + NSMenu + GtkMenuBar, ~6 weeks × 3 OS spread across v0.0.5 → v0.0.7
or a single big v0.1.0 push). Decide after a few v0.0.x iterations
whether real users hit this gap or whether the webview-only surface is
already enough for productive apps.
