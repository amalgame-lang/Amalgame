# Proposal: `amc migrate` — LLM-assisted source-to-Amalgame

**Status:** v1+v2 shipped in v0.4.0 (2026-05-09). v3 partial: real
usage-stat cost reporting shipped (PR pending). API streaming via
SSE still deferred.
**Author:** v0.4 cycle
**Tracking PRs:** #149 (v0), #158 (v1.1), #161 (v1.2), #162 (v1.3),
#164 (`amc generate`), #165 (`amc explain`), #166 (v2 providers),
#167 (cost estimation), #169 (--stream), #170 (result cache).

## Problem

A developer who learns about Amalgame today has no easy way to try it
on their existing code. They must rewrite a file by hand, looking up
the syntax in the guide for every construct. The cost of "kicking
the tires" is high, so most curious devs bounce.

For project-scale adoption, the cost is even higher: a 50-file
codebase means 50 manual rewrites with no automated assistance.

## Goal

Ship `amc migrate <path>` that takes source code in any popular
language and emits idiomatic Amalgame next to it. The migration must
preserve semantics, follow Amalgame conventions (gleaned from the
existing guide + samples), and explicitly mark constructs that have
no clean Amalgame equivalent.

### Non-goals (v1)

- **Bidirectional translation** (Amalgame → other) — out of scope.
- **Round-trip guarantees** — the output is a starting point a
  human will polish, not a drop-in replacement.
- **AST-level deterministic transpilation** — we explicitly bet on
  LLM heuristics rather than building parsers per source language.
- **Build-system integration** — generating CMake, package.json
  equivalents, etc. is left to the developer for now.
- **Automated semantic verification** — the developer is responsible
  for testing the output against their existing test suite. Future
  versions may add a "round-trip via test runner" check.

## High-level design

`amc migrate` is a thin CLI wrapper that:

1. Detects the source language from the file extension.
2. Loads a prompt template tailored to that language.
3. Builds a context payload that bundles:
   - The Amalgame language reference (`grammar.ebnf` + relevant
     guide chapters)
   - Canonical Amalgame samples for common patterns
   - The source file content
4. Shells out to the local `claude` CLI with the assembled prompt.
5. Writes Claude's response to `<file-stem>.am` next to the input
   (or to `--output`-specified path).

We deliberately do **not** ship a parser per source language. The LLM
reads the source directly and produces idiomatic Amalgame in one
shot. The source-side complexity (Python decorators, Java generics
erasure, TS conditional types, etc.) is the LLM's problem, not ours.

### Why shell out to `claude` CLI

- **No API key management in `amc`** — the user's existing Claude
  Code subscription is reused.
- **No vendor SDK in the bootstrap** — `amc` stays
  dependency-light. The runtime requirement is just "the `claude`
  binary is on PATH."
- **Prompt caching is automatic** — the Claude API caches the
  static prefix (Amalgame doc) across calls within a session, so
  batch migrations amortize the doc payload.

The trade-off is a **hard dependency on Claude Code** being
installed. We document this prominently and gate `amc migrate`
behind a runtime check that produces an actionable error if `claude`
isn't found (with a link to the install page).

## Detailed design

### CLI UX

```
amc migrate <path> [--output <out>] [--dry-run] [--model <id>]
                   [--prompt-only] [--lang <hint>] [--provider <name>]
                   [--no-check] [--max-lines <n>] [--force]
```

- `<path>`: file or directory. If a directory, recursively migrates
  every recognized source file.
- `--output <out>`: when input is a single file, write to `<out>`
  instead of `<stem>.am`. Ignored for directory input (writes
  in-place next to each source).
- `--dry-run`: print what would be migrated without invoking the
  provider. Useful for inspecting the recursion before paying.
- `--model <id>`: pass through to the provider's `--model` flag.
  Defaults to whatever the provider defaults to.
- `--prompt-only`: emit the assembled prompt to stdout and exit
  without calling the provider. For debugging the prompt template.
- `--lang <hint>`: override extension-based language detection.
  Useful for files with unusual extensions or shebang-only scripts.
- `--provider <name>`: pick the AI backend (default: `claude`,
  see "Provider extensibility" below).
- `--no-check`: skip the post-migration `amc --check` validation
  (otherwise on by default).
- `--max-lines <n>`: override the per-file source cap (default
  2000 lines / ~80 KB).
- `--force`: overwrite existing `.am` files without prompting.

Single-file example:

```
$ amc migrate src/user_service.ts
[migrate] detected language: TypeScript
[migrate] prompt: 18 KB (16 KB cached + 2 KB source)
[migrate] calling claude...
[migrate] wrote src/user_service.am (312 lines, 4 TODO markers)
```

Directory example:

