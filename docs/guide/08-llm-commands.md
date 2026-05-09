# 8 · LLM-driven commands

Three commands let you collaborate with an LLM on Amalgame code:

| Command | Direction |
|---|---|
| **`amc migrate <file\|dir>`** | other language → Amalgame |
| **`amc generate "<prompt>"`** | natural language → Amalgame |
| **`amc explain <file.am>`** | Amalgame → natural language |

All three share the same provider stack, the same on-disk
prompt header, the same `--dry-run` cost estimation. They came in
together with the v0.4.0 release.

If you've never run them: skim the **Quick start** and the
**Provider selection** sections, then come back when you need the
detail. The full design rationale is in
[`docs/proposals/amc-migrate.md`](../proposals/amc-migrate.md).

## Quick start

```bash
# Translate one file
amc migrate src/user_service.ts

# Translate a whole directory (writes <stem>.am next to each source)
amc migrate src/

# Generate a fresh program from a prompt
amc generate "an HTTP server with /health and /version routes" -o api.am

# Explain a .am file in plain English
amc explain src/lsp.am

# Same, but in French
amc explain src/lsp.am --lang French
```

The output of `migrate` and `generate` is automatically validated
with `amc --check`. If it fails, the file is still written so you
can inspect / fix it manually, but the exit code is non-zero.

## Provider selection

Five providers ship out of the box:

| Provider | Backend | Auto-selected when |
|---|---|---|
| `claude` | shells out to the local `claude` CLI (Claude Code) | (default fallback) |
| `claude-api` | `POST api.anthropic.com/v1/messages` | `ANTHROPIC_API_KEY` is set |
| `chatgpt` | `POST api.openai.com/v1/chat/completions` | `OPENAI_API_KEY` is set |
| `gemini` | `POST generativelanguage.googleapis.com/v1beta/...` | `GEMINI_API_KEY` is set |
| `custom` | shells out to `$AMC_CUSTOM_PROVIDER_CMD` (stdin = prompt, stdout = response) | never (opt-in only) |

**Auto-selection precedence**, when `--provider` isn't passed:

1. `ANTHROPIC_API_KEY` → `claude-api`
2. `OPENAI_API_KEY` → `chatgpt`
3. `GEMINI_API_KEY` → `gemini`
4. fallback → `claude` (CLI)

Override explicitly:

```bash
amc migrate file.cs --provider chatgpt --model gpt-4o
amc generate "..." --provider gemini --model gemini-1.5-pro
amc explain file.am --provider custom
```

For `custom`, point at any script that reads a prompt from stdin
and writes the response to stdout:

```bash
export AMC_CUSTOM_PROVIDER_CMD="ollama run codellama"
amc migrate file.go --provider custom
```

## Cost estimation

`--dry-run` reports an estimated per-call cost based on a 4-char-per-token
heuristic and hardcoded model price tables:

```
$ amc migrate src/api.ts --dry-run
[migrate] would migrate: src/api.ts (TypeScript, 240 lines)
[migrate] would write:   src/api.am
[migrate] provider:      claude-api
[migrate] estimated cost: ~6862 in + ~1000 out -> ~$0.05 (claude-sonnet-4-6)
```

CLI shell-out (`claude`, `custom`) reports `free` since the bill is on
your subscription / local model.

## Result cache (`amc migrate` only)

Re-running `amc migrate` on an unchanged source with the same system
prompt skips the LLM call entirely. The cache lives at
`~/.cache/amalgame/migrate/<sha256>.am` and is keyed on
`SHA-256(source + system prompt)`. So:

| Change | Effect |
|---|---|
| Source edited | new hash → re-call LLM |
| `amc` upgraded with new conventions | new hash → re-call |
| `grammar.ebnf` updated | new hash → re-call |
| Nothing changed | cache hit → reuse output |

`--no-cache` bypasses both lookup and store.

## On-disk prompt docs

The system prompt for all three commands embeds Amalgame's grammar
reference (`docs/language/grammar.ebnf`) and language tour
(`docs/guide/02-language-tour.md`) so the LLM has authoritative
syntax. The lookup order:

