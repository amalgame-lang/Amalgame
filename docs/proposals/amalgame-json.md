# Proposal: `Amalgame.Json` — first-class JSON parsing and encoding

**Status:** design (2026-05-09). Implementation deferred to a follow-up PR.
**Author:** v0.4 cycle, post-`amc migrate` v2.
**Tracking PRs:** TBD (this doc only).

## Problem

Four code sites in the compiler currently do JSON by hand:

| File | Function | Purpose |
|------|----------|---------|
| `src/lsp.am` | `JsonStr(body, key)` | extract a string field from an LSP request |
| `src/lsp.am` | `JsonInt(body, key)` | extract an int field (request id, line, character) |
| `src/lsp.am` | `EscapeJsonStr(s)` | escape a value going into an LSP response |
| `src/migrate.am` | `JsonExtract(body, prefix)` | extract first `"...":"<value>"` after a literal prefix |
| `src/migrate.am` | `JsonExtractText(body)` | convenience wrapper around `JsonExtract` for Anthropic responses |
| `src/migrate.am` | `JsonEscape(s)` | escape a value going into an LLM API request |

The two extractors are **substring-pattern matchers**, not parsers. They
scan for `"<key>"` literally in the document, jump to the next quote,
and read until an un-escaped `"`. This is enough for:

- Flat JSON-RPC requests where keys never collide with content (LSP).
- Anthropic / OpenAI / Gemini response shapes where the answer text is
  the only `"text":"..."` (or equivalent) at any nesting level we care
  about.

It breaks on:

- Keys whose name appears inside a string value somewhere earlier in
  the document. Today the document layout makes this rare; tomorrow
  it will not.
- Nested objects and arrays — there's no way to ask for
  `usage.input_tokens` or to walk `content[0].text` without rewriting
  the extractor for each shape.
- Numeric values that aren't on the immediate key (the same key-prefix
  trick doesn't extend to "the *number* under this key inside *that*
  object").
- Anything Unicode-escaped (`\uXXXX`).

The two escapers are subsets of RFC 8259 string escaping: quote,
backslash, `\n`, `\r`, `\t`. They diverge slightly (`EscapeJsonStr`
also handles `\\` and uses `Cr()` for the carriage return; `JsonEscape`
inlines `"\r"`). Both are correct enough for what they touch today,
but every new caller is a new chance to forget a case.

We are about to add `amc migrate v3` which needs to read
`response.usage.input_tokens` and `response.usage.output_tokens` from
the Anthropic API to do **real** cost reporting (vs. the heuristic
estimate in `--dry-run`). Wiring v3 on top of `JsonExtract` means a
second prefix-substring trick that doesn't generalize. Better to spend
the LoC once on a real parser and have it pay back at every site.

## Goal

Ship `Amalgame.Json` — a self-hosted JSON parser, accessor library, and
encoder with an idiomatic Amalgame surface. Use it to replace the four
ad-hoc sites and unblock `amc migrate v3` cost reporting.

### Non-goals (v1)

- **JSON5 / relaxed parsing** — trailing commas, comments, unquoted
  keys, single-quoted strings. Stay strict RFC 8259 to start.
- **Streaming / incremental parsing** — every input fits in memory in
  our use cases. A `JsonReader` that yields tokens can come later if a
  caller wants to parse multi-megabyte responses.
- **Schema validation** — no JSON Schema, no record-style binding to
  Amalgame classes. Callers walk `JsonValue` by hand.
- **Numeric precision beyond `int` and `float`** — no big-integer, no
  arbitrary-precision decimal. `int` is i64 (which covers
  `usage.input_tokens` for any plausible request) and `float` is f64.
- **Pretty-printing options** — encoder emits compact output. Indented
  output is a v2 add (single `bool` flag).
- **Round-trip preservation of key order** — objects are backed by
  `Map<string, JsonValue>`; key order on decode is best-effort.

## High-level design

A single file `src/stdlib/json.am` (new directory) exposes:

