# Amalgame — User Guide

A complete reference for users and contributors. If you just want to
write Amalgame, start at chapter 1 and keep going. If you're here to
hack on the compiler itself, jump to chapter 7.

## Table of contents

1. [Getting started](01-getting-started.md) — install, hello world, the
   compile-then-link rhythm.
2. [Language tour](02-language-tour.md) — every feature with runnable
   snippets: types, classes, generics, pattern matching, decorators…
3. [CLI reference](03-cli-reference.md) — every `amc` flag, exit codes,
   typical workflows, debugging tips.
4. [Standard library](04-stdlib.md) — `Console`, `File`, `Path`,
   `IO.FileWatcher`, `Math`, `Math.Vec` (Vec3/Vec4/Mat4), `String`,
   `List`/`Map`/`Set`, `Http`, `Tcp`, `Net.WebSocket`, `Json`,
   `Formats.Yaml`, `Formats.MsgPack`, `Random`, `Encoding`,
   `DateTime`, `Crypto`, `Regex`, `Compress`, `Logging`, `Service`,
   `Database.SQLite`, `Database.DuckDB`.
5. [Runtime & C interop](05-runtime-and-interop.md) — memory model,
   bdwgc, mapping types to C, calling C from Amalgame, inline-C
   blocks (`@c { … }`) since v0.7.4.
6. [Build & tooling](06-build-and-tooling.md) — `build_amc.sh`,
   the snapshot bootstrap, CI, releases, and the
   `amc dap` debugger (v0.8.0+).
7. [Compiler internals](07-internals.md) — how `amc` is structured,
   adding a builtin, adding a syntax form, adding a CGen rule.
8. [LLM commands](08-llm-commands.md) — `amc migrate`, `amc generate`,
   `amc explain`: providers, env vars, caching, design rationale.
9. [`amalgame-ui-web`](09-ui-web/README.md) — desktop GUI binding
   over the OS webview (WebView2 / WKWebView / WebKitGTK). Five
   sub-chapters: getting started, widget catalogue, events +
   partial-DOM updates, layout + theming, escape hatches.
10. [Cookbook](10-cookbook.md) — idiomatic snippets for the patterns
    you'll need every day: strings, collections, files + JSON, CLI
    arg parsing, processes, networking, pattern matching, threads,
    inline-C primitives.

The repo's [README](../../README.md) has the elevator pitch and screenshots.
The [ROADMAP](../../ROADMAP_COMPLET.md) tracks what's planned next.
