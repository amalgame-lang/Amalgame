# Continuation prompt — start a new chat with this

> Last refreshed 2026-05-09 after shipping v0.4.3 (Json migration completes + amc migrate v3).
> The block below is a self-contained prompt designed to bootstrap a
> new Claude session with full context. Copy-paste it as your first
> message in a fresh conversation.

---

```
I'm working on Amalgame, a self-hosted programming language that
transpiles to C. I keep the project in
/home/neitsab/Développement/Amalgame.

Current state (May 2026, v0.4.3):

- The compiler `amc` is written in Amalgame in src/ and compiles
  itself in ~5 seconds via ./build_amc.sh.
- 3-rung bootstrap chain in build_amc.sh:
    ./amc                  → current self-hosted (may break in dev)
    ./snapshot/amc         → last known-good amc, captured by
                             tools/save-snapshot.sh after green tests.
                             snapshot/amc_lib.c is committed.
    ./build/amc            → Vala bootstrap in archive/vala-bootstrap/,
                             no longer in CI but kept locally for cold
                             starts (./compile.sh).
- The runtime headers are at runtime/. Cross-platform (POSIX +
  Windows winsock2 via #ifdef _WIN32 in Amalgame_Net.h). libcurl is
  required by the runtime for the Net module + the new claude-api /
  chatgpt / gemini providers.
- Test runner (./tests/run_all_tests.sh) drives ./amc directly.
  Build artefacts go to /tmp via mktemp; the source tree stays clean.
  Currently 307 PASS / 0 FAIL / 0 SKIP across the core/stdlib/fmt/
  amc-new sub-suites.
- Multi-OS CI (.github/workflows/ci.yml) — Linux + macOS + Windows
  MSYS2. Linux uses snapshot + self-hosted amc; no Vala in the graph.
  CI compiles with -Wint-conversion as an error on macOS/Windows;
  pin int-typed locals via `let n: int = …` when the codegen erases
  the return type to void* across a method-call boundary.
- Releases automated on `v*` tag (.github/workflows/release.yml).
  Latest is v0.4.3 — see CHANGELOG.md for the per-release detail.
  develop → main → tag is the release flow. Both develop and main
  are protected (force-push + delete blocked, PR required, admin
  bypass allowed).
- VS Code extension in editors/vscode/ — TextMate grammar +
  language config + LSP client (vscode-languageclient over stdio).
  Configurable via `amalgame.serverPath` in user settings to point
  at a local amc build.
- Formatter: `amc fmt file.am` re-emits canonical source with
  comments preserved. Idempotent on every compiler source.
- Linter: `amc --lint file.am` runs static analysis: unreachable
  code, unused locals, shadowed names.
- LSP: `amc lsp` over stdio JSON-RPC. v0.3.4 ships diagnostics;
  v0.3.5 adds hover + global completion; v0.3.6 (PR #146) makes
  the LSP workspace-aware — it scans every *.am file under the
  detected workspace root so cross-file types (e.g. NodeKind from
  src/parser/ast.am) resolve in any open file.
- LLM-driven workflows (v0.4.0):
    amc migrate <file|dir>   translate other languages → .am
    amc generate "<prompt>"  write a new .am from prose
    amc explain  <file.am>   read a .am, write prose
  All three share: provider auto-selection from env (ANTHROPIC_API_KEY
  → claude-api, OPENAI_API_KEY → chatgpt, GEMINI_API_KEY → gemini,
  fallback claude CLI), on-disk grammar+tour as system prompt
  (cacheable via Anthropic prompt cache), --dry-run cost estimation,
  --no-check / --force / -o / --lang flags, and (for generate +
  explain) --stream via the claude CLI.
- amc migrate also has: directory recursion, result caching by
  SHA-256(source + system prompt) at ~/.cache/amalgame/migrate/,
  and post-write `amc --check` validation.
- User guide at docs/guide/README.md (chapters 1–8, with chapter
  8 being the new LLM-commands reference).
- Grammar: docs/language/grammar.ebnf mirrors src/parser/parser.am
  exactly. grammar.md is the prose companion.
- README + CHANGELOG at the repo root.
- Design proposals: docs/proposals/amc-migrate.md tracks the v1+v2
  roadmap for the LLM commands (largely shipped now).

Recently shipped (v0.4.3 Json migration completes, 2026-05-09):

  - src/lsp.am request dispatcher swapped from JsonStr/JsonInt
    substring extractors to a single Json.Parse(body) walked via
    typed-local Get(...).At(...) chains.
  - src/migrate.am providers parse actual response shapes:
      Anthropic → root.content[0].text
      OpenAI    → root.choices[0].message.content
      Gemini    → root.candidates[0].content.parts[0].text
  - amc migrate v3 (PR #194): real cost line printed after every
    successful call from each provider's usage object. No `~`,
    exact billed tokens. CLI shell-out and custom keep silent
    (InputTokens == -1).
  - Six dead JSON helpers removed (~150 LoC out).
  - The earlier "Json.Parse hangs on 16 KB bodies" turned out to
    be a probe artefact — bash ${#body} counts characters, not
    bytes, so the Content-Length frame under-counted UTF-8
    multibyte content. Real client traffic uses byte-accurate
    counts and the parser does a 16 KB body in ~37 ms.

Recently shipped (v0.4.2 stdlib + DX release, 2026-05-09):

  Stdlib + tooling:
  - Amalgame.Json (PR #182, #183): first-class JSON parser +
    encoder + accessor surface in the new src/stdlib/ directory.
    Strict RFC 8259 with full escape support including \uXXXX +
    surrogate pairs. JsonValue with Is*/As*/Get/At/Length/Keys.
    Replaces 4 ad-hoc substring extractors in the compiler in a
    future phase 2 PR.
  - amc new <name> (PR #184): project scaffolder à la cargo new.
    Three templates (exe/lib/test) with working build.sh,
    runtime auto-discovery via $AMALGAME_HOME / install dirs /
    `which amc`, READMEs, .gitignore. Path-aware
    (basename = namespace stem).
  - LSP member completion (PR #185): obj.<cursor> narrows to the
    receiver type. Two-step receiver resolution: global-symbol
    lookup, then a local-decl text scan covering
    `let x = new T(...)` / `let x: T = ...`.

  Infra:
  - Repo transferred to amalgame-lang/Amalgame (PR #187).
    GitHub redirects the old URLs ~1 year; canonical references
    landed everywhere.
  - tests/run_amc_new_tests.sh + new 5th-arg `extra_inputs` on
    run_stdlib_tests.sh so per-module tests can pull in stdlib
    sources alongside the sample.

Recently shipped (v0.4.0 LLM-driven release, 2026-05-09):

  Compiler infrastructure (unblocked the LLM-generated code paths):
  - Lambda v2.5 (PR #142): non-int signatures — xs.Map(x => x.Name)
    over List<Class> works. TypeChecker patches the lambda PARAM
    type from the higher-order callsite, CGen emits typed unbox
    + boxed return.
  - LSP workspace-aware (PR #146): scans every *.am under the
    workspace root, eliminating false "Unknown symbol" floods
    on cross-file types (NodeKind, SourceSnippet, etc.).
  - Parser: TS-style `name: type` param syntax accepted (PR #152)
    + multi-line method chains (PR #154), both because LLM output
    leans into those forms.
  - CGen: auto-qualify implicit field access (PR #155) — `Id = id`
    in a method body lowers to `self->Id = id` so users from
    C# / TS / Kotlin can skip the explicit `this.` qualifier.
  - Stdlib: Env.Get/Has builtins + Http_PostWithHeaders return-type
    tracking (PR #160).

  amc migrate / generate / explain (the actual LLM commands):
  - v0 single-file migrate (PR #149)
  - v1.1 directory recursion (PR #158)
  - v1.2 claude-api provider via Anthropic HTTP (PR #161)
  - v1.3 prompt loaded from disk + system/user split for prompt
    caching (PR #162)
  - amc generate (PR #164)
  - amc explain (PR #165)
  - v2 providers: chatgpt, gemini, custom (PR #166)
  - cost estimation in --dry-run (PR #167)
  - --stream for generate/explain via claude CLI passthrough (PR #169)
  - result caching by SHA-256 (PR #170)

  Plus various small DX fixes: --help flags, cleanup of obsolete
  files (use.sh, .github/workflow/ typo dir).

Known limitations:

- Lambda Reduce signatures still need init-arg type inference. If
  you hit issues with .Reduce, fall back to a for-in with `var acc`.
- String interpolation `"x: {coll.Get(i)}"` doesn't propagate the
  inferred type into the embedded call. Workaround: stage in
  named locals before printing.
- ForEach captures by value — `var sum = 0; xs.ForEach(x => sum = sum + x)`
  doesn't accumulate. Use Reduce for accumulation.
- API streaming (claude-api / chatgpt / gemini) is buffered, not
  Server-Sent Events. --stream only flows for the local claude CLI.
- LSP grammar TextMate has a couple of false-positive zones
  (coloration around certain enum.method patterns). Tracked.

Workflow rules:

- gitflow simplified: feature/<name> → develop → main → tag.
  Never commit directly to main or develop. Both branches are
  protected on GitHub.
- Execute git/gh commands directly (don't ask first). Same for
  bash and file edits — the user wants action, not confirmation
  loops. Asking is for design questions ("scope A or B?",
  "which approach?"), not for permission to run a command.
- Destructive ops (git reset --hard, push --force, branch -D,
  tag deletion) still get confirmed first.
- Use `gh pr create --body "$(cat <<'EOF' … EOF)"` for PR bodies.
  The heredoc avoids shell quoting issues. Keep markdown
  reasonable (don't paste raw triple-backtick blocks containing
  special chars).
- Code, commit messages, PR bodies stay in English. Chat replies
  in French.
- TodoWrite for multi-step plans; one-liners can skip it.

Where to head next (from ROADMAP_COMPLET.md):

  1. Stdlib expansion: pick one or two from DateTime / Json /
     Regex / Random / Encoding / Compress / Crypto / Threading.
     Each is 200-400 LoC. Tied to the open "Stdlib delivery
     model" design question (currently header-only).
  2. LSP member completion: `obj.<cursor>` narrowed to the
     receiver's type. ~150 LoC on top of the v0.3.5 global
     completion.
  3. amc new <name> [--template exe|lib|test]: scaffolding
     command à la cargo new / dotnet new. File templates +
     dispatcher branch in main.am + a roundtrip test that
     scaffolds + compiles under /tmp. ~200-400 LoC.
  4. amc migrate v3 follow-ups (deferred): API streaming via
     SSE parser, actual usage stats from API responses (vs the
     heuristic estimate), cost reporting in run summary.
  5. amc test polish: --runtime <path> flag, per-file timeouts,
     parallel execution.
  6. Process v2: split stderr from stdout via real pipes, add
     timeouts, async streaming.

Quick checks before claiming a feature is done:

  ./build_amc.sh
  ./tests/run_all_tests.sh

For non-trivial compiler changes, read docs/guide/07-internals.md
first. The pipeline is single-pass: lex → parse → resolve →
typecheck → (lint?) → cgen.

For changes to the LLM commands, see docs/proposals/amc-migrate.md
(design rationale + roadmap status) and docs/guide/08-llm-commands.md
(user-facing reference).
```