```
$ amc migrate src/
[migrate] discovering source files in src/...
[migrate] found 12 files: 8 .ts, 3 .py, 1 .go
[migrate] processing src/auth.ts ...                 [ok]
[migrate] processing src/user.ts ...                 [ok]
[migrate] processing src/legacy/decorators.py ...    [ok 2 TODOs]
...
[migrate] summary: 12/12 succeeded, 7 TODO markers across 4 files
```

### Language detection

Extension → source-language label table:

| Extension | Language                |
|-----------|-------------------------|
| `.ts`, `.tsx`     | TypeScript      |
| `.js`, `.jsx`, `.mjs` | JavaScript  |
| `.py`             | Python          |
| `.java`           | Java            |
| `.cs`             | C#              |
| `.go`             | Go              |
| `.rs`             | Rust            |
| `.cpp`, `.cc`, `.cxx`, `.hpp`, `.h++` | C++ |
| `.c`, `.h`        | C               |
| `.kt`, `.kts`     | Kotlin          |
| `.swift`          | Swift           |
| `.rb`             | Ruby            |
| `.php`            | PHP             |

For unknown extensions, `amc migrate` errors out with a message
suggesting `--lang <hint>`. Files matching no entry are skipped (not
errored) during directory recursion to keep batch runs from
exploding on `.md` / `.json` siblings.

The label is **only** a hint to the LLM and a slot in the prompt
template — there is no parser dispatch behind it.

### Prompt structure

The prompt has three layers, ordered for prompt-cache hits on the
prefix:

```
[STATIC HEADER — cached across calls]
  - Project introduction (one paragraph)
  - Amalgame language reference (grammar.ebnf, ~200 lines)
  - Idioms & conventions (excerpts from docs/guide chapters 2-6)
  - 6-10 canonical samples (one per common pattern: class with
    fields, generic List operations, match expression, Result-style
    error handling, file IO, collection comprehension)
  - Known limitations (lambda v2.5 caveats, string interpolation
    propagation, etc. — sourced from CONTINUATION.md)

[LANGUAGE-SPECIFIC HEADER — cached per-language]
  - "You are translating <LANGUAGE> source code to Amalgame."
  - Mapping cheatsheet for that language pair (e.g. for TS:
    "TypeScript `interface` with method bodies → Amalgame
    `interface` (interfaces are method-only); state on a TS
    interface gets lifted to a class")
  - List of constructs that have no Amalgame equivalent and how
    to handle them (insert a `// TODO[migrate]: ...` marker)

[PER-FILE PROMPT — uncached]
  - The source file content, fenced
  - Output instructions: emit only the .am file content, no prose
