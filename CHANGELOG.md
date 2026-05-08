# Changelog

All notable changes to Amalgame are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) loosely.

For releases prior to v0.3.2, see the git log and `ROADMAP_COMPLET.md`.

---

## [v0.3.2] — 2026-05-08

This release closes every open compiler bug tracked in v0.3.1's
`SKIP_SELFHOST` list, restores `try` / `catch` / `throw` / `finally`
end-to-end in the self-hosted parser, ships the first round of
generic type inference for `List<T>` and `Map<K,V>`, and introduces
`amc --lint` (basic static analysis). The full test suite is now
**150/150 green with zero SKIPs**.

> **Re-tagged note**: an earlier v0.3.2 release went out before two
> follow-up changes (typechecker false positives during multi-file
> compilation; `amc --lint` MVP). The v0.3.2 tag and GitHub release
> were re-cut to include them. Binary checksums therefore differ
> from the original v0.3.2 artefacts.

### Language

- **`try` / `catch` / `throw` / `finally`** restored end-to-end in the
  self-hosted compiler (the Vala bootstrap had it; the rewrite hadn't
  grown a `ParseTry` yet). Lowers via `setjmp`/`longjmp` using the
  existing `_am_throw` runtime helpers.
- **`Type.Variant` patterns in `match`** — `Direction.North => …` is
  now recognised as a pattern. `ParseMatchPattern` reads an optional
  `.IDENT` suffix.
- **Generic type inference (locals + params + returns)**:
  `let xs = new List<int>()`, `xs: List<int>`, `xs = MakeList()`,
  and `Foo(List<int> xs)` all carry the elem type, so `xs.Get(i)`
  lowers to `(int)AmalgameList_get(xs, i)` without a manual cast.
  Same for `Map<K,V>.Get(k)` (which wasn't even dispatched before).
- **`obj.Method()` instance syntax for strings** — `s.Length()`,
  `"foo".Trim()`, `s.Replace(a, b)`, … route through `String_*`.

### Compiler

- **Multi-file type checking** — TypeChecker now walks every parsed
  program (was hard-coded to `programs[0]`).
- **Multi-file false-positive cleanup** — the typechecker's
  per-node type map and local-scope state now reset between
  programs, the per-program filename is read from `prog.Str2`
  rather than the constructor argument, and bare `return` (parsed
  as `Return(_unknown_, …)`) is recognised as such instead of
  being treated as a value-return. `build_amc.sh` is now silent.
- **Null-safety** — three independent bugs fixed: NodeKey collision
  in the typechecker (Kind is now part of the key), bare `?` was
  dropped by the lexer (added `OP_QMARK`), and `TypeToC("T?")` no
  longer doubles up the C pointer.

### Tooling

- **`amc --lint`** — new flag that runs a static-analysis pass on
  top of the parsed AST and emits non-fatal warnings. MVP catches
  unreachable code after `return` / `throw` / `break` / `continue`,
  including inside nested `if` / `while` / `for-in` / `try` bodies.
  The skeleton (`src/linter.am`) is set up to grow more checks
  (unused locals, shadowed names, …) without touching anywhere
  else.

### Formatter

- **Same-line comments** preserved on round-trip: `let x = 1  // foo`
  no longer sees its comment bumped to the next line.
- **`import` directives** preserved: the parser used to discard them.
- `try` / `catch` / `throw` round-trip lossless.

### Tests / infra

- Suite is now **149/149** (87 core + 50 stdlib + 12 fmt) under
  `./amc`. **`SKIP_SELFHOST` is empty.**
- `run_multifile_test` shell helper fixed (was passing `-o foo.c`,
  amc dutifully expanded it to `foo.c.c` and never produced an
  executable).
- `tests/run_all_tests.sh` completes end-to-end for the first time
  (its `set -e` no longer trips on a half-failing suite).

[v0.3.2]: https://github.com/BastienMOUGET/Amalgame/releases/tag/v0.3.2