---

## What's actually in the repo right now

For quick recall when reopening the project:

```
src/                    ← Amalgame compiler in Amalgame
├── lexer/                token.am, lexer.am
├── parser/               ast.am, parser.am
├── resolver/             symbol.am, resolver.am
├── generator/            c_gen.am, gen_test.am
├── formatter/            formatter.am          (`amc fmt`)
├── linter.am             static analysis      (`amc --lint`)
├── lsp.am                workspace-aware LSP  (`amc lsp`)
├── migrate.am            LLM source-to-Amalgame    (`amc migrate`)
├── generate.am           LLM prose-to-Amalgame     (`amc generate`)
├── explain.am            LLM Amalgame-to-prose     (`amc explain`)
├── typechecker.am
├── diagnostics.am
├── main.am               (CLI: compile, fmt, test, lsp, --lint, --check,
                                migrate, generate, explain)
└── amc_lib.c             (generated)

runtime/                ← C runtime (bdwgc, strings, IO, collections, net,
                          environment, process). Env_Get/Has aliases
                          + Http_PostWithHeaders return-type tracked
                          since v0.4.0 (PR #160). libcurl required for
                          Net + claude-api / chatgpt / gemini providers.
stdlib/strings.am       ← stdlib API reference (declarations only)
tests/                  ← samples + run_*.sh runners (output in /tmp)
docs/guide/             ← user guide chapters 1–8 (8 = LLM commands)
docs/language/          ← grammar.ebnf + grammar.md
docs/proposals/         ← design proposals (RFC-style); amc-migrate.md
                          tracks the LLM-roadmap status
snapshot/               ← amc_lib.c (committed) + amc binary (gitignored)
                          + INFO.md (provenance)
tools/save-snapshot.sh  ← capture a known-good amc after green tests
archive/vala-bootstrap/ ← original Vala compiler (recovery only)
editors/vscode/         ← VS Code extension (TextMate grammar + LSP client)
.github/workflows/      ← CI + Release automation
install/                ← Homebrew / Inno Setup / install.sh sketches
README.md               ← elevator pitch + tested samples
ROADMAP_COMPLET.md      ← canonical "what's next" board
CHANGELOG.md            ← per-release notes (v0.3.2 onwards)
CONTINUATION.md         ← this file
```