```
namespace Amalgame.Json

public enum JsonKind {
    Null
    Bool
    Int
    Float
    String
    Array
    Object
}

public class JsonValue {
    public Kind: JsonKind

    // Accessors — return the underlying primitive, or a sensible
    // zero value (false / 0 / "" / empty list / empty map) if the
    // Kind doesn't match. Callers who care use `Is*()` first.
    //
    // Numeric coercion: `AsInt()` on a `Float` value truncates (3.7 → 3).
    // `AsFloat()` on an `Int` widens (3 → 3.0). Other Kinds zero out.
    public bool        AsBool()
    public int         AsInt()
    public float       AsFloat()
    public string      AsString()
    public List<JsonValue>          AsArray()
    public Map<string, JsonValue>   AsObject()

    // Convenience guards
    public bool IsNull()
    public bool IsBool()
    public bool IsInt()
    public bool IsFloat()
    public bool IsString()
    public bool IsArray()
    public bool IsObject()

    // Object access — returns Null-kind JsonValue if the key
    // doesn't exist (so chained `.Get("a").Get("b").AsInt()` is safe).
    public JsonValue Get(string key)
    public bool      Has(string key)

    // Array access — same Null-kind sentinel for out-of-range.
    public JsonValue At(int i)
    public int       Length()
}

public class JsonError {
    public Message: string
    public Line:    int
    public Column:  int
}

public class JsonResult {
    public Ok:    bool
    public Value: JsonValue   // Null-kind on Ok=false
    public Error: JsonError
}

public class Json {
    // Parse a JSON document. Returns Ok=false with a positioned
    // error on malformed input.
    public static JsonResult Parse(string source)

    // Same shape as Parse but throws on error. Use when you trust
    // the input (e.g. wire-format from a known source).
    public static JsonValue ParseOrThrow(string source)

    // Encode to the compact JSON form. No trailing newline.
    public static string Encode(JsonValue value)

    // Escape a string for embedding inside a JSON string literal.
    // Does NOT add the surrounding quotes — callers concatenate them.
    public static string EscapeString(string s)

    // Constructors for building JsonValue trees programmatically.
    public static JsonValue NullValue()
    public static JsonValue OfBool(bool b)
    public static JsonValue OfInt(int n)
    public static JsonValue OfFloat(float f)
    public static JsonValue OfString(string s)
    public static JsonValue OfArray(List<JsonValue> xs)
    public static JsonValue OfObject(Map<string, JsonValue> m)
}
```

### Why a class-with-tag instead of an algebraic enum

Amalgame supports algebraic enums, and `JsonValue` is a textbook case
for one (`Null | Bool(bool) | ...`). The current matcher / destructuring
support is solid for primitives but interacts awkwardly with
`Array(List<JsonValue>)` — recursion through a generic collection in
an enum payload exercises corners of the typechecker that haven't been
stress-tested. We may flush those out in a follow-up; for now,
`JsonValue { Kind, ... }` is portable, debuggable, and lets us wire the
encoder/decoder without language work.

This is reversible: if algebraic-enum payloads with generics turn out
to work cleanly, `JsonValue` becomes a simple alias and the Kind tag
goes away. Public API doesn't change.

### Why pure Amalgame, not a runtime helper

The other stdlib modules (`String`, `File`, `Process`, `Net`) are C
helpers in `runtime/Amalgame_*.h` exposed to Amalgame as builtins. JSON
could follow the same path with `runtime/Amalgame_Json.h`. We don't,
because:

- **Self-hosting credibility.** A JSON parser is a moderately
  non-trivial recursive program. Writing it in Amalgame and seeing it
  parse the LSP wire format is the kind of thing that signals "this
  language is real." It also exercises algebraic enums, generics,
  string handling, and recursive class methods in one place — the
  best testbed we have for the compiler.
- **No new C dependency.** The runtime is already shipping curl for
  the HTTP-provider migrate paths; adding a JSON parser there would
  push us into "do we depend on cJSON or roll one in C." Keeping the
  parser in Amalgame keeps the runtime lean.