```

The static header is the same for every file in a session, so
Anthropic's prompt cache (5-min TTL) amortizes its cost from the
second file onward in a batch run. We aim to keep it under 24 KB to
fit comfortably in the cache window.

### TODO markers

When the LLM encounters a construct without a clean Amalgame
mapping, it inserts a comment:

```kotlin
// TODO[migrate]: Python decorators (@dataclass) — Amalgame's
// equivalent is `data class`. Original: @dataclass class User: ...
// Manually verify field order matches the source.
```

The marker prefix is grep-able. `amc migrate` summarizes counts at
end-of-run so the developer knows where to focus manual review.

### Provider extensibility

`amc migrate` ships with **`claude`** as the default and only
provider in v0. The architecture leaves a clean seam for adding
others (ChatGPT, Gemini, local models, etc.) without rewriting the
migration logic.

A provider is identified by a name (`claude`, `chatgpt`, `gemini`,
…) and corresponds to a small dispatch entry that knows how to
invoke its underlying CLI:

```
provider name → command template
─────────────────────────────────
claude        → claude -p [--model <id>] <prompt>
chatgpt       → (v1+) chatgpt -p <prompt>           # or `openai api ...`
gemini        → (v1+) gemini -p <prompt>
custom        → (v2+) user-defined script via env var
```

The dispatch table lives inside `amc` (no external config in v0).
Each provider entry implements one operation: "given an assembled
prompt string, return the raw assistant response." The rest of the
migration pipeline (language detection, prompt assembly, output
writing, `--check` validation, TODO marker counting) is provider-
agnostic.

Selection precedence:
1. `--provider <name>` flag (highest)
2. `AMC_MIGRATE_PROVIDER` env var
3. `claude` (default)

If the selected provider's binary is not on `PATH`, `amc migrate`
errors with an actionable message ("install <provider> or pick a
different one with `--provider`").

In v0, only `claude` (CLI shell out) is implemented; selecting any
other name errors with "provider <name> not yet supported." This
keeps the PR small while the public API (flag + env var + selection
rules) is locked in from day one — adding more entries later is
purely additive, not a breaking change.

#### Provider rollout sequence

The order matters for adoption: ship the cheapest-to-build provider
first (the one we ourselves can use to dogfood), then add the more
distributable ones.

| Version | Provider added              | Mechanism                       | Why this order |
|---------|------------------------------|---------------------------------|----------------|
| v0      | `claude` (CLI shell out)     | `Process.RunCapture` to local `claude` binary | ~30 LoC, validates the whole pipeline. Requires Claude Code installed — fine for the dogfooding phase (the Amalgame author has it). |
| v1      | `claude-api` (Anthropic HTTP) | Direct API call (curl shell out or `Net.Http.Post` from the existing runtime) with `ANTHROPIC_API_KEY` env var | Standalone — works on any machine with an API key. Becomes the default when the env var is set. Unlocks distribution to devs who don't use Claude Code. |
| v2      | `chatgpt`, `gemini`          | Direct API calls per provider, each with its own env var (`OPENAI_API_KEY`, `GEMINI_API_KEY`) | Multi-IA story. Each is a new dispatch entry; no rewrite of the migration pipeline. |
| v2+     | `custom`                     | User-defined script that reads prompt from stdin and writes response to stdout | Power users wire in any LLM CLI (local llama, Mistral, etc.) without patching `amc`. |

The v0 → v1 jump is the meaningful one: it's the moment `amc
migrate` graduates from "tool the author uses on his own machine"
to "tool any developer can run after `cargo install`-ing Amalgame."
Until v1, expect feedback that `amc migrate` is "neat but I can't
try it without installing Claude Code first" — that's an accepted
v0 limitation.

User-extensible providers (custom scripts) intentionally land last
so the built-in providers stabilize the prompt-format contract
before third-party tools start depending on it.

### Output format

- Single file mode: writes `<stem>.am` next to `<file>` (or the
  `--output` path).
- Directory mode: writes each output `.am` next to its source. Does
  **not** delete the source. Existing `.am` files at target paths
  are overwritten with a confirmation prompt (suppressible with
  `--force`).
- A run summary is printed to stdout. Per-file logs go to stderr.
- Exit code: 0 on full success, 1 if any file failed to migrate
  (parse error, provider binary error, write failure, validation
  failure unless `--no-check`).

### Post-migration validation

After writing each `.am` file, `amc migrate` immediately runs
`amc --check <file>.am` against it (unless `--no-check` is passed).
Any errors reported by the checker are tagged on that file's line in
the run summary:

```
[migrate] processing src/auth.ts ...     [ok]
[migrate] processing src/legacy.py ...   [ok 2 TODOs, 1 check err]
```

The validation failure does **not** block the next file in batch
mode — it just bumps the run-summary counter. The output `.am` is
still written so the user can inspect what the LLM produced and
fix it manually. Exit code is non-zero at end-of-run if any file
had a validation error.

This matters because LLMs occasionally produce code that *looks*
right but fails the typechecker (especially around generic type
parameters or visibility modifiers). Catching that immediately,
right at migration time, saves the user a debug session two days
later when they try to compile.

### Source size limit

By default, `amc migrate` refuses files exceeding **2,000 lines or
80 KB**, whichever comes first. The error message suggests splitting
the file or overriding the limit with `--max-lines <n>`.

Rationale:
- Anthropic's prompt cache TTL is 5 minutes and the cache window is
  finite. Stuffing a 5,000-line file into one call inflates the
  prompt past the cache window and burns the cache for the rest of
  the batch.
- Quality also degrades: LLMs handle a single 80 KB file noticeably
  worse than two 40 KB files migrated separately. The hard cap nudges
  users toward a workflow that produces better output.
- The cap is a soft limit (override available), not a hard
  refusal — power users with budget to burn can crank it up.

### Configuration

- `AMC_MIGRATE_MODEL` env var: sets the default model
  (overridden by `--model`).
- No config file in v1 — keep the surface small. If users want to
  customize the prompt template, they can use `--prompt-only` to
  capture it, edit, and pipe back into `claude` themselves.

## Implementation plan

### v0 — proof of concept (this proposal's first PR)

- New file `src/migrate.am`: the migration command, all logic in
  one place initially.
- Wire `migrate` subcommand into `src/main.am` next to `fmt`,
  `test`, `lsp`.
- Hardcoded prompt template inline (no external doc loading yet);
  mention this as a v0 limitation.
- Support single-file input only; directory mode comes in v1.
- Provider dispatch table with `claude` only; `--provider` flag
  + `AMC_MIGRATE_PROVIDER` env var wired so v1 just adds entries.
- Source size cap (`--max-lines`, default 2000) with clean refusal.
- Post-migration `amc --check` validation, suppressible via
  `--no-check`.
- `--dry-run`, `--lang`, `--prompt-only`, `--force` flags.
- One sample test that exercises `amc migrate --prompt-only` on a
  trivial source file and asserts the prompt contains expected
  markers.

### v1 — production-ready

- Load prompt prefix from disk at runtime (read `grammar.ebnf` +
  selected guide chapters from the install location, falling back
  to a packaged copy if the install doesn't include `docs/`).
- Directory recursion + batch progress reporting.
- TODO marker counting + summary.
- **Add `claude-api` provider** (Anthropic HTTP via curl or
  `Net.Http.Post`). When `ANTHROPIC_API_KEY` is set, becomes the
  default — so users without Claude Code installed can finally
  use `amc migrate` out of the box.
- Documented in `docs/guide/` as a new chapter.
- Tests for: language detection, prompt assembly, error paths
  (provider binary not on PATH, file not found, write permission
  denied, missing API key).

### v2 — quality / DX polish

- **Add `chatgpt` and `gemini` providers** as direct API calls
  with their own env vars (`OPENAI_API_KEY`, `GEMINI_API_KEY`).
  Each is one new dispatch table entry — no rewrite of the
  migration pipeline.
- Prompt customization via per-project `amc.migrate.toml`
  (overrides for the language-specific header, extra samples to
  include).
- Round-trip checking: when source has tests, optionally migrate
  them too and report whether the migrated suite passes.
- Package the prompt prefix as a versioned blob so users on older
  Amalgame versions can target newer language versions.

### v2+ — power-user extensibility

- `custom` provider that delegates to a user-defined script.
  Convention: stdin = prompt, stdout = response. Lets users wire
  in local llama / Mistral / any LLM CLI without patching `amc`.
  Defers to last so the built-in providers can stabilize the
  prompt-format contract first.

## Open questions

1. **Multi-file refactors** — when a TypeScript file imports
   types from a sibling, the LLM needs to see the sibling to do a
   good job. v1 sticks to per-file (state limitations explicitly).
   Multi-file context is a v2+ feature, probably tied to a
   `--with-context <pattern>` flag that bundles named siblings into
   the per-file prompt slot.

2. **Cost estimation** — should `--dry-run` print an approximate
   token count + cost estimate? Useful for batch planning. Adds
   nontrivial complexity (need a tokenizer or a heuristic). Defer
   to v1 once the API provider is in (the API returns usage stats
   we can aggregate).

3. **Streaming output** — `claude -p` supports streaming.
   Worth piping it through to `amc migrate`'s stdout? Improves
   perceived latency for large files but complicates the writer.
   Defer to v2.

4. **Caching previous results** — if the user re-runs `amc
   migrate` on a file whose source hasn't changed, should we
   skip and reuse the previous output? Cheap (sha256 of source +
   prompt prefix as cache key), valuable for iterative workflows.
   Probably fold into v1.

## Future work

- **`amc explain <file.am>` reverse path**: explain an Amalgame
  file in natural language. Useful for code review / onboarding.
- **`amc port <file.am> --to typescript`**: Amalgame → other
  language, for embedding Amalgame logic in foreign codebases.
- **VS Code "Migrate this file" command**: same flow, triggered
  from the editor instead of CLI.

## Appendix A: Prompt template sketch

(Not committed as a runnable file yet — included here so the
shape is reviewable in this proposal.)

```
You are translating source code from <LANGUAGE> to Amalgame.
Amalgame is a self-hosted language that transpiles to C. Idiomatic
Amalgame uses class-based OOP with explicit visibility modifiers,
generic collections (List<T>, Map<K,V>, Set<T>), and ML-style
match expressions. Higher-order list operations like .Map / .Filter
take lambdas. Error handling is exception-based.

# Amalgame language reference (grammar)
[~200 lines from docs/language/grammar.ebnf]

# Amalgame conventions
- Files start with `namespace <Name>` then declarations.
- Public classes use `public class Name { public ... }`.
- Data classes (record-like): `public data class User(string Name, int Age)`.
- Lambdas: `(x, y) => x + y` or `x => { let d = x*2; return d+1 }`.
- ...

# Canonical Amalgame samples
[grammar examples for common patterns: see docs/guide/]

# <LANGUAGE>-specific mapping notes
[per-language cheatsheet]

# Known Amalgame limitations
- Lambdas can't return a non-pointer non-int when used outside Map/Filter
  (workaround: extract to a local before yielding).
- String interpolation does not propagate inferred types into embedded calls
  (workaround: stage in named locals before printing).
- ...

# Source file to translate
```<language>
[file content]
```

# Output instructions
Reply with ONLY the Amalgame source code, no prose, no markdown
fences. If you encounter a construct that has no clean Amalgame
equivalent, insert a `// TODO[migrate]: <reason>` comment instead
of best-effort guessing.
```