## Memory the assistant has about this project

The session-persistent memory store at
`~/.claude/projects/-home-neitsab-D-veloppement-Amalgame/memory/`
already holds:

- `project_amalgame.md` — project overview
- `reference_build.md` — build/test commands, file roles
- `feedback_gitflow.md` — features always on develop, never on main
- `feedback_autonomous_edits.md` — execute Edit/Write/Bash/git
  directly without per-step permission prompts
- `feedback_language.md` — chat in French; code/commits stay English
- `project_amc_migrate_idea.md` — the original idea log for the
  LLM roadmap (now mostly shipped)

A new session reads these automatically and applies them.

## Common pitfalls when resuming

1. **Bootstrap circularity** — when you add a runtime helper or
   builtin, the running `amc` doesn't know about it until you rebuild.
   `build_amc.sh` step 1 tolerates non-zero exit from `amc` if the
   `.c` file was produced; gcc remains the real correctness gate.
   This bit twice during v0.4 (Env_Get and Http_PostWithHeaders return
   types had to land in their own infrastructure PR before the migrate
   v1.2 PR could even bootstrap).

2. **`void*` erasure across method-call boundaries** — the self-host
   codegen emits `void* x = SomeFunc()` instead of carrying the
   typed return when the call chains through a generic List<T>
   field. Two known fixes:
   (a) pin the local: `let pn: int = this.X.Count()`
   (b) split into a typed helper that takes the AstNode parameter.
   Both patterns are already in use across the compiler. CI on
   macOS/Windows treats -Wint-conversion as an error, so warnings
   ignored locally on Ubuntu will fail the merge.