1. `<exec-dir>/../share/amalgame/docs/...` (Linux/macOS install
   layout — populated by `install.sh` and the release tarballs)
2. `<exec-dir>/docs/...` (Windows / portable install)
3. `./docs/...` (source checkout / development)

If none of those is reachable, the commands degrade gracefully to
an inline conventions block — still functional, but the model loses
access to the canonical grammar. Run `amc migrate --prompt-only` on
any file to dump the assembled system+user prompt and inspect what
the LLM actually sees.

## Streaming output (`generate` and `explain`)

For long generations, `--stream` flows the response straight to
stdout as it's produced. Constraints (rejected upfront with clean
errors):

- Requires `--provider claude` (CLI). API streaming via SSE is a v3
  follow-up.
- Incompatible with `-o` (no buffered text to write to a file).

```bash
amc generate "implement quicksort with pivot tuning" --stream
```

## Flag matrix

| Flag | migrate | generate | explain |
|---|:---:|:---:|:---:|
| `<positional>` | file or dir | prompt | file |
| `-o / --output` | ✓ (single-file) | ✓ | ✓ |
| `--lang` | ✓ (source language hint) | — | ✓ (output language) |
| `--provider` | ✓ | ✓ | ✓ |
| `--model` | ✓ | ✓ | ✓ |
| `--dry-run` | ✓ | ✓ | ✓ |
| `--prompt-only` | ✓ | ✓ | ✓ |
| `--stream` | — | ✓ | ✓ |
| `--no-check` | ✓ | ✓ (with `-o`) | — (no output validation) |
| `--no-cache` | ✓ | — (no cache) | — |
| `--max-lines` | ✓ (default 2000) | — | — |
| `--force` | ✓ | ✓ | ✓ |

## Known limitations

- API streaming is buffered, not Server-Sent Events. Only the local
  `claude` CLI streams visibly via `--stream`.
- The LLM occasionally produces code that compiles but doesn't match
  the source's intent — always review the migrated file. The
  automated `amc --check` only catches typing errors, not semantic
  drift.
- Source files exceeding `--max-lines` (default 2000) are refused
  with a suggestion to split. Override at your own risk — large
  prompts blow past Anthropic's prompt-cache window and produce
  noticeably worse output.
- `Reduce` lambda signatures still need init-arg type inference —
  the LLM sometimes generates code that runs into this. The
  workaround is documented in the prompt, but if you hit it, fall
  back to a `for-in` with `var acc`.

## Troubleshooting

| Symptom | Likely fix |
|---|---|
| `claude CLI not found on PATH` | Install Claude Code, or use `--provider claude-api` (needs `ANTHROPIC_API_KEY`). |
| `ANTHROPIC_API_KEY not set` | `export ANTHROPIC_API_KEY=sk-ant-...`. Same shape for OPENAI / GEMINI. |
| `provider 'X' not supported` | Typo, or you tried v3 streaming on an API provider. Check `amc migrate --help`. |
| HTTP 401 / 403 from API | Wrong key or expired. Verify with `curl -H "x-api-key: $ANTHROPIC_API_KEY" https://api.anthropic.com/v1/models`. |
| HTTP 429 | Rate limited. Wait, or downgrade `--model`. |
| `check failed (typechecker errors in the migrated file)` | Inspect the `.am` (it was still written). Common cause: an Amalgame limitation the LLM didn't dodge — see "Known limitations" above. |
| Migration is slow | Default model on `claude` CLI is Opus. Pass `--model claude-sonnet-4-6` for ~3× speed-up. |

## Where to look in the code

- [`src/migrate.am`](../../src/migrate.am) — `MigrateCommand`. Hosts
  the provider dispatch table, JSON helpers, cache, cost estimator.
- [`src/generate.am`](../../src/generate.am) — `GenerateCommand`.
  Reuses `MigrateCommand.CallProviderRaw` + `LoadDocsHeader` +
  `AutoSelectProvider`.
- [`src/explain.am`](../../src/explain.am) — `ExplainCommand`. Same
  reuse pattern.
- [`docs/proposals/amc-migrate.md`](../proposals/amc-migrate.md) —
  full design doc with the v0/v1/v2 roadmap.
