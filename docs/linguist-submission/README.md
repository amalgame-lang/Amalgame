# GitHub Linguist submission kit — Amalgame

This directory holds everything needed to add **Amalgame** to
[github-linguist/linguist][linguist], so that:

- `.am` files get the **Amalgame** badge in the repo Languages bar
  (today they are invisible — Linguist doesn't know the language, so the
  bar is dominated by build scripts: Shell, PowerShell, JS, …);
- ` ```amalgame ` markdown fences syntax-highlight on github.com;
- `.am` files render highlighted in the GitHub file viewer and diffs.

## ⚠️ The one thing we can't control: usage threshold

Linguist's policy is to add a new language **only once it is in use across
hundreds of repositories** on github.com (the maintainers cite "in use in
hundreds of repositories" and reject new languages below that bar). This
is the blocker noted in `ROADMAP_COMPLET.md` ("Submit Amalgame to GitHub
Linguist"). Everything else below is ready; acceptance depends on public
adoption catching up.

**Track adoption** with the GitHub code search before opening the PR — the
maintainers will ask for evidence:

```
# rough proxy for "repos containing Amalgame source"
https://github.com/search?q=%22namespace+Amalgame%22+OR+%22import+Amalgame%22+extension%3Aam&type=code
```

Don't open the PR until that count is comfortably in the hundreds; an
early PR gets closed and can sour a later one.

## What's in here

| File | Purpose |
|------|---------|
| `languages.yml.snippet`   | The entry to paste into `lib/linguist/languages.yml`. |
| `heuristics.yml.snippet`  | `.am` disambiguation (Amalgame vs **Automake**, which already owns `.am`). |
| `samples/Amalgame/`       | Curated real `.am` files that train Linguist's classifier. |
| `collect-samples.sh`      | Copies a wider representative set into a linguist checkout. |

## Prerequisites we already satisfy

- **TextMate grammar** — `editors/vscode/syntaxes/amalgame.tmLanguage.json`,
  scope `source.amalgame` (matches `tm_scope` in the snippet).
- **OSI license** — repo root `LICENSE` (Apache-2.0); the VS Code extension
  carrying the grammar is MIT. Linguist requires the grammar's source repo
  to carry an OSI-approved license. ✅
- **Real samples** — 159 `.am` files in this repo; a curated subset is in
  `samples/Amalgame/`.

## Step-by-step (once the usage threshold is met)

1. **Fork & clone** [github-linguist/linguist][linguist], make a branch:
   `git checkout -b add-amalgame`.

2. **Add the grammar as a submodule.** Linguist tracks grammars under
   `vendor/grammars`. Point it at this repo (the grammar lives inside it):
   ```
   script/add-grammar https://github.com/amalgame-lang/Amalgame
   ```
   This adds the submodule and registers `source.amalgame` in
   `vendor/grammars.yml` / `grammars.yml`. Confirm the scope resolves:
   ```
   script/list-grammars | grep amalgame
   ```

3. **Add the language entry.** Paste `languages.yml.snippet` into
   `lib/linguist/languages.yml` at the right alphabetical spot (A-run).
   Then assign the id — **do not hand-pick it**:
   ```
   script/update-ids        # fills/validates language_id, fails on collision
   ```

4. **Add the heuristic.** Merge `heuristics.yml.snippet` into
   `lib/linguist/heuristics.yml` (so `.am` is split Amalgame vs Automake).

5. **Add the samples.**
   ```
   ./collect-samples.sh /path/to/linguist
   ```
   (or copy `samples/Amalgame/*` by hand into `samples/Amalgame/`).

6. **Build & test locally:**
   ```
   bundle install
   bundle exec rake samples        # rebuild classifier from samples
   bundle exec rake test           # full suite
   script/licensed                 # license check for the new grammar
   ```
   Add focused coverage: a few `.am` paths in `test/test_blob.rb` /
   `test/test_heuristics.rb` asserting `Amalgame` detection, including at
   least one Automake `.am` that must STILL resolve to Automake (guards the
   heuristic against regressions).

7. **Open the PR** against `github-linguist/linguist`, following their PR
   template. Lead with the adoption evidence (search counts / notable
   repos), list the satisfied prerequisites, and note the `.am` collision
   is handled by the heuristic + samples.

## Field choices (and why)

- `color: "#8B5CF6"` — violet ocellus from the peacock logo
  (`assets/logo.svg`). One hex is required; swap if you prefer another
  feather (`#3B82F6` blue, `#06B6D4` cyan, `#EC4899` pink …).
- `tm_scope: source.amalgame` — matches the shipped grammar.
- `ace_mode: csharp` / `codemirror_mode: clike` /
  `codemirror_mime_type: text/x-csharp` — closest existing editor modes.
  Amalgame's surface syntax (`namespace`, `public class`,
  `static void Main(string[] args)`, `Console.WriteLine`, `record`) reads
  almost exactly like C#, so the C# modes give the best fallback
  highlighting in Ace/CodeMirror until a dedicated mode exists.

## Interim, fully under our control (NOT done here)

A repo-local `.gitattributes` override —
`*.am linguist-language=Kotlin` (or `=C#`) — would immediately make the
Languages bar reflect the `.am` codebase instead of build scripts. The
trade-off is the bar would read "Kotlin"/"C#", not "Amalgame". We chose
to wait for the real Linguist entry rather than ship a misleading label;
the override remains available as a one-liner if priorities change.

[linguist]: https://github.com/github-linguist/linguist