3. **File order in `AMC_SOURCES`** — `diagnostics.am` must come
   before files that reference `SourceMap` / `SourceSnippet`.
   `main.am` is intentionally **excluded** from the gen_test build
   list because it carries a `Program.Main` that conflicts with
   `gen_test.am`'s own.

4. **Generics still erase to `void*` at the C level** — primitive
   collection elements are boxed via `(void*)(intptr_t)`. The cgen
   tracks elem types for `List<T>` / `Map<K,V>` (locals, params,
   returns, annotations) and emits `(T)…_get(…)` so callers see the
   right type. Lambda v2.5 extends this to xs.Filter / xs.Map results.

5. **`?.` evaluates the receiver twice** — keep the receiver
   side-effect-free, or extract it to a `let` first.

6. **Snapshot vs Vala** — when introducing new syntax, *take a
   snapshot first* (tools/save-snapshot.sh) so the bootstrap chain
   stays usable. The Vala bootstrap can no longer parse much of
   what's in src/, so it's only useful for a from-scratch recovery
   of an old commit, not for ongoing work.

7. **Multi-line string concatenation** — the self-host parser
   chokes on `let s = "a" +\n   "b" +\n   "c"`. Keep `+`-chains on
   a single line, or use `var s = ""; s = s + "a"; s = s + "b"`.
   (Multi-line method chains DO work since PR #154 — only string
   concatenation is still single-line.)

8. **Lambda capture is by value** — `var sum = 0; xs.ForEach(x =>
   sum = sum + x)` won't accumulate; the closure has a copy of sum.
   Use Reduce for accumulation.

9. **Re-tagging a release** — if you need to retag (e.g. fixed CI
   config), delete the existing GitHub Release via the web UI BEFORE
   pushing the new tag. `softprops/action-gh-release@v2` edits
   instead of replacing, leaving stray assets from the old run.
   In practice it's almost always cleaner to bump the patch number.

10. **String interpolation in source code that happens to contain
    examples of itself** — when writing prompts in Amalgame source
    that include `"x={x}"` literally, the lexer interpolates the
    embedded `{x}`. Workaround used in migrate.am / generate.am /
    explain.am: split the literal braces out as their own concat
    pieces (`let lb = "{"; out = out + "..." + lb + "x" + "}"`).

11. **gen_test linking and libcurl** — adding a runtime symbol that
    pulls in curl (e.g. wrapping Http_*) means gen_test now needs
    `-lcurl` too. build_amc.sh links it since v0.4.0 (PR #161). If a
    new runtime call brings in a fresh transitive dep, propagate the
    `-l<lib>` flag here as well.

12. **Bump the `--version` string in `src/main.am` BEFORE every tag** —
    the version is hardcoded in the `--version` flag handler:

        Console.WriteLine("amc <X.Y.Z> (self-hosted Amalgame compiler)")

    This was forgotten on the v0.4.0 push (binary still printed
    `0.3.6` after the tag). Workaround was to bump to v0.4.1 with
    the fix. Pre-tag checklist:

        grep -n "amc 0\." src/main.am   # finds the line
        # → edit to the new version
        ./build_amc.sh
        ./tests/run_all_tests.sh
        ./tools/save-snapshot.sh
        # → commit, PR to develop, develop → main, then tag

    A linter rule could catch this (compare the version string
    against the latest tag) but currently relies on the contributor
    remembering — which means it WILL be forgotten again unless
    we automate it.

13. **One-time post-clone setup for `merge=ours`** — `.gitattributes`
    declares `merge=ours` for `snapshot/amc_lib.c`, `snapshot/INFO.md`,
    and `src/amc_lib.c` (generated artefacts; canonical resolution
    on conflict is "rebuild from sources"). For the strategy to take
    effect, git needs to know "ours" is a valid driver. Run once
    after cloning:

        git config merge.ours.driver true

    Without this, git falls back to a normal three-way merge on the
    generated files and you get the same conflict noise as before.