- **Reachable from `amc fmt` / `amc lint`.** Future tooling that
  consumes JSON (config files, IDE state, package manifests) can use
  the same module without re-linking the compiler against an extra
  C library.

The cost is performance. A pure-Amalgame parser is ~3–10× slower than
hand-tuned C. For our call sites this doesn't matter:

- LSP request bodies are small (typically <2 KB).
- LLM responses are larger (10–200 KB) but we make at most a few per
  invocation, and the parse cost is dwarfed by network and inference
  time.

If a future caller hits a real ceiling, we can introduce
`runtime/Amalgame_Json.h` underneath and keep the `Amalgame.Json` API
unchanged.

## Detailed design

### Parser

A straightforward recursive-descent parser:

```
ParseValue   -> ParseObject | ParseArray | ParseString | ParseNumber
              | ParseBool | ParseNull
ParseObject  -> '{' (ParseString ':' ParseValue (',' ...)?)? '}'
ParseArray   -> '[' (ParseValue (',' ...)?)? ']'
ParseString  -> '"' (UnescapeChar | <byte>)* '"'
ParseNumber  -> '-'? Int Frac? Exp?     (rejects leading '+', leading '0' followed by digit)
```

Implementation notes:

- One `JsonParser` class holding `Source: string`, `Pos: int`,
  `Line: int`, `Column: int`. Single-pass, no lookback beyond peeking
  one character.
- Whitespace is the JSON-spec set (`\t \n \r ' '`). `\r` is treated as
  a line break only when not immediately followed by `\n` — same
  cross-platform handling we already do in the compiler lexer.
