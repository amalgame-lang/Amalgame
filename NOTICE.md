# NOTICE

## Authorship and copyright

Amalgame is the work of **Bastien Mouget** (`@BastienMOUGET`), sole
author and copyright holder. Every source file, runtime header,
build script, test, and document under this repository is
© 2026 Bastien Mouget unless explicitly attributed otherwise in a
file header.

All rights are reserved by the author except as licensed under the
Apache License 2.0 (see [LICENSE](LICENSE)).

## On the use of AI coding assistants

Portions of this codebase were written with the help of an AI
coding assistant (Anthropic's Claude). This is the same kind of
authorship-assistance relationship as using an IDE's
auto-completion, a refactoring tool, or a linter — the AI is a
tool that produces output the author reviews, edits, and commits.
**The AI is not a co-author** within the meaning of intellectual
property law.

Some commits in the git history carry a `Co-Authored-By: Claude
…` trailer. Those trailers are an artifact of the AI tool's
default behaviour and **do not reflect any actual co-authorship
or shared rights** in the work. They are retained for historical
honesty (rewriting the git history would invalidate published
release tags and CI snapshots) but should not be interpreted as
assigning any rights to the AI provider or to any party other
than Bastien Mouget.

From 2026-05-10 onward, new commits no longer carry the
`Co-Authored-By` trailer to remove this ambiguity.

## Third-party components

The Amalgame runtime links against the following third-party
libraries at compile time:

- **Boehm-Demers-Weiser conservative garbage collector**
  ([bdwgc](https://github.com/ivmai/bdwgc)) — MIT-style license.
  Used for `code_alloc` / `GC_MALLOC` calls throughout the
  generated C.
- **libcurl** (https://curl.se/libcurl/) — MIT/X license.
  Used by `Amalgame.Net` (`Http_Get`, `Http_Post`, etc.).
- **libsqlite3** (optional, for future `Amalgame.Database`
  binding) — public domain.

These are linked dynamically against system installations; no
copies are vendored into this repository. Their respective
licenses apply to their own source.

## Trademarks

"Amalgame" as a project name is the author's; no trademark
registration has been filed at the date of this notice.
