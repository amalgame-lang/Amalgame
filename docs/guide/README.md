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
   `Math`, `String`, `List`/`Map`/`Set`, `Http`, `Tcp`, `Json`,
   `Random`, `Encoding`, `DateTime`.
5. [Runtime & C interop](05-runtime-and-interop.md) — memory model,
   bdwgc, mapping types to C, calling C from Amalgame.
6. [Build & tooling](06-build-and-tooling.md) — `build_amc.sh`,
   the snapshot bootstrap, CI, releases.
7. [Compiler internals](07-internals.md) — how `amc` is structured,
   adding a builtin, adding a syntax form, adding a CGen rule.
8. [LLM commands](08-llm-commands.md) — `amc migrate`, `amc generate`,
   `amc explain`: providers, env vars, caching, design rationale.

The repo's [README](../../README.md) has the elevator pitch and screenshots.
The [ROADMAP](../../ROADMAP_COMPLET.md) tracks what's planned next.