- `\uXXXX` decodes to UTF-8 bytes. Surrogate pairs (`𝄞`)
  decode as a single code point. We already produce UTF-8 byte sequences
  for `\u` escapes in the lexer (PR #150-ish — `feature/unicode-escape`),
  so the helper exists.
- Numbers parse with the existing `String_ToInt` / `String_ToFloat`
  helpers. Float parsing uses `String_ToFloat` (introduced for the
  `float` literal lexer in v0.3).
- Errors carry a 1-based line/column pair pointing at the offending
  byte. The `JsonError` shape mirrors `TypeError` so callers
  formatting both can use the same code path.

Estimated size: 300–400 LoC including the unescape table.

### Encoder

Recursive walk over `JsonValue`:

- `Null` → `"null"`
- `Bool` → `"true"` / `"false"`
- `Int` → `String_FromInt(n)`
- `Float` → `String_FromFloat(f)` — emit shortest round-tripping
  representation. Special cases: `NaN` and `Inf` are not valid JSON;
  the encoder errors (we'll raise a clear runtime panic for v1).
- `String` → quotes + `EscapeString(s)` + quotes.
- `Array` → `[v0,v1,...]`, no spaces.
- `Object` → `{"k0":v0,"k1":v1,...}`, no spaces. Key order = `Map`'s
  insertion / iteration order.

Estimated size: 80–120 LoC.

### `EscapeString`

Same control-character handling as the existing escapers, plus:

- `\b` (backspace) and `\f` (form feed) — RFC 8259 mandates these.
- Unicode escape for code points 0x00–0x1F that aren't already in the
  named-escape set. (`EscapeJsonStr` and `JsonEscape` currently emit
  raw bytes for these — technically out-of-spec but tolerated by
  every parser we feed today. The new escaper does it right.)

## Migration plan

Three phases, each landed as its own PR so the diff stays reviewable.

**Phase 1 — ship the module.** Add `src/stdlib/json.am`, the test
suite (see below), and wire it into the build. No call site change.
Existing `JsonStr` / `JsonInt` / `JsonExtract` / `JsonEscape` /
`EscapeJsonStr` keep working. PR is purely additive.

**Phase 2 — swap call sites.** Replace each ad-hoc helper with a
`Json.Parse` + `value.Get(...).AsX()` chain (or `Json.EscapeString`
for the escapers). Land per file: `src/lsp.am` first (small surface,
hot in CI), then `src/migrate.am`, then any `generate.am` /
`explain.am` sites that match the same pattern. After each file,
re-run the LSP probe + the LLM-command tests to catch shape drift.

**Phase 3 — remove the helpers.** Delete `JsonStr`, `JsonInt`,
`JsonExtract`, `JsonExtractText`, `JsonEscape`, `EscapeJsonStr`. Pure
deletion PR; nothing should still call them after Phase 2.

## Testing

A `tests/samples/json_*.am` family covering:

- **Parser smoke tests** — primitives (`null`, `true`, `false`, `42`,
  `3.14`, `"hello"`), nested object, nested array, mixed structure.
- **String escapes** — `\"`, `\\`, `\n`, `\t`, `\b`, `\f`, `\/`,
  `é` (Unicode BMP), `𝄞` (surrogate pair).
- **Round-trip** — parse + encode + parse, assert structural equality
  on a corpus that includes:
    - A real Anthropic API response (captured fixture, secrets
      stripped). Verifies `usage.input_tokens` and the
      `content[0].text` access paths used by `amc migrate v3`.
    - A real LSP `initialize` request and a `publishDiagnostics`
      notification.
    - The `package.json` from `editors/vscode/`.
- **Error cases** — trailing comma, unclosed string, bad escape, lone
  high surrogate, multi-byte input that's almost-but-not-quite UTF-8.
  Each asserts `Ok == false` with an expected line/column.
- **Encoder edge cases** — empty array `[]`, empty object `{}`, deep
  nesting (verifies the encoder doesn't have an arbitrary recursion
  cap), all control bytes escape correctly.

Target: every public method on `JsonValue` and every parser path has
at least one test. Coverage instrumentation isn't ergonomic on the
self-hosted compiler yet, so we eyeball this rather than measure.

## Decisions (settled in review on 2026-05-09)

- **Split `Int` and `Float` kinds on parse.** `42` parses as `Int`,
  `42.0` parses as `Float`. Matches what existing `JsonInt` callers
  expect and preserves round-trip fidelity (`1` and `1.0` re-encode
  to themselves). The cost is one extra Kind variant.
- **`AsInt()` on a `Float` value truncates.** `AsInt()` on `3.7`
  returns `3`. Consistent with `String_ToInt("3.7") == 3` and with
  most mainstream JSON libs. Strict-mode "Kind mismatch returns 0"
  was rejected as too surprising for the typical "I just want the
  number" caller; throwing would require infra we don't yet have
  pervasively.
- **`json.am` lives in a new `src/stdlib/` directory.** Scopes
  future stdlib additions (`Amalgame.DateTime`, `Amalgame.Regex`,
  `Amalgame.Random`, ...) to a known place. Requires a one-line
  adjustment to `build_amc.sh` to glob `src/stdlib/*.am` alongside
  the existing source roots.

## Other open items (deferred, not blocking v1)

- **Error recovery vs. fail-fast.** Currently fail-fast (return on
  first parse error). If we ever need partial-parse semantics (e.g.
  for "show me what you got, then the error"), the parser will need
  to thread a `bool failFast` flag. Probably yes-default-fail-fast,
  no-flag-now.
- **Algebraic-enum migration.** When/if we move `JsonValue` to a
  proper tagged union, the public surface (`AsInt()`, `Get(key)`,
  etc.) stays the same. The breaking change is for callers that
  built `new JsonValue()` and set `.Kind` directly. The proposed v1
  does not encourage that pattern (constructors are static factories
  on `Json`), so the migration cost should be near-zero.

## Estimated cost

- **Phase 1** (module + tests): ~500 LoC of Amalgame, ~150 LoC of
  test fixtures. ~1 day of work for a focused session.
- **Phase 2** (swap call sites): ~80 LoC net deleted across 3 files.
  ~2 hours.
- **Phase 3** (delete dead helpers): pure deletion, ~150 LoC out.
  ~30 min.

Total: ~1.5 days end-to-end, deliverable as 3 sequential PRs against
`develop`.
