# Vala bootstrap compiler (archived)

This directory contains the original Vala-based Amalgame compiler used to
bootstrap the language. As of May 2026 the project is **self-hosted**: the
active compiler `amc` is written in Amalgame itself, and is rebuilt by
`./build_amc.sh` from `src/amalgame/`.

The Vala sources are kept here as a **cold-start recovery path**: if the
self-hosted compiler ever produces a non-functional `amc` binary, you can
rebuild a working compiler from these sources via:

```bash
./compile.sh    # produces ./build/amc (Vala) via meson
```

`build_amc.sh` automatically falls back to `./build/amc` when `./amc`
doesn't exist (cold start), then switches back to the self-hosted compiler
once the bootstrap is complete.

## Layout

- `src/core/`              — Lexer, parser, analyzer (resolver, typechecker)
- `src/transpiler/`        — Vala C-generator + diagnostics
- `src/pkg/`               — Package manager
- `src/main.vala`          — CLI entry point

The active runtime headers consumed by both compilers live at the project
root in `src/transpiler/runtime/` (`_runtime.h`, `Amalgame_String.h`, etc.)
and are NOT archived.
