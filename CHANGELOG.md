# Changelog

All notable changes to Amalgame are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) loosely.

For releases prior to v0.3.2, see the git log and `ROADMAP_COMPLET.md`.

---

## [v0.7.3] — 2026-05-12

The **"WebSocket + final `.h` runtime"** patch release. 4 new
tests on top of v0.7.2 (598 → **602 PASS**). Marks the freeze
on new `runtime/Amalgame_*.h` headers — every stdlib module
from v0.7.6 onwards will land as a `.am` file via project G
(inline-C `@c { ... }` blocks).

### `Amalgame.Net.WebSocket` — RFC 6455 client

```
let ws = WebSocket.Connect("echo.websocket.org", 80, "/")
if (ws != null && ws.IsConnected()) {
    let _ = ws.SendText("hello")
    let reply: string = ws.ReceiveText()
    Console.WriteLine(reply)         // "hello"
    ws.Close()
}
```

Surface:
- **`WebSocket.Connect(host, port, path) → WebSocket?`** — TCP
  open + HTTP upgrade handshake (GET + Sec-WebSocket-Key/Accept
  with SHA-1 + Base64 verify). Returns `null` on any failure
  (refused TCP / DNS / HTTP non-101 / Sec-WebSocket-Accept
  mismatch).
- **`ws.SendText(s) → bool`** — client → server text frame
  (opcode 0x1, FIN set, masked per RFC).
- **`ws.ReceiveText() → string?`** — blocks for one server → client
  frame; transparently replies to Ping (0x9) with Pong (0xa) and
  reads the next non-control frame; returns `null` on Close
  (0x8) or read error; returns `""` for binary / continuation
  frames so callers can opt to ignore them.
- **`ws.Close()`** — sends a Close frame and shuts the TCP fd.
- **`ws.IsConnected()` / `ws.GetHost()` / `ws.GetPort()`** —
  trivial accessors.
- **`WebSocket.AcceptKey(clientKey) → string`** — derives the
  Sec-WebSocket-Accept value. Exposed so test fixtures (and
  third-party WS-aware code) can verify the handshake without
  reaching into the runtime.

Implementation lives in `runtime/Amalgame_WebSocket.h`,
self-contained: SHA-1 (FIPS 180-4) and Base64 (RFC 4648) are
inlined there since `Amalgame.Crypto` only ships SHA-256.
Frame parser handles both masked + unmasked server → client
frames defensively; payload size capped at 16 MiB so a buggy /
malicious server can't OOM the client.

Out of scope (next iterations):
- **`wss://` TLS** — needs an OpenSSL binding or a TcpTls
  layer; gets its own release.
- **Binary opcodes (0x2)** — same machinery, returns
  `List<int>` instead of `string`. Wait for a real consumer.
- **Continuation frames** — multi-fragment messages aren't
  reassembled in v1 (each frame is a complete message). Add
  when a real protocol needs them.
- **per-message-deflate negotiation** — handshake-level
  optimisation; pairs with `Amalgame.Compress` once a
  `Sec-WebSocket-Extensions: permessage-deflate` consumer
  shows up.
- **HTTP subprotocols** (`Sec-WebSocket-Protocol`) — easy add
  to `Connect()`'s signature when needed.

### 4 stdlib tests

- **RFC 6455 §1.3 canonical Sec-WebSocket-Accept** —
  client key `"dGhlIHNhbXBsZSBub25jZQ=="` must produce
  `"s3pPLMBiTxaQ9kYGzzhZRbK+xOo="`. Exercises the SHA-1 + Base64
  pair end-to-end without a live server.
- **Empty client key edge case** — AcceptKey("") returns a
  28-char base64 string (not a crash).
- **`WebSocket.Connect` to refused port** — returns `null`
  (well-defined "no server" path).
- **`WebSocket.Connect` to bogus DNS** — returns `null` on
  `getaddrinfo` failure.

Live-server integration testing via Python `websockets` is
opt-in; the runner skips it silently when the module isn't
installed. A dedicated WS-server harness lands in a follow-up
once a real consumer (LSP-over-WS?) needs end-to-end coverage.

### Roadmap reorientation

The freeze on new `runtime/Amalgame_*.h` headers takes effect
this release. The two follow-ups are:

- **v0.7.4 = project G — inline-C blocks (`@c { ... }`)** —
  major language feature. Once shipped, every stdlib module
  can be written as a `.am` file with C-level glue inlined,
  collapsing the `runtime/header.h + stdlib/facade.am` dance.
- **v0.7.5 = project F — `libamalgame.a` pre-compile** — the
  user-facing stdlib `.am` modules pre-compiled into a static
  library shipped with the amc binary. Today every `amc -o
  foo foo.am` re-parses ~10 `.am` stdlib modules; F drops
  that overhead.

Both projects are detailed in the "Open design questions"
section.

---

## [v0.7.2] — 2026-05-12

The **"stdlib expansion 3"** patch release. 23 new tests on top
of v0.7.1 (575 → **598 PASS**). Plus a cgen fix that drops a
paper-cut on chained collection-method calls.

### `Amalgame.Compress` — zlib gzip + raw-deflate codec

```
let body: List<int> = File.ReadBytes("payload.bin")
let gz: List<int>   = Compress.Gzip(body)
File.WriteBytes("payload.bin.gz", gz)

let back: List<int> = Compress.Gunzip(gz)
// back == body (byte-exact round-trip)
```

Four entry points, paired:

- **`Compress.Gzip` / `Gunzip`** — RFC 1952 wrapper. Same
  bytes `gzip -c` would produce, correct file-format magic,
  suitable for HTTP `Content-Encoding: gzip` or `.gz` files.
- **`Compress.Deflate` / `Inflate`** — RFC 1951 raw stream,
  no header, smaller, suitable for embedded protocols
  (WebSocket per-message-deflate, custom binary RPCs).
- **`Compress.GzipString` / `GunzipString`** — UTF-8 string
  convenience pair.

Input + output as `List<int>` byte buffers (one entry per
byte, 0..255). Same shape as `Amalgame.Crypto.Sha256` /
`Amalgame.Random.SystemBytes` — pipe-friendly.

Build adds `-lz` to every gcc invocation that already links
against `libgc` / `libcurl` (`build_amc.sh`, the user-compile
path in `main.am`, every test runner). 9 stdlib tests cover
empty input, gzip magic bytes, large-input shrink ratio,
round-trip exactness, and raw-deflate symmetry.

### `Amalgame.Formats.MsgPack` — binary codec via JsonValue

```
let bytes: List<int> = MsgPack.EncodeJson(jv)   // any JsonValue
let jv2: JsonValue   = MsgPack.DecodeJson(bytes)
```

MessagePack 1.0 subset, pure-Amalgame (no runtime header) on
top of `JsonValue`. Coverage:

| Marker            | Bytes        | Type                  |
|-------------------|--------------|-----------------------|
| `0xc0`            | nil          | `JsonValue.IsNull()`  |
| `0xc2` / `0xc3`   | bool         | `IsBool()` / `AsBool()` |
| `0xxxxxxx`        | positive fixint (0..127) | `IsInt()` |
| `111xxxxx`        | negative fixint (-32..-1) | |
| `0xd0` / `0xd1` / `0xd2` | int 8 / 16 / 32 | |
| `101xxxxx` + bytes | fixstr (0..31)  | `IsString()` |
| `0xd9` / `0xda` + len + bytes | str 8 / 16 | |
| `1001xxxx` + items | fixarray (0..15) | `IsArray()` |
| `0xdc` + 2-byte len | array 16 | |
| `1000xxxx` + pairs | fixmap (0..15) | `IsObject()` |
| `0xde` + 2-byte len | map 16 | |

Out of scope (v2): int64, float 32/64, bin, ext, timestamps —
each easy to add (encoder gets a branch, decoder gets a prefix
match), but not blocking for typical config / RPC payloads
where the fix-* small forms win.

14 stdlib tests cover encode + decode round-trips for every
type above, with edge-case nesting (fixarray of fixints,
fixmap of mixed values, single-key map, empty containers).

Round-trips through `JsonValue` mean any code that builds JSON
trees can switch to MsgPack with a one-line `Json.Stringify(jv)`
→ `MsgPack.EncodeJson(jv)` rename.

### CGen fix: chained `<call>.Count()` on collections

`EmitCalleeStr` used to emit `<bareReceiverType>_<methodName>`
verbatim for chained method calls. When the inner call returned
`AmalgameList*` / `Map*` / `Set*` (e.g. `jv.AsArray().Count()`),
the PascalCase `Count` collided with the C runtime's camelCase
`AmalgameList_count`, leaking an `-Wimplicit-function-declaration`
warning and an undefined symbol at link.

Fix downcases the first letter of `mname` when `bareR` is one
of the three collection structs. User classes still keep
PascalCase methods (`ArgParser_Flag` / `MsgPackCursor_Read`
etc. unchanged). Roadmap "Compiler — polish" item recoché.

### Deferred to v0.7.3

`Amalgame.Net.WebSocket` (RFC 6455 client) — ~1 day of focused
work for the handshake + SHA-1 Sec-WebSocket-Accept verify +
frame masking + ping/pong + a live-server test harness. The
test infrastructure is its own piece of work; better to land
it in a dedicated release. Tracked in the roadmap.

---

## [v0.7.1] — 2026-05-12

The **"stdlib expansion 2"** patch release — second installment
of the v1.0-bound stdlib fill-in. 43 new tests on top of v0.7.0
(532 → **575 PASS**).

### `Amalgame.Formats.Yaml` — YAML 1.2 subset reader

CI / app config use case:

```
let cfg = Yaml.Parse(File.ReadAll("app.yml"))
let port: int = cfg.Get("server").Get("port").AsInt()
let envs = cfg.Get("environments")  // YamlValue (Array)
let firstEnv = envs.At(0).Get("name").AsString()
```

Coverage:
- Block mappings (nested arbitrarily deep).
- Block sequences of scalars and of maps.
- Typed scalars: bool (`true` / `false`), int, float, plain
  string, single- and double-quoted strings (with `\n` / `\t` /
  `\\` / `\"` escapes in double quotes; `''` → `'` in single).
- Comments (`#` to end of line, with the leading-space rule
  for inline comments).
- Blank-line tolerance.
- Missing-key returns a Null-kind `YamlValue` so chains like
  `cfg.Get("a").Get("b").AsString()` stay readable.

Out of scope (raise an error or fall back to plain string —
not the goal of a config-reader subset): anchors / aliases,
multi-doc `---`, flow style `[1,2]` / `{a: b}`, multiline
scalars (folded `>`, literal `|`), tags (`!!str`).

`YamlValue` tree mirrors `TomlValue` shape so callers can
switch formats with a one-line `Toml.Parse` ↔ `Yaml.Parse`
rename.

### `Amalgame.DateTime` — UTC breakdown accessors

`Instant` gains six new methods:

```
let i = Instant.Now()
Console.WriteLine(String_FromInt(i.Year())  + "-"
                + String_FromInt(i.Month()) + "-"
                + String_FromInt(i.Day())   + " "
                + String_FromInt(i.Hour()))
```

`Year()` / `Month()` (1–12) / `Day()` (1–31) / `Hour()` (0–23) /
`Minute()` (0–59) / `Second()` (0–60 with leap-second tick) —
all UTC, via `gmtime_r` / `gmtime_s`.

Named timezones (`LocalTime`, `In(zone)`, `strftime`-ish
formatter, `+HH:MM` offset parsing) stay deferred — needs a
tzdata shipping strategy (bundle IANA vs. OS delegate via
POSIX `TZ` env / `/usr/share/zoneinfo` / Windows
`GetDynamicTimeZoneInformation`). The UTC half covers the
typical server-side / logging use case.

### `Amalgame.Regex` — POSIX extended-regex binding

```
Regex.Test("[0-9]+", "abc")                    // false
let m = Regex.Match("([a-z]+) ([a-z]+)", "foo bar")
m.GroupText(0)                                  // "foo"
m.GroupText(1)                                  // "bar"
Regex.ReplaceAll("[0-9]+", "a1b2c3", "X")       // "aXbXcX"
```

Surface:
- `Regex.Test(pat, subj) → bool` — cheap predicate; skips the
  match-result allocation.
- `Regex.Match(pat, subj) → Match*` (or `null` on no-match) —
  with `GetText` / `GetStart` / `GetEnd` / `GroupCount` /
  `GroupText(i)` / `GroupStart(i)` / `GroupEnd(i)` for the
  parenthesised groups (up to 16, indices 0..N-1).
- `Regex.Replace(pat, subj, repl)` — first occurrence; `\1`
  capture-expansion stays literal in v1.
- `Regex.ReplaceAll(pat, subj, repl)` — every non-overlapping
  occurrence, zero-length-match safe (steps past the match by
  at least one byte to avoid spinning on patterns like `^` or
  `.*`).

Syntax is POSIX extended (ERE) — the standard suspects work:
`. * + ? ^ $ [...] ( ) | {n,m}`. PCRE-only features (`\d` /
`\w` / `\s` shorthand, look-arounds, non-greedy modifiers,
named captures, Unicode property classes) are out of scope —
bind libpcre2 in a future package when a real consumer needs
them. `regex.h` is in libc everywhere we ship (POSIX, MinGW)
so no third-party dependency.

### Wiring recipe

Three places to register a new builtin runtime-backed type
follows from v0.7.0 — see commits `7360d4c` (v0.6.4 BuildInfo)
and `56b03c4` (v0.7.0 Vec/FileWatcher) for the pattern.

---

## [v0.7.0] — 2026-05-12

The **"stdlib expansion 1"** minor release — first installment of
the v1.0-bound stdlib fill-in. 23 new tests on top of v0.6.4
(509 → **532 PASS**).

### `Amalgame.Math.Vec` — Vec3 / Vec4 / Mat4

Scalar 3D math primitives for game / graphics / linear-algebra
use cases. Runtime in `runtime/Amalgame_Math_Vec.h`; facade in
`src/stdlib/math_vec.am`. The cgen short-circuits method calls
to the runtime helpers directly so user code reads at the right
level of abstraction without paying the namespace-dispatch cost.

```
let a = new Vec3(1.0, 2.0, 3.0)
let b = new Vec3(4.0, 5.0, 6.0)
let c = a.Cross(b)                // (-3, 6, -3)
let m = Mat4.RotateZ(Math.Pi() / 4.0)
let p = m.TransformVec4(new Vec4(1.0, 0.0, 0.0, 1.0))
```

Coverage:
- **Vec3** — ctor + GetX/Y/Z + Add/Sub/Scale/Dot/Cross/Length/
  Normalize/Equals.
- **Vec4** — ctor + GetX/Y/Z/W + Add/Sub/Scale/Dot.
- **Mat4** — Identity/Translate/Scale/RotateX/Y/Z/Multiply/
  TransformVec4/Get/Set. Column-major (OpenGL convention) so
  `glUniformMatrix4fv` works without transposition.

All operations heap-allocate via GC_MALLOC so chained calls
don't alias their inputs (functional style — `a.Add(b)` returns
a fresh `Vec3*`, leaving `a` and `b` untouched).

16 stdlib tests cover every method, including a verified
RotateZ(π/2) on (1, 0, 0, 1) → (0, 1, 0, 1).

Complex numbers and BigInt are deferred — different scope (no
GMP dep wanted); revisit when a real consumer needs them.

### `Amalgame.IO.FileWatcher` — single-file mtime polling

```
let w = new FileWatcher("/etc/foo.toml")
while (!Service.ShouldStop()) {
    if (w.Changed()) { reload() }
    // sleep loop
}
```

Cross-platform via `stat(2)` / `_stat64`. API:
- `new FileWatcher(path)` — snapshot the initial mtime.
- `w.Exists()` — true iff the file is currently present.
- `w.Changed()` — true the first call after the mtime advances
  (or the file appears / disappears); false on subsequent calls
  until the next change. After a true return, the watcher's
  internal snapshot is updated to the new mtime so the polling
  loop self-resets without extra bookkeeping.
- `w.GetPath()` — the path passed at construction.

Covers the "reload a config file" / "rebuild on source change"
80% use case. Directory-level recursive watches via `inotify` /
`FSEvents` / `ReadDirectoryChangesW` are deferred — add when a
real consumer needs them; the polling MVP works on every
filesystem amc compiles for.

7 stdlib tests use the delete / re-create flip for
deterministic results (mtime-advance granularity varies across
filesystems).

### Wiring

Three places to register a new builtin runtime-backed type in
the bootstrap pipeline (`Vec3`/`Vec4`/`Mat4`/`FileWatcher` all
follow the same recipe — see commit `56b03c4` for the diff):

1. `src/resolver/symbol.am` — add to the `builtins` list.
2. `src/resolver/resolver.am` — `DeclareGlobal("Type", "type", false)`
   plus one `DeclareGlobal` per runtime helper.
3. `src/generator/c_gen.am` — entries in `TypeToC`,
   `InferTypeFromExpr` (NEW_EXPR + CALL paths), `NEW_EXPR`
   emitter (new-keyword short-circuit), `isCoreStdlib`
   (static-call dispatch), `bareType`-strip path (instance-call
   dispatch), and an `#include` line in the auto-emitted prelude.

---

## [v0.6.4] — 2026-05-12

The **"cgen cleanup + profile"** patch release.

### Snapshot C output shrunk (-9%)

`amc_lib.c` goes from 22 860 → 21 361 lines (-1 499 lines) and
1.17 MB → 1.06 MB (-9%). Two source changes:

- cgen no longer emits a `__attribute__((unused))` marker on
  every `VAR_DECL` (3 342 occurrences gone).
- cgen no longer emits `(void)self;` / `(void)<param>;`
  boilerplate at the top of every method body (~2 000 lines
  gone).

The gcc invocations (`build_amc.sh` + `main.am` user-compile
flow) now pass `-Wno-unused-variable -Wno-unused-parameter
-Wno-unused-but-set-variable` instead. `amc --lint` is the
canonical "is this variable actually used" gate — it already
flagged unused locals, shadowing, the v0.6.3 catch-binder, and
var-never-reassigned. The trade is: gcc warnings hidden at
build, linter handles correctness; in exchange, every PR review
diff on the checked-in snapshot is meaningfully smaller, and
`compile + link` is a hair faster.

### Compile-time phase profiling

`amc --verbose foo.am` now prints per-phase timings on stderr:

```
  parse:     154us
  resolve:   199us
  typecheck: 9us
  cgen:      411us
Compiling: 1 file(s)
Generated: foo.c (30 lines) [Library]
Build OK
```

Powered by a single `Stopwatch` (`Amalgame.DateTime`) reset at
each phase boundary. Reveals the hot spot without external
profiling.

`./build_amc.sh` total wall time is now ~2 s end-to-end (was ~5 s
when the AST-cache roadmap entry was originally written), so the
proposed pickled-AST cache is no longer high-priority — would
risk a serializer-bug class for marginal gain. Revisit if a
single-file compile ever exceeds ~50 ms.

### Internal: Box/Unbox helpers consolidated

Audit confirmed `BoxAsVoid(expr)` and `UnboxScalar(ctype, expr)`
already serve every site that previously open-coded the
`(void*)(intptr_t)X` dance. Roadmap entry recoché [x] — no code
change.

---

## [v0.6.3] — 2026-05-12

The **"internal refactor"** patch release. Three items from the
"Compiler — internal refactoring & optimization" backlog.

### Reduce void* erasure on local containers

cgen's element/value-type inference for `xs.Get(i)` on a local
`List<T>` / `Map<K,V>` now consults the `__local__` /
`__local_map__` registry that `TrackGenericLocal` already
populates at the declaration site. Drops the
`let n: string = xs.Get(0)` workaround at most call sites:

```
// Before
let n = names.Get(0)   // names: List<string>
// → void* n = (void*)AmalgameList_get(names, 0);

// After
let n = names.Get(0)
// → code_string n = (code_string)AmalgameList_get(names, 0);
```

Class-method return types were already propagating since v0.5;
the gap was specifically on the generic-collection accessors.

### Linter coverage

`amc --lint` learns two new diagnostics:

- **catch-binder unused** — `try { ... } catch e { ... }` where
  the binder isn't read warns `unused local 'e' (prefix with
  '_' to silence)`. Same opt-out as other unused locals.
- **`var`-declared-but-never-reassigned** — flags `var x = 0;
  print(x)` where `x` is read but never reassigned, suggesting
  `let` would communicate the immutability better. Picks up
  every `=` / `+=` / `-=` / `*=` / `/=` / `%=` / `&=` / `|=` /
  `^=` / `<<=` / `>>=` operator as a reassignment.

Still TBD (each needs typecheck integration or a non-trivial
walk): suspicious match (missing default + non-exhaustive
enum), implicit fallthrough, dead `import`.

### `ArgParser` framework

`src/argparser.am` ships a fluent registration class for the
subcommand arg loops that were duplicated across `migrate.am`,
`generate.am`, `explain.am`, `main.am::RunFmt`/`RunTest`, and
`add_cmd.am`:

```
let ap = new ArgParser()
ap.Flag("-w").Flag("--write").Flag("-h").Flag("--help")
  .Option("--from")
  .Parse(argc, 2)

if (ap.HelpRequested()) { ... }
if (String_Length(ap.GetUnknown()) > 0) {
    Console.WriteError("amc fmt: unknown argument '"
        + ap.GetUnknown() + "'")
}
let write = ap.HasFlag("-w") || ap.HasFlag("--write")
let files = ap.GetPositionals()
```

`RunFmt` migrated as the inaugural caller. Other subcommands
can move to the framework incrementally without coordinating a
big-bang rewrite. Lacks short-flag clustering (`-vh`),
`--key=value` form, and `--` end-of-flags marker — add when a
real caller needs them.

---

## [v0.6.2] — 2026-05-12

The **"compiler audit + parser bugfix"** patch release.

### Roadmap audit

Six items in the "Compiler — internal refactoring & optimization"
section had been silently resolved by intervening compiler work
(NodeKey fix, cgen chained-call handling, constructor forward-
decl pass) but were never re-checked. Audit pass verified each
via local repros and marked them `[x]`:

- `>> N` inside a `let` no longer drops the shift operator.
- Constructor forward-declarations now precede call sites.
- `while (cur != null)` traverses a linked list without GC drama.
- Typechecker return-type for enum members in IF bodies is
  correct (NodeKey fix from null-safety closed the leak).
- Chained `obj.Field.M()` and `obj.M().M()` both lower right.
- `Snapshot size` item (b) — `.gitattributes merge=ours` —
  shipped already; split the remaining "shrink C output" half
  into its own item.

### Parser: diagnose TS-style free functions

`public List<int> Helper(int n) { ... }` at file scope used to
parse without complaint; the cgen then emitted the call site as
`Helper(...)` against no definition, leaking a
`-Wimplicit-function-declaration` warning into the user binary.
The parser already caught the `fn name(...)` form; this release
extends the same lookahead-and-diagnose pattern to TS/C-style
return-type-then-name signatures:

```
$ amc --check helper.am
Top-level functions aren't supported (got 'Helper' at 2:8).
Wrap it inside a class as `public static`.
```

`ParseDecl` scans the next ~16 tokens for an `IDENT (` pair
before any `{ / ; / }` punctuator and emits the error, then
skips past the body. False positives only on top-level bare
calls (already invalid Amalgame).

---

## [v0.6.1] — 2026-05-12

The **"package-manager polish + ship-to-prod"** release. Eight QoL
items landed on top of v0.6.0, plus a `--version` rework that
embeds build provenance.

### New `amc package` verbs

- **`amc package info <name>`** — single-package detail view:
  description, url, tier, licence, category, maintainer, every
  indexed version with compat marker, and whether the package is
  currently installed in this project (matched by URL slug
  against `amalgame.lock`).
- **`amc package outdated`** — cross-references the lockfile
  against the index and lists installed deps with a newer
  compatible tag, ready to feed into `amc package update`. Silent
  on up-to-date deps (grep-friendly in CI); unindexed deps go to
  stderr. Exit code stays 0 in non-error cases.
- **`amc package notice`** — aggregates each installed package's
  `[package].license / authors / description` from its cached
  `amalgame.toml` into a NOTICE-style listing on stdout, ready to
  redirect into `NOTICE_DEPS.md` for downstream commercial
  redistribution / Apache-2.0 NOTICE propagation.
- **`amc package check [--frozen]`** — verifies `amalgame.lock`
  matches the installed cache. Bare form is informational
  (always exits 0); `--frozen` exits 1 on mismatch — drop-in for
  CI fail-fast lanes that catch a PR bumping the lockfile
  without re-installing.

### Flags on existing verbs

- **`amc package search --no-versions`** — skip the per-package
  versions block for faster browsing of long search results.
- **`amc package versions --json`** — machine-readable output
  (jq / CI compat probes); schema is stable.

### Compiler / cgen

- **`Path.{Combine,Sep,IsAbsolute,Normalize}`** now lower to
  their flat runtime symbol without requiring
  `import Amalgame.Path` — the four method names match the
  runtime 1:1 so it's safe. `Directory` / `Filename` /
  `Extension` / `Stem` still need the import (they wrap
  `Path_Get*` and would mis-mangle without the namespace
  dispatch). Removes ceremony from `amc`'s own code.

### `amc --version` enriched

Bakes the git short-SHA + UTC build timestamp at link time via
`-DAMC_GIT_REV=...` / `-DAMC_BUILD_DATE=...` (`build_amc.sh`
wires both; fall back to "" cleanly when the defines are
absent). Banner now surfaces author / licence / website /
repository / issues URLs:

```
amc 0.6.1 (commit abc12345, built 2026-05-12T01:23:45Z)
Self-hosted Amalgame compiler.
Copyright (c) 2026 Bastien Mouget. License: Apache-2.0.
Website:    https://amalgame.me
Repository: https://github.com/amalgame-lang/Amalgame
Issues:     https://github.com/amalgame-lang/Amalgame/issues
```

New `runtime/Amalgame_BuildInfo.h` exposes the defines via
inline `BuildInfo_GitRev` / `BuildInfo_BuildDate` helpers;
`BuildInfo` joins the cgen's core-stdlib dispatch list so
`BuildInfo.GitRev()` lowers without ceremony.

### Help-text audit

`amc package add --help` now documents the `<shortname>`
auto-resolve form (v0.6.0), the `--no-precompile` flag (v0.5.4),
and the full semver operator set on `required-amalgame`. The
top-level `amc --help` verb list adds the new verbs (`versions`,
`info`, `outdated`, `notice`, `check`). Fixes a stray
`amc update` typo in `update`'s missing-arg error path. Drops
the reference to the never-shipped `amc package gc` from
`remove --help`. `PrintPackageUsage` surfaces the `[@<tag>]`
safety suffix on `remove`.

### Documentation

- `ROADMAP_COMPLET.md` — per-release breakdown of the package-
  manager entry (v0.5.3 → v0.6.1) + `Path.*` dispatch note +
  new entry for unifying the ~1.9k lines of bash test runners
  under `amc test`.
- `docs/proposals/amalgame-package-manager.md` — documents the
  packages-index schema v2 (`[[version]]` array), back-compat
  matrix, ordering convention, and 30-min TTL.

### External repos

- `amalgame-database-nosql-redis` + `amalgame-messaging-mqtt` CI
  matrix bumped to `[v0.6.0, v0.5.0]` — catches regressions on
  the latest amc while keeping the baseline coverage.
- `amalgame-database-sqlite` v0.2.1 orphan tag deleted (it was a
  red CI history entry with no release attached).

---

## [v0.6.0] — 2026-05-12

The **"auto-resolve + semver constraints"** release. Two related
package-manager improvements that mature the install surface
toward what Cargo / npm users expect.

### Auto-resolve `amc package add <pkg>` (no tag)

```
$ amc package add duckdb
Auto-resolved 'duckdb' → 'duckdb@v0.1.1' (latest compatible with amc 0.6.0)
Resolving shortname 'duckdb' via amalgame-lang/packages-index...
  → github.com/amalgame-lang/amalgame-database-duckdb (official)
…
Added duckdb v0.1.1
```

Drop the `@<tag>` suffix on **indexed** shortnames — amc fetches
the index, walks the package's `[[version]]` entries newest-last,
and picks the first one whose `required-amalgame` is satisfied by
the running amc. Full git URLs still need an explicit `@<tag>`
(the index is the SoT for "what's known to amc"; unindexed repos
opt out by design).

### Semver constraints in `required-amalgame`

`PackageRegistry.VersionSatisfies` learned more operators:

| Operator | Example         | Semantics                          |
| -------- | --------------- | ---------------------------------- |
| `>=`     | `>=0.5.0`       | At least (existing)                |
| `>`      | `>0.5.5`        | Strictly greater                   |
| `<=`     | `<=0.5.5`       | At most                            |
| `<`      | `<0.5.5`        | Strictly less                      |
| `=`      | `=0.5.5`        | Exact match                        |
| `^`      | `^0.5.3`        | Caret (locks major, or minor for 0.x) |
| `~`      | `~1.2.3`        | Tilde (locks major.minor)          |
| bare     | `0.5.0`         | Treated as `>=` (back-compat)      |

Caret is the npm/Cargo flavour: `^1.2.3` ↔ `>=1.2.3, <2.0.0` for
`major > 0`; `^0.Y.Z` ↔ `>=0.Y.Z, <0.(Y+1).0` because 0.x minor
bumps may break; `^0.0.Z` ↔ `=0.0.Z`. Tilde always locks `major.minor`.

19 new stdlib tests cover every operator + edge cases (caret 1.x
/ 0.x / 0.0.x branches, tilde minor reject, bare-as-ge fallback).

---

## [v0.5.6] — 2026-05-12

The **"index TTL + QoL"** patch release.

### Index cache TTL

`amc package search` / `versions` / `add` previously cached the
packages-index **forever** at `~/.amalgame/cache/packages-index.toml`.
Bumping a tag in a package repo + waiting for the auto-PR to merge
on the index didn't surface until the user manually ran
`--refresh` or `amc package cache clear`. Annoying.

v0.5.6 adds a **30-min TTL** via `date -r <file> +%s` (POSIX +
MSYS2 + Cygwin). Older than that → automatic re-fetch on the
next `search` / `versions` / `add`. Network failure during
refresh falls back to serving the stale cache with a warning
(better than a hard error when offline).

### Network failure resilience

When the index can't be fetched (offline, 5xx, etc.) but a
cached file exists, `FetchIndex()` now returns the stale cache
with a `warning: index fetch failed, serving stale cache`
message on stderr. Previously: hard error. Tradeoff: slightly
stale data over no-go.

### External packages — test runner fixes

`amalgame-database-nosql-redis` and `amalgame-messaging-mqtt`
had `tests/run_tests.sh` still using the v0.5.0-era `amc add`
syntax (removed in v0.5.1 by PR #303). Their CI was silently
SKIPping every case for months. Now both runners use the
AMALGAME_PACKAGES_DIR + symlink trick, matching what SQLite
and DuckDB do.

---

## [v0.5.5] — 2026-05-11

The **"search-with-versions"** release. `amc package search` and a
new `amc package versions <pkg>` verb show every indexed version
of a package with **compat status** against the running amc — no
more guessing which tag to pass to `amc package add`.

### Index schema bump (packages-index v2)

[`amalgame-lang/packages-index`](https://github.com/amalgame-lang/packages-index)
gains a flat `[[version]]` array — one entry per (shortname, tag)
pair, linked to `[[package]]` by the `package` field:

```toml
[[version]]
package           = "duckdb"
tag               = "v0.1.1"
required-amalgame = ">=0.5.4"
```

amc reads these and shows compat per tag during `search` / `versions`
without cloning any repo. Back-compat: amc v0.5.4 and older ignore
the new array; the index keeps the existing `[[package]]` schema.

DuckDB is now registered (was missing in the v0.5.3 / v0.5.4 index).

### New: `amc package search` with versions inline

```
$ amc package search duckdb
✓ duckdb — DuckDB binding — vendored C++ amalgamation (MIT)…
    github.com/amalgame-lang/amalgame-database-duckdb (official, Apache-2.0)
    versions:
      v0.1.1 ✓ (needs amc >=0.5.4)  ← latest compatible
      v0.1.0 ✓ (needs amc >=0.5.3)
```

Each tag shows `✓` / `✗` against the running amc. The newest
compatible one gets a `← latest compatible` marker so the user
knows which `add` invocation to type.

### New: `amc package versions <name>`

```
$ amc package versions sqlite
sqlite — github.com/amalgame-lang/amalgame-database-sqlite
    versions:
      v0.2.0 ✓ (needs amc >=0.5.0)  ← latest compatible
```

Shortcut for `search <name>` filtered to one package. Cheap
discovery: "what versions of X work with my amc?"

### `--refresh` for index cache

`amc package search --refresh` (and `versions --refresh`) drop
the cached `~/.amalgame/cache/packages-index.toml` before re-
fetching. Useful when a new version was just published.

### `amc package list` shows pinned version

```
$ amc package list
1 package(s) installed:

  DuckDB @ v0.1.1 — amalgame-database-duckdb
    namespace: Amalgame.Database.DuckDB
    header:    /home/.../runtime/Amalgame_Database_DuckDB.h
```

The tag now lives on `LoadedPackage.Tag`, populated by
`PackageRegistry.LoadFrom` from the lockfile's `[[package]].tag`.

### `amc package remove` accepts `@<tag>` safety suffix

```
$ amc package remove duckdb@v0.1.1   # safe: verify-then-strip
$ amc package remove duckdb          # bare: strip whatever is pinned
```

When the `@<tag>` suffix is present, amc refuses to remove unless
the installed tag matches. Protects against typo'ing the wrong
version after an `amc package update`.

---

## [v0.5.4] — 2026-05-11

The **"precompile-on-install + cross-platform home"** release. Two
groups of work that share a single new helper, hence one PR.

### Package manager — precompile-on-install

Heavy C/C++ packages (DuckDB, future Postgres static builds) can
now declare:

```toml
[stdlib]
precompile = true
```

When that flag is set, `amc package add foo@vX.Y.Z` compiles each
`[stdlib].sources` entry **at install time** into a persistent
cache at:

```
~/.amalgame/packages/<host>/<owner>/<repo>/<tag>_<sha>/
└─ build/<platform>/<class>-<basename>.o
```

`amc test` and `amc build` look there first (before the v0.5.2
`/tmp/amc-pkg-*.o` lazy cache), so the 5-minute DuckDB compile is
paid **once** at install instead of every fresh `amc test`. Survives
reboots. Wiped by `amc package remove`.

Opt-out: `amc package add foo@vX.Y.Z --no-precompile` to skip even
if the manifest says yes (useful in CI batch installs where you
want install separated from build).

### Calibration — auto-learning ETA

Each precompile writes a sample to `~/.amalgame/calibration.toml`:

```toml
[[sample]]
lang      = "cxx"
size_kb   = 25500
elapsed_s = 167
pkg_ver   = "amalgame-database-duckdb@v0.1.0"
```

Future `amc package add` reads these samples and shows an ETA
derived from the weighted average (lang-bucketed) on this specific
machine. No hardcoded "5 min" lie in manifests — the estimate
self-corrects to your CPU over a few installs. First-time users
get an honest "no ETA yet" message plus elapsed time after.

### Cross-platform `$HOME` resolution

`PackageRegistry.AmalgameHome()` resolves the user state dir in
order:

1. `$AMALGAME_HOME` (explicit override)
2. `$HOME` (POSIX, MSYS2, Cygwin, WSL — always set)
3. `$USERPROFILE` (Windows native cmd.exe / PowerShell)

Fixes a pre-existing bug where amc fell back to `/tmp` on Windows
natively (cmd.exe / PowerShell don't set `$HOME`). MSYS2-only CI
wasn't exercising this. All path concatenation now goes through
`Path_Combine` which handles both `/` and `\` separators.

### Other

- New `PackageRegistry.PlatformTag()` returns lowercased `os-arch`
  (`linux-x86_64`, `macos-arm64`, `windows-x86_64`) so multi-OS
  cache layouts don't collide on a shared `$HOME`.
- 10 new stdlib tests cover precompile / PkgDir / platform tag /
  Calibration round-trip (240 PASS total, up from 231).

---

## [v0.5.3] — 2026-05-11

The **"C++-bearing packages"** release. Lays the wiring needed
to ship vendored C++ amalgamations (DuckDB landing first) without
forcing the user to know anything about g++ or libstdc++.

### Package manager — manifest schema growth

Four new optional keys land on `[stdlib]`:

```toml
[stdlib]
sources  = ["foo.cpp"]                  # extension drives g++ vs gcc
cflags   = "-DFOO=1"                    # extra flags for .c sources
cxxflags = "-O3 -DNDEBUG -std=c++17"    # extra flags for .cpp/.cc/.cxx
libs     = ["stdc++", "m"]              # bare names → -l<name> at link
```

Plus one on `[package]`:

```toml
[package]
schema-version = 1   # rejects installs when amc supports < value
```

`PackageRegistry.LoadedPackage` carries the new fields; `amc package
add` refuses to install a package declaring a higher
`schema-version` than the running amc understands (parallel to the
existing `required-amalgame` gate).

### `amc test` — C/C++ dispatch

`Program.PreCompilePackageSources` now switches between `gcc` and
`g++` based on file extension (`.cpp` / `.cc` / `.cxx` → g++). When
any installed package contributes a C++ source, `RunTest` switches
the final-link compiler from `gcc` to `g++` so libstdc++ + C++
static-init order resolve correctly (a plain `gcc -lstdc++` doesn't
guarantee the latter). Per-package `cflags` / `cxxflags` get spliced
into the per-source pre-compile; `libs` get spliced as `-l<name>` at
the final link.

### Standard library

- `Amalgame.Database.DuckDB` — DuckDB binding, shipped as a new
  external package `amalgame-lang/amalgame-database-duckdb` (vendored
  C++ amalgamation, MIT). Embedded analytical (OLAP) database. See
  the [stdlib chapter](docs/guide/04-stdlib.md#databaseduckdb--embedded-analytics-via-opt-in-package).

### Gotchas retired

- **`amc test` doesn't link C++ packages** — gone in v0.5.3 (this
  release). The dispatch + per-package flags + libs make the path
  symmetric to v0.5.2's C-only auto-link.

---

## [v0.5.2] — 2026-05-11

The **"package CLI polish + amc test linking"** release. Two
batches of work since v0.5.0 land under one tag (v0.5.1 was never
cut as a standalone tag — its content is rolled into v0.5.2):

### CLI grouping — `amc package <action>` (PR #303)

The flat verbs `amc add` / `amc remove` / `amc search` / `amc list` /
`amc update` / `amc cache` are now grouped under a single
subcommand, dotnet-style, with `amc pkg` as a short alias:

```
amc package add github.com/amalgame-lang/amalgame-database-sqlite@v0.2.0
amc package list
amc package search redis
amc pkg cache gc
```

**Breaking** vs v0.5.0: the flat forms are removed. Update any
scripts or CI that called `amc add` to `amc package add`.

### `amc test` is package-aware

- **Auto-install missing deps** (PR #302) — before discovery, the
  runner reads `amalgame.lock`, compares against the local cache,
  and `git clone`s any missing tag into `~/.amalgame/packages/`.
  No more "I cloned a fresh checkout and the tests fail until I
  manually run `amc package add`".
- **Auto-link vendored sources** (PR #304) — packages declaring
  `[stdlib].sources` in their manifest get their `.c` files
  pre-compiled to cached `/tmp/amc-pkg-*.o` objects, then spliced
  into every test binary's `gcc` invocation alongside `-ldl
  -lpthread`. SQLite tests now work out-of-the-box; future
  vendoring backends (DuckDB etc.) get this for free.

### Docs refresh (PR #305)

- `03-cli-reference.md` documents the new `amc package <action>`
  table.
- `04-stdlib.md` SQLite section updated to the new install command
  and explains the automatic linking.
- `07-internals.md` `amc test` runner now documents the pre-compile
  step + `amcRuntime` resolution + `-ldl -lpthread` link flags.
- `ROADMAP_COMPLET.md` ticks the package-manager item off the
  planned list.

### Behind the scenes

- `LoadedPackage` carries a `Sources: List<string>` field populated
  from `[stdlib].sources` in each package's manifest.
- New helper `Program.PreCompilePackageSources(registry,
  amcRuntime)` walks the registry once per `amc test` run.
- `RunTest` resolves `amcRuntime` once up front (env →
  `dirname(amc)` → `./runtime`) instead of per-test-file.

---

## [v0.5.0] — 2026-05-11

The **"package manager + ecosystem launch"** release. v0.5 is the
biggest architectural shift since v0.3 self-hosting: the
monolithic compiler bundle is split into a **lean core** plus an
**opt-in package ecosystem**, with a real package-manager CLI,
manifest format, lock file, and 3 inaugural external packages
shipped alongside this release.

### Headline: `amc add`

```
amc add github.com/amalgame-lang/amalgame-database-sqlite@v0.2.0
```

…clones an external package, validates its manifest, writes the
project's `amalgame.toml` + `amalgame.lock`, and from that point
on `import Amalgame.Database.SQLite` Just Works in user code.

### 3 inaugural external packages

The previously bundled optional backends are now self-hosted in
their own repos with their own CI, their own release cadence,
their own tests:

| Package | Replaces (pre-v0.5) | License |
|---|---|---|
| [`amalgame-lang/amalgame-database-sqlite@v0.2.0`](https://github.com/amalgame-lang/amalgame-database-sqlite) | bundled SQLite in v0.4.15 | Apache-2.0 over public-domain SQLite |
| [`amalgame-lang/amalgame-database-nosql-redis@v0.2.0`](https://github.com/amalgame-lang/amalgame-database-nosql-redis) | bundled Redis in v0.4.17 | Apache-2.0 |
| [`amalgame-lang/amalgame-messaging-mqtt@v0.2.0`](https://github.com/amalgame-lang/amalgame-messaging-mqtt) | bundled MQTT in v0.4.17 | Apache-2.0 |

User-facing Amalgame source is **unchanged** — same
`SQLite.Open(":memory:")` / `Redis.Set(r, k, v)` /
`MQTT.Publish(m, t, p)`. Only difference: the dep is declared
explicitly in `amalgame.toml` instead of being bundled.

### Added — package manager pipeline

- **`Amalgame.Formats.Toml`** (PR #283) — TOML 1.0 subset parser
  + serializer. ~480 LoC pure Amalgame. Supports tables, nested
  tables, inline tables, arrays, basic + literal strings,
  integers, booleans, `#` comments, and `[[foo]]` array-of-
  tables (PR #285, needed for the lock-file format).
- **`Amalgame.Formats.*` umbrella** — `Amalgame.Json` migrated
  to `Amalgame.Formats.Json`. Future XML / YAML / CSV / INI /
  MessagePack / CBOR fit naturally. Breaking for v0.4.x users
  but trivial find-replace; no compat shim.
- **`amc add <git-url>@<tag>`** (PR #284) — package installer.
  Cache layout `~/.amalgame/packages/<host>/<owner>/<repo>/<tag>_<sha>/`.
  Idempotent (re-running is a no-op). Validates manifest at
  install: `[package].name` matches URL slug, `version` matches
  tag, `license` declared, `required-amalgame` constraint is
  satisfied by the running compiler.
- **`PackageRegistry`** (PR #286) — compiler-side loader reads
  `amalgame.lock` + each cached manifest, exposes a typed
  registry. Single source of truth for the resolver + cgen.
- **Resolver wiring** (PR #287) — declares each package's class
  + functions as globals so user code resolves cleanly without
  any per-package hardcoding in `src/resolver/resolver.am`.
- **CGen wiring** (PR #288) — isStdlib short-circuit consults
  the registry; return-type table is manifest-driven; prelude
  `#include` lines are emitted per installed package.
- **End-to-end pipeline test** (PR #289) — fixture exercises
  Toml → PackageRegistry → Resolver → CGen on a fake package
  without any network call.
- **Namespace-mangled C symbols** (PR #295) — package class
  methods lower to the namespace-prefixed C name
  (`Redis.Open` → `Amalgame_Database_NoSQL_Redis_Open`) so two
  packages exposing the same short class can't collide at link
  time. Core stdlib (Console / File / Math / …) keeps its flat
  `Class_Method` symbols — single-author, no risk.
- **`required-amalgame` compat gate** (PR #291) — package
  manifests declare `required-amalgame = ">=X.Y.Z"`. amc
  refuses to install if the running compiler version doesn't
  satisfy it (errors at install instead of mysteriously at
  gcc-link time).

### Removed — optional backends moved to packages

- `runtime/Amalgame_Database_SQLite.h` + the 9MB vendored
  amalgamation (`runtime/Amalgame_Database/sqlite/sqlite3.{c,h}`)
  → `amalgame-lang/amalgame-database-sqlite`
- `runtime/Amalgame_Database_Redis.h`
  → `amalgame-lang/amalgame-database-nosql-redis`
- `runtime/Amalgame_Messaging_MQTT.h`
  → `amalgame-lang/amalgame-messaging-mqtt`
- All hardcoded isStdlib / return-type / DeclareGlobal entries
  for the three classes — replaced by manifest-driven dispatch.
- The corresponding test fixtures and runner blocks — moved to
  each package's own `tests/run_tests.sh`.

### Changed — architectural

- **Main repo binary size** drops from ~470KB to ~370KB —
  9MB of vendored SQLite no longer ships with amc.
- **Main repo CI** tests only the compiler + core stdlib + the
  package-manager pipeline. No external server dependency.
  Clone-and-test runs green out of the box; no Redis, mosquitto,
  or SQLite-dev install required.
- **Each package** owns its own tests, CI workflow, release
  cadence. Bug fixes ship from the package repo without a
  compiler release.

### Tests / infra

- Main repo suite: **205 core / 219 stdlib / 12 fmt / 34 amc-new
  = 470 PASS / 0 FAIL / 0 SKIP** (was 438/24 in v0.4.17 — the
  SKIPs were Redis + MQTT cases waiting for a server, now those
  live in the package repos).
- 14 PRs landed (#282–#295). Develop accumulation per the v0.5
  batching policy — no intermediate patch releases since v0.4.17.

### Migration from v0.4.x

User code at v0.4.17 today:

```amalgame
import Amalgame.Database.SQLite
let db = SQLite.Open(":memory:")
```

…keeps working at v0.5.0 — but the dep must now be declared:

```bash
amc add github.com/amalgame-lang/amalgame-database-sqlite@v0.2.0
```

Same for Redis + MQTT. Json users update one import:
`import Amalgame.Json` → `import Amalgame.Formats.Json`. No
compat shim — pre-1.0 language, breaking renames are documented.

### Design doc

Full design captured in [`docs/proposals/amalgame-package-manager.md`](docs/proposals/amalgame-package-manager.md).

### Roadmap — v0.5.x + v0.6

- **v0.5.x** patch follow-ups (no breaking):
  - Transitive dep resolution + cycle/conflict detection
  - `amalgame-lang/packages-index` shortname lookup
    (`amc add redis@v0.2.0`)
  - `amc remove` / `update` / `list` / `search`
  - Path deps (`{ path = "../foo" }`)
  - Semver range constraints (`^X.Y.Z`, `~X.Y.Z`)
- **v0.6**: multi-version coexistence (Cargo-style dual-link
  enabled by the v0.5 mangling).

---

## [v0.4.17] — 2026-05-11

The "Redis client + LSP folding" release. Closes the last
explicitly-requested LSP slice (foldingRange, slice 5) and ships
the second `Amalgame.Database.*` brick after SQLite — this time
a pure-protocol client with no vendored lib, opening the NoSQL
branch of the namespace. Also a small fix to the release-flow
tooling so it stops emitting the `Co-Authored-By: Claude` trailer
that contradicts the project's authorship policy. Suite grows to
**438 PASS / 0 FAIL / 14 SKIP** (the 14 SKIP are Redis cases that
need a running server; the runner gates on a TCP probe so they
SKIP cleanly when no server is up).

### Added

- **`Amalgame.Database.NoSQL.Redis`** (PR #277) — pure-protocol
  RESP2 client over raw TCP, no vendored client lib. The
  runtime header (`runtime/Amalgame_Database_Redis.h`, ~340 LoC)
  reuses the cross-platform socket layer from `Amalgame_Net.h`
  so it works against Redis / KeyDB / Dragonfly / Valkey
  unchanged. Surface: `Redis.Open(host, port)`, `Close`,
  `IsOpen`, `LastError`, `Ping`, `Set`, `Get`, `Del`, `Exists`,
  `Incr`, `Decr`, `Expire`. Namespace nested under
  `Amalgame.Database.NoSQL.<Engine>` so siblings (Mongo,
  Cassandra, DynamoDB) can land alongside. AUTH / SELECT,
  pipelining, pub/sub, MULTI/EXEC transactions, SCAN, TLS,
  binary values with embedded NULs deferred to v2.
- **`amc lsp` foldingRange** (PR #276, slice 5) — token-driven
  brace-pair matching for class / method / block bodies; plus
  runs of `//` comments (`kind:"comment"`) and consecutive
  `import …` statements (`kind:"imports"`). Filters one- and
  two-line blocks so the gutter stays uncluttered. Capability
  advertised as `foldingRangeProvider: true`; VS Code and other
  LSP clients wire the chevron rendering automatically.

### Changed

- **`tools/release.sh`** (PR #275) — drop the hardcoded
  `Co-Authored-By: Claude` trailer from the release-commit
  message it generates. Aligns the script with the project's
  authorship policy stated in `NOTICE.md` (AI is a tool, not
  a co-author at law). The manual v0.4.16 release flow already
  followed this; the patch makes the automated path do the
  same so the next `./tools/release.sh` invocation stays
  compliant without manual edits.

### Roadmap

- **v0.5 — stdlib package manager + DB-backend extraction.**
  Once 3–4 optional backends are in tree (Redis is #2; DuckDB,
  Postgres, MQTT are the natural next), the monolithic-stdlib
  model starts hurting and a real package manager pays off.
  Design space: manifest format (`package.am` /
  `amalgame.toml`), `amc add <repo-url>` CLI that git-clones
  into `~/.amalgame/packages/`, resolver search path,
  cgen-plugin manifest replacing today's hardcoded isStdlib
  list, linker flags + vendored .c amalgamations declared per
  package. Inaugural packages: `amalgame-lang/amalgame-sqlite`
  and `amalgame-lang/amalgame-redis`, both extracted from the
  current monolithic stdlib as proof the design holds against
  two distinct shapes (vendored amalgamation vs pure protocol).

### Tests / infra

- Suite grows to **438 PASS / 0 FAIL / 14 SKIP** (was 434).
  +4 LSP cases for the new foldingRange handler, +14 Redis
  cases gated on a TCP reachability probe.
- Snapshot refreshed.

---

## [v0.4.16] — 2026-05-11

A small roll-up release that lands the post-v0.4.15 develop
accumulation: a roadmap expansion across the database + messaging
families, a stale-doc refresh, and one dead-script removal. No
code changes — the test suite stays at **434 PASS / 0 FAIL / 0
SKIP** and the snapshot is refreshed unchanged.

### Changed

- **`ROADMAP_COMPLET.md`** (PRs #270, #271) — fleshes out two
  stdlib families that previously sat as one-liners:
  - `Amalgame.Database.<Engine>` siblings now spell out Oracle
    (Instant Client dynamic-link, proprietary download) and SQL
    Server (MS ODBC default + FreeTDS fallback) alongside the
    already-tracked DuckDB / Postgres / MySQL. The NoSQL branch
    adds MongoDB (libmongoc + libbson), Redis (pure-Amalgame
    RESP3 client ~300 LoC), DynamoDB / Cosmos DB / Firestore
    (HTTP over Net.Http + Json), Cassandra / ScyllaDB (CQL
    binary protocol).
  - New `Amalgame.Messaging.<Broker>` family — pure-Amalgame
    MQTT (~300 LoC) + NATS Core (~250 LoC), plus dynamic-link
    backends for Kafka (librdkafka) and RabbitMQ (librabbitmq
    AMQP). Sits parallel to the database namespace so brokers
    and storage engines stay separately namespaced.
- **`CONTINUATION.md`** (PR #272) — refreshed for v0.4.15. The
  file had drifted to v0.4.4 state (Path / Logging / Service /
  Database.SQLite were all missing from the stdlib block); now
  reflects the current 434-test self-hosted state, the four
  closed LSP slices, the `amc new` service template, and the
  authorship / contribution policy bullets.

### Removed

- **`CLEANUP.sh`** (PR #269) — referenced bootstrap artefacts
  (`build/`, `stage1/`, `stage2/`) that haven't existed since
  the self-hosting transition. Dead code; deleting it is a net
  reduction in "wait, what does this do?" surface area.

### Tests / infra

- No code changes; snapshot refreshed via `tools/save-snapshot.sh`
  for consistency with the bumped `--version` string.
- Test suite: **434 PASS / 0 FAIL / 0 SKIP** (unchanged).

---

## [v0.4.15] — 2026-05-11

The "embedded database lands" release. Fourth stdlib brick after
Path / Logging / Service, plus a cluster of governance + ergonomics
upgrades — NOTICE.md, CONTRIBUTING.md, an auto-close workflow for
external PRs, and the docs/guide refresh.

### Added

- **`Amalgame.Database.SQLite`** (PR #266) — embedded SQL via the
  vendored SQLite amalgamation
  (`runtime/Amalgame_Database/sqlite/sqlite3.c` + `sqlite3.h`,
  public-domain dedication). No `libsqlite3-dev` package needed
  on any OS; the amalgamation compiles directly into the user
  binary. Surface: `SQLite.Open(path)`, `Close`, `IsOpen`,
  `Exec(sql)`, `QueryAll(sql)` → `List<List<string>>`,
  `LastInsertId`, `Changes`, `LastError`. Namespace nests under
  `Amalgame.Database.<Engine>` so sibling backends (DuckDB,
  Postgres, MySQL) can land alongside without conflicts.
- **`.github/workflows/auto-close-external-prs.yml`** (PR #265)
  — auto-closes PRs opened from forks with a polite pointer to
  `CONTRIBUTING.md`. Detects external-vs-internal via the
  head-repo / base-repo mismatch, so `release/*` / `feat/*` /
  `docs/*` branches pushed by the maintainer skip the hook.
- **NOTICE.md** (PR #264, expanded here) — clarifies authorship
  (sole author = Bastien Mouget; the AI used during development
  is a tool, not a co-author at law), a "You are free to use
  Amalgame" plain-language Apache-2.0 grant summary, and a full
  third-party-licence audit (bdwgc / libcurl / SQLite / NSSM /
  host compilers) with compatibility notes for each.
- **CONTRIBUTING.md** (PR #264) — external contributions paused
  while the project is solo-developed; Issues stay open for bug
  reports; forks allowed per Apache-2.0.
- **docs/guide/04-stdlib.md** — Database.SQLite section with
  worked example + linking instructions + v1 limitations
  (no parameter binding yet, text-only columns, no explicit
  transactions — all tracked for v2).

### Changed

- **`tests/run_stdlib_tests.sh`** precompiles the SQLite
  amalgamation once at startup (~10s) and links the resulting
  `.o` into every stdlib test binary. Per-test overhead stays
  flat; the .o is dead-code-eliminated for non-Database tests.

### Roadmap

- `Amalgame.Database.<Engine>` siblings — DuckDB (vendored
  amalgamation), Postgres + MySQL (dynamic-link to system libs).
- `Amalgame.Database.SQLite` v2 — parameter binding via `?`
  placeholders, typed column accessors (`row.AsInt(0)`), prepared-
  statement reuse, transactions.
- `Amalgame.Service` v2 — native Windows SCM dispatcher, drop the
  NSSM dependency on Windows.

### Tests / infra

- Suite grows to **434 PASS / 0 FAIL / 0 SKIP** (was 421). 13 new
  cases cover open, DDL, DML, last-insert-id, changes counter,
  query+columns, aggregate, error path, delete, close. End-to-end
  on an in-memory database.
- Repo size: `+10MB` for the vendored SQLite amalgamation.
  `linguist-vendored=true` in `.gitattributes` keeps the lines
  out of GitHub's language stats.
- Snapshot refreshed.

---

## [v0.4.14] — 2026-05-10

The "service scaffolder lands" release. Path + Logging + Service
stdlib bricks (v0.4.11–v0.4.13) all came together to unblock
`amc new --template service` — one command generates a full daemon
project with cross-platform install paths.

### Added

- **`amc new --template service`** (PR #261) — fourth `amc new`
  template. Generates:
    - `src/main.am` with the canonical `Service.Install()` /
      `while !ShouldStop()` / `Log.Info` loop pattern.
    - **Linux side**: `<name>.service` systemd unit + `install.sh`
      that runs `systemctl daemon-reload`, `enable`, `restart`.
      Logs route to journald; `journalctl -fu <name>` to tail.
    - **Windows side**: `install.ps1` registers the binary as a
      Windows service via [NSSM](https://nssm.cc) (auto-downloaded
      if missing). NSSM transparently wraps any console binary as
      a proper Windows service: SCM dispatch, status reporting,
      auto-restart, log rotation. Operator interface is identical
      to a native service (`sc start` / `Get-Service` / Event
      Viewer all work).
    - `build.sh` (POSIX `gcc`) + `build.ps1` (MinGW `gcc`). Both
      auto-locate `runtime/` via `AMALGAME_HOME` env var, system
      install dirs, or the `amc` binary's neighbour directory.
    - `README.md` with the full run / install / operate matrix
      for every supported OS, plus a rationale section on why
      NSSM and how the v2 native-SCM path will eventually drop
      the wrapper.

  Native Windows Service mode (`StartServiceCtrlDispatcher` +
  `RegisterServiceCtrlHandler` + `SetServiceStatus`) is tracked
  as v2 — requires a worker-thread split and an amc-side wrap of
  generated `main()`. NSSM is the v1 pragmatic answer; existing
  NSSM installs will keep working through the v2 transition since
  NSSM-managed services run the same binary either way.

### Changed

- **Logging runtime helpers renamed `Logging_*` → `Log_*`** —
  matches the class facade name so `Log.Info(...)` in user code
  lowers to `Log_Info(...)` via the cgen's `isStdlib` short-
  circuit, same convention as `Console_*` / `File_*` / `Math_*`.
  The `src/stdlib/logging.am` facade is now optional sugar; user
  code reaches the same runtime helpers directly when amc auto-
  includes `Amalgame_Logging.h`. The service template generates
  code that uses this direct path — no facade import needed.
- **`Log` and `Service` added to the cgen's stdlib short-circuit
  list** plus `DeclareGlobal("Log", "type", ...)` /
  `DeclareGlobal("Service", "type", ...)` in the resolver, so
  the short-syntax usage (`Log.Info(...)` / `Service.Sleep(...)`)
  resolves cleanly without importing the facade source.

### Roadmap

- `Amalgame.Service` v2 — native SCM dispatcher (drop NSSM
  dependency on Windows).
- `amc new --template service` v2 — macOS launchd plist +
  `launchctl bootstrap` wrapper.
- `Amalgame.Database` — SQLite via libsqlite3 (next stdlib brick).

### Tests / infra

- Suite grows to **421 PASS / 0 FAIL / 0 SKIP** (was 407). 12
  new `amc new` cases cover the service template (file presence,
  generated `main.am` references the right APIs, systemd unit
  has the right `ExecStart`, `install.ps1` references NSSM).
- End-to-end roundtrip verified locally — `amc new`, `./build.sh`,
  run the binary, observe `INFO myd starting` / `INFO tick 0`,
  SIGTERM, observe `INFO myd shutting down cleanly`.
- Snapshot refreshed.

---

## [v0.4.13] — 2026-05-10

The "Service stdlib lands" release. Third stdlib brick after Path
(v0.4.11) and Logging (v0.4.12); last pre-requisite before the
`amc new --template service` scaffolder.

### Added

- **`Amalgame.Service`** (PR #256) — long-running process
  primitives wrapping POSIX `signal()` + `nanosleep()` (Linux/
  macOS) and `SetConsoleCtrlHandler` + `Sleep()` (Windows) behind
  the same Amalgame surface. Callers don't branch on platform.

  Surface: `Service.Install()`, `Service.ShouldStop()`,
  `Service.RequestStop()`, `Service.Sleep(ms)`. The typical loop:

      Service.Install()
      while (!Service.ShouldStop()) {
          Log.Info("heartbeat")
          Service.Sleep(5000)
      }

  Shutdown flag is `sig_atomic_t` on POSIX, `LONG` + `Interlocked-
  Exchange` on Windows — async-signal-safe and atomic respectively,
  no mutex needed for v1 single-process scope. POSIX `Service.
  Sleep` uses `nanosleep` (interruptible via EINTR); Windows
  slices into 100ms chunks (no portable equivalent for console
  signals).

### Roadmap

- `amc new <name> --template service` — now unblocked. Scaffolds
  a project using `Amalgame.Service` + `Amalgame.Logging` with
  a `<name>.service` systemd unit + `install.sh` for Linux v1.
- `--template forms` (cross-platform GUI) — SDL2 binding work
  tracked separately.

### Tests / infra

- Suite grows to **407 PASS / 0 FAIL / 0 SKIP** (was 403). 4 new
  cases drive the programmatic shutdown path (`RequestStop` flips
  the same flag the OS would). Direct SIGTERM delivery to the
  test runner would kill the suite, so the test exercises the
  post-flag state instead.
- Snapshot refreshed.

---

## [v0.4.12] — 2026-05-10

The "Logging stdlib lands" release. v0.4.11 added `Amalgame.Path`;
v0.4.12 adds the second stdlib brick on the way to `amc new
--template service`: leveled logging with a stderr + optional
file sink.

### Added

- **`Amalgame.Logging`** (PR #253) — `public class Log` with four
  levels (Debug / Info / Warn / Error), UTC ISO 8601 timestamps,
  fixed-width labels, optional file sink. Configuration is
  process-wide singleton state held in the runtime (`Logging_*`
  helpers), same pattern as `Exit_Set` / `Exit_Get`.

  Level names are case-insensitive on the first letter — "DEBUG",
  "Debug", "debug" all map. Unknown names default to "info"
  silently; logging itself should never crash a process. File
  sink reopens on each emit — slower than holding a stream open
  but robust against external log rotation, which is the typical
  production setup.

  Single-process, thread-unsafe v1 — fine for CLIs and single-
  threaded servers. Mutex is the v2 ask once a real multi-threaded
  user lands.

### Roadmap

- `Amalgame.Database` — minimum SQLite via libsqlite3.
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders. Now unblocked on the
  Path + Logging side.

### Tests / infra

- Suite grows to **403 PASS / 0 FAIL / 0 SKIP** (was 393). 10 new
  cases cover level round-trips, case-insensitive parsing,
  unknown-level fallback, file sink round-trip, and emit + level
  filter via the file sink.
- Snapshot refreshed.

---

## [v0.4.11] — 2026-05-10

The "Path stdlib lands" release. v0.4.10 closed LSP slice 4;
v0.4.11 pivots to the stdlib backlog and ships the first of
several gaps: cross-platform path manipulation as a proper
`namespace Amalgame.Path` facade.

### Added

- **`Amalgame.Path`** (PR #250) — `public class Path` exposing
  Combine, Extension, Filename, Directory, Stem, IsAbsolute,
  Normalize, and Sep. Wraps the existing C runtime `Path_*`
  helpers and adds four new operations (`Stem`, `IsAbsolute`,
  `Normalize`, `Sep`) for the gaps the runtime didn't cover.
  Normalize follows Go's `filepath.Clean` semantics — pure
  string operation, doesn't touch the filesystem, so it works
  on non-existent paths. Output canonical separator is always
  `/` — Windows accepts forward slashes everywhere, and a
  deterministic canonical form is what callers expect.

### Roadmap

- `Amalgame.Logging` — structured logging with levels, JSON +
  text formatters, console + file sinks. Tracked alongside Path
  as a pre-requisite for `amc new --template service`.
- `Amalgame.Database` — minimum SQLite via libsqlite3.
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders.

### Tests / infra

- Suite grows to **393 PASS / 0 FAIL / 0 SKIP** (was 372).
  21 new cases under `tests/run_stdlib_tests.sh` exercise every
  Path method + the documented edge cases.
- Snapshot refreshed.

---

## [v0.4.10] — 2026-05-10

The "LSP slice 4 closes" release. v0.4.9 finished slice 3 (rename
+ call hierarchy); v0.4.10 ships the next two features that ride on
the typechecker's per-node inference: inferred-type inlay hints and
the lightbulb / Quick Fix UI.

### Added

- **`textDocument/inlayHint`** (PR #246) — walks the open file's
  AST + runs the typechecker, emits one `InlayHint` per `VAR_DECL`
  that has an RHS but no annotation. Hint pinned right after the
  variable name, kind=1 (Type), paddingLeft=true so VS Code renders
  it as a greyed-out `name : type` decoration. Skips annotated
  decls, tuple destructuring, and decls whose inferred type is `?`.
- **`textDocument/codeAction`** (PR #247) — first quick fix lands:
  *Add type annotation*. When the cursor (or selection start) is on
  a `let x = …` whose annotation is empty and the typechecker can
  infer a concrete type, the server emits a `CodeAction` whose
  `WorkspaceEdit` inserts `: <type>` right after the variable name.
  Reuses the same inference path inlayHint already walks. The
  framework — dispatch + WorkspaceEdit emission — also unblocks
  future quick fixes (typechecker-driven annotation correction,
  unused-local removal, top-level-fn wrap-in-class) without
  another round-trip through the parser.

Capabilities advertised:
  `inlayHintProvider: true`
  `codeActionProvider: true`

### Roadmap

- Slice 5 LSP — tighter `selectionRange` (parser nameStart hook),
  more code actions (driven by linter / typechecker diags).
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot refreshed
  twice across the cycle (one per LSP-touching PR).
- `tests/run_tests.sh` lsp-init expected string updated for
  `inlayHintProvider:true` and `codeActionProvider:true`.

---

## [v0.4.9] — 2026-05-10

The "LSP slice 3 closes" release. v0.4.8 made navigation fast and
broad; v0.4.9 fills in the IDE features that ride on top of the
already-walked AST: rename's pre-flight check (so F2's UI stops
silently failing on punctuation) and the Call Hierarchy panel
(Shift+Alt+H).

### Added

- **`textDocument/prepareRename`** (PR #243) — F2's pre-flight
  check. Returns `{ range, placeholder }` when the cursor is on a
  renameable token (IDENTIFIER / MEMBER / CLASS_DECL / ENUM_DECL /
  METHOD_DECL / VAR_DECL / PARAM with a non-empty Name), null
  otherwise. VS Code now shows the proper "you cannot rename this
  element" message on punctuation/whitespace instead of silently
  noop-ing.
- **`textDocument/prepareCallHierarchy`** + **`callHierarchy/incomingCalls`**
  + **`callHierarchy/outgoingCalls`** (PR #243) — exposes the
  caller/callee graph for the method enclosing the cursor. Shift+Alt+H
  in VS Code lights up; the outgoing pass walks the method body
  collecting CALL nodes grouped by callee Name; the incoming pass
  walks every program for CALL nodes whose callee matches `item.name`
  and groups call sites by their enclosing METHOD_DECL. Items
  roundtrip name/uri/line/character via the spec's `data` field so
  follow-up requests re-locate the method without another search.

### Roadmap

- Slice 4 LSP work: tighter `selectionRange` (parser nameStart
  hook), inlay hints (parameter names, inferred types), code
  actions (organize imports, "extract to method").
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot refreshed
  for the new cgen output.
- `tests/run_tests.sh` lsp-init expected string updated for
  `renameProvider:{prepareProvider:true}` and `callHierarchyProvider:true`.

---

## [v0.4.8] — 2026-05-10

The "LSP gets fast + comprehensive" release. v0.4.7 made Cmd+Click
work; this release makes it instant, expands navigation to the
features users hit constantly (Find All References, Outline,
project-wide symbol search, parameter / local jumps), and patches
the VS Code extension wart that broke the natural `~/`-style
`serverPath` value.

### Added

- **`amc lsp` workspace resolver caching** (PR #232) — every
  hover / completion / definition / references request used to
  re-parse every `.am` in the workspace from scratch. On a 30-
  file repo that's 2.7s per Cmd+Click; VS Code shows the spinner.
  Now sibling files are parsed once on first use and reused
  across requests; the open file's freshly-parsed AST overrides
  the cached entry per request so editor-fresh text always wins.
  Probe before / after on three sequential definition calls:

      def #1: 0.061s    def #2: 0.037s    def #3: 0.040s

  ~45–70× speedup. Cache is keyed on the resolved workspace
  root; navigating to a different project rebuilds it once.
- **`textDocument/documentSymbol`** (PR #235) — Outline panel,
  breadcrumbs, and Ctrl+Shift+O fuzzy-find now light up for
  every `.am` file. Top-level: classes (kind 5), enums (kind 10).
  Class children: methods (kind 6), fields (kind 8). Enum
  variants surface as kind 22 (EnumMember).
- **`textDocument/typeDefinition`** + **`workspace/symbol`**
  (PR #236) — Ctrl+T project-wide symbol search across every
  cached `.am` file (case-insensitive substring filter), and
  Cmd+K F12 "Go to Type Definition" advertised. Type definition
  is identical to definition for now; diverges once the
  typechecker exposes per-node inferred types.
- **`textDocument/references`** (PR #237) — Find All References
  / Shift+F12 walks every program, recursively descends through
  every AstNode child slot (`Left/Right/Cond/Body/Else/Children/
  Params/Args`), collects every IDENTIFIER / MEMBER whose Name
  matches the symbol under the cursor. Probe shape on `NodeKind`
  in `parser/ast.am`: 433 hits across 9 files in 220ms.
- **`textDocument/definition` resolves params + local var-decls**
  (PR #238) — first pass searches the enclosing method's Params
  + recursively-walked Body for VAR_DECLs matching the lookup
  name, then falls through to the workspace top-level walk. Bug
  report: clicking on `model` at an assignment site in
  `migrate.am` returned null because `model` is a local. Now
  jumps to the `var model: string = ""` declaration.

### Fixed

- **VS Code extension: `amalgame.serverPath` trim + tilde-expand**
  (PR #234) — `child_process.spawn` doesn't run through a shell,
  so neither incidental whitespace nor `~/` were processed. A
  setting like `' ~/.local/bin/amc'` (leading space, tilde) failed
  with ENOENT and the LSP never started. New `resolveServerPath`
  helper trims, replaces a leading `~` with `os.homedir()`, falls
  through to `'amc'` if the resulting string is empty. Bumped
  extension version to 0.2.1.

### Roadmap

- Slice 3 LSP work staying on the roadmap: tighter
  `selectionRange` (parser nameStart hook), member-def in
  definition (jump to a method body when clicking on
  `obj.method()`), rename, call hierarchy, inlay hints, code
  actions. Each is sized to land as a small follow-up PR.
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders. Roadmap entries
  expanded in PR #232 with the SDL2 / systemd / launchd / SCM
  outline.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot
  refreshed multiple times across the cycle (each LSP-touching
  PR + the cgen patches needed a re-bootstrap so cold-start
  clones could parse the new sources).
- Run scripts updated to track every new LSP capability advertised
  in the initialize reply (`documentSymbolProvider`,
  `workspaceSymbolProvider`, `referencesProvider`,
  `typeDefinitionProvider`).

---

## [v0.4.7] — 2026-05-10

The "LSP gets useful" release. The IDE experience moves from
"diagnostics + hover + completion, with ~150 spurious typechecker
errors per workspace" to "diagnostics-clean + Cmd+Click navigates
to definition". Plus a one-line resolver fix that turned out to
be the root cause of the long-parked typechecker false-positives.

### Added

- **`textDocument/definition` + `textDocument/declaration`**
  (PR #227) — Cmd+Click jumps to the declaration site of any
  top-level symbol (class, enum, interface) reachable through
  the workspace resolver. Slice 1 of `amc lsp` v2 navigation
  per the roadmap. Slice 2 (member methods, fields, references)
  lands as a follow-up.
- **`Amalgame.Crypto` cleanup-only follow-up tooling**
  — `xs.Set(idx, value)` wired into the cgen as a side effect
  of investigating the resolver bug below. The runtime
  `AmalgameList_set` was already there; the cgen surface is
  new (PR #228).

### Fixed

- **Resolver: `MemberTable.Set` was `Values.Add`-ing on duplicate
  key** (PR #228) — single-line semantic bug that desynced the
  parallel `Keys` / `Values` arrays. Every subsequent lookup
  returned a value belonging to *another* (member, class) pair.
  The build path never tripped on it (each MemberTable instance
  saw each member once); workspace LSP did, because the open
  file is parsed once from didOpen text and *also* picked up by
  the workspace `find` scan, re-collecting its classes. LSP probe
  before / after across the 9 main compiler files: ~150 spurious
  typechecker diagnostics → **0**. Resolves the "37-case spurious
  typechecker on lexer.am via the LSP" item parked in
  ROADMAP_COMPLET since v0.4.4.
- **LSP: `UriToPath` percent-decodes `%XX` escapes** (PR #227) —
  VS Code sends URIs with percent-encoded UTF-8 for any non-ASCII
  byte in the path (`Développement` → `D%C3%A9veloppement`). The
  previous decoder kept the literal bytes, the workspace scan
  walked an inexistent path, the resolver fell back to
  single-file mode, and any cross-file type reference became an
  "Unknown symbol" diagnostic. Self-contained ~50 LoC pure-
  Amalgame decoder + hex helper. On `c_gen.am` alone: ~130
  spurious "Unknown symbol 'NodeKind'" → 0.
- **LSP: definition replies now percent-encode the response URI**
  (PR #229) — the inverse of the request-side fix. VS Code's
  DocumentURI parser silently rejects replies whose `uri` carries
  raw UTF-8 bytes; symptom was Cmd+Click "spinning forever" even
  though the LSP did respond. New `PercentEncodePath` walks the
  reply path byte-by-byte, leaves the URI-unreserved set
  (`A-Za-z0-9-._~/`) literal, percent-encodes everything else.
- **Cleanup: chain-mash typed-local workarounds removed**
  (PR #225) — now that PR #221 (constructor forward-decl) and
  PR #222 (`obj.Field.Method()` / `obj.Method().Method()`
  chain-mash) are in, the workarounds in `lsp.am`, `migrate.am`,
  and `stdlib/datetime.am::InstantResult` are mechanically
  unnecessary. Same PR uncovered + fixed two more cgen type-
  inference gaps (`Get`-handler chain receiver,
  `MethodRetGet` chain receiver) that the workarounds had been
  masking.

### Tooling

- **`tools/release.sh` patched in four spots** (PR #220) after
  walking through it manually for v0.4.5: README sed catches
  both bump patterns, back-merge from main runs before the
  develop → main PR, `--admin --merge` fallback for repos where
  auto-merge isn't available, CHANGELOG-edit prompt prefers
  `$EDITOR` and falls through cleanly when there's no tty.
- **`release-pdf.yml` lmodern fix + `workflow_dispatch`**
  (PR #219) — Ubuntu 24.04's `texlive-fonts-recommended` doesn't
  pull `lmodern`, which pandoc's default LaTeX template needs.
  v0.4.5 missed its PDF; v0.4.6+ have the asset attached on the
  first run. Manual trigger added so older tags can be reprocessed.

### Roadmap

- **`amc lsp` v2 navigation** (PR #226) — full inventory of the
  next-tier LSP features (declaration, type definition,
  references, hover preview, find-all-references, document
  symbols, workspace symbol, call hierarchy, rename, inlay
  hints, code actions). Slice 1 (definition) lands here; the
  rest is sized to drop in as small PRs.
- **`amc lsp` performance — workspace resolver caching**
  (PR #229) — every request currently re-parses the entire
  workspace (~2.5–3s on a 30-file repo). Caching with
  invalidation on didChange is the next perf win.
- **VS Code extension robustness on `serverPath`** (PR #229) —
  ~5 LoC fix in `extension.js`: trim whitespace + expand
  leading `~/` before `child_process.spawn` so the natural
  shell-style values just work.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot refreshed
  several times across the cycle (each cgen-touching PR + the
  resolver bug fix needed a re-bootstrap so cold-start clones
  could parse the new sources).

---

## [v0.4.6] — 2026-05-10

The "compiler quirk-cleanup" release. Three patches to long-known
parser/cgen sharp edges, plus an end-to-end fix to `tools/release.sh`
informed by walking through it manually for v0.4.5. No new user-
facing features; this is purely making the compiler emit cleaner C
and the release flow less twitchy.

### Fixed

- **Parser: `expr >> N` and `expr << N` no longer drop the shift**
  (PR #215). `ParseRelational` was correctly calling `ParseShift` on
  its RHS but went straight to `ParseAdd` on the LHS, so any
  expression starting with a shift got truncated. One-line fix
  restores the intended `equality > relational > shift > add > mul`
  precedence ladder. Cleaned up the `r / 256` workaround in
  `random.am`. `encoding.am` keeps `/` / `*` because the cgen
  doesn't yet preserve precedence parens around mixed `+` / shift
  expressions — separate roadmap item.
- **Parser: top-level `fn` now errors clearly** (PR #221). Before:
  `public fn foo()` outside a class parsed silently as Unknown,
  the body was swallowed, the call site compiled fine, the linker
  failed (or worse, called the C `double` keyword by accident).
  Now: `amc` exits 1 with `Top-level functions aren't supported
  (got 'fn foo' at L:C). Wrap it inside a class as 'public static'.`
  Drive-by: the regular compile path now actually checks
  `par.HasErrors()` after each parse — only `amc fmt` was doing
  it before.
- **CGen: constructor `_new(...)` forward-decls emitted before any
  class body** (PR #221). Before: a constructor whose body called
  `new B(...)` for a class declared later (or even earlier in the
  same file but emitted after) hit `gcc: implicit declaration` +
  `conflicting types` for `App_B_new`. Fix walks every class decl
  at pass-2 entry and emits its `_new` signature before any body.
  +316 forward declarations across the bundled compiler.
- **CGen: chain-mash on `obj.Field.Method()` and
  `obj.Method().Method()`** (PR #222). Two related bugs in
  `EmitCalleeStr` + the call-site self-injection block both caused
  the cgen to mash a member or call expression into the method
  name instead of resolving it through the typed receiver:

      Before                   →  After
      o.Field.Get()            →  Inner_Get(o->Field)
                                 (was: o->Field_Get())
      o.GetInner().Doubled()   →  Inner_Doubled(Outer_GetInner(o))
                                 (was: Outer_GetInner(o)_Doubled())

  The typed-local workarounds (`let mid: T = obj.Field; mid.Method()`)
  in `lsp.am`, `migrate.am`, `stdlib/json.am`, `stdlib/datetime.am`
  are now mechanically unnecessary; cleanup deferred to a separate
  PR to keep the diff focused.

### Tooling

- **`tools/release.sh` patched in four spots** (PR #220) after
  walking through it manually for v0.4.5:
    1. `sed` for README now also bumps the `Current version: **vX.Y.Z**.`
       overview line, not just the `# amc X.Y.Z` comment.
    2. New "Back-merge main into develop" step before the
       develop → main PR — prevents the conflict cascade when main
       carries a merge commit develop never absorbed (the standard
       outcome of any previous develop → main PR with conflict
       resolution).
    3. `--admin --merge` fallback retried on each poll tick if
       `--auto --merge` rejects (this repo's main branch doesn't
       allow auto-merge with merge-commit strategy).
    4. CHANGELOG-edit prompt uses `$EDITOR` if set, falls back to
       the read-from-tty pattern, finally warns + continues if
       neither path works (was hanging non-interactive runs).

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP** — cleanup release,
  no new test surface added. Snapshot refreshed three times during
  the cycle (after each cgen-touching PR) so cold-start clones
  bootstrap from a parser+cgen consistent with the new sources.

---

## [v0.4.5] — 2026-05-10

The "infrastructure cleanup + Crypto" release. Drops the Vala
bootstrap entirely now that the snapshot is the sole bootstrap
entry point, adds `Amalgame.Crypto` (SHA-256 + HMAC), fixes a
silent shift-drop bug in the parser, and wires a release-time
PDF guide build. 14 commits across 6 PRs.

### Added

- **`Amalgame.Crypto`** (PR #213) — SHA-256 (FIPS 180-4) and
  HMAC-SHA-256 (RFC 2104) primitives in two static facades:
  `Sha256` for the bare hash, `Hmac` for keyed authentication.
  Pure-C runtime header `runtime/Amalgame_Crypto.h` (~150 lines,
  no external crypto dep). API:

      Sha256.Bytes(data: List<int>)    -> List<int>   # raw 32 bytes
      Sha256.Hex(data: List<int>)      -> string      # hex
      Sha256.OfString(s: string)       -> string      # UTF-8 → hex

      Hmac.Sha256(key, msg: List<int>)        -> List<int>
      Hmac.Sha256Hex(key, msg: List<int>)     -> string
      Hmac.Sha256OfStrings(key, msg: string)  -> string

  Tested against FIPS 180-4 SHA-256 vectors ('abc', empty,
  100×'a' multi-block) and RFC 4231 HMAC-SHA-256 cases 1–2.
  Documented caveat: constant-time MAC comparison is the
  caller's responsibility.

- **`release-pdf` GitHub Actions workflow** (PR #211) — on every
  `v*` tag, concatenates `docs/guide/0*.md` (chapters 01..08) and
  builds `amalgame-guide-vX.Y.Z.pdf` via pandoc + xelatex,
  attached to the GitHub Release. The amalgame.me /releases page
  picks it up automatically.

### Changed

- **README + guide code blocks** (PR #216) — 61 ` ```amalgame `
  fences swapped to ` ```kotlin ` as a syntax-highlight fallback.
  GitHub Linguist doesn't recognise `amalgame` yet (roadmap entry
  added), and Kotlin shares enough keywords (`let` / `var` /
  `class` / `null` / `for x in xs`) to highlight ~80% of the
  syntax usefully.

### Fixed

- **Parser: `expr >> N` and `expr << N` no longer drop the shift**
  (PR #215). `ParseRelational` was correctly calling `ParseShift`
  on its RHS but went straight to `ParseAdd` on the LHS, so any
  expression starting with a shift got truncated. One-line fix
  restores the intended `equality > relational > shift > add >
  mul` precedence ladder. Cleaned up the `r / 256` → `r >> 8`
  workaround in `random.am`. `encoding.am` keeps `/` / `*` for
  now since its slices appear inside mixed `+` expressions where
  the cgen-precedence-parens issue would change C evaluation
  order — separate roadmap item.

### Removed

- **Vala bootstrap** (PR #210) — `archive/vala-bootstrap/`
  (Vala compiler sources, meson.build, ~13 500 LoC), `compile.sh`
  (Vala build wrapper), `install/release.sh` (meson-based release
  builder, superseded by `release.yml`), top-level `install.sh`
  (stale stub referencing `codec --version` and the pre-rename
  repo URL), `bootstrap/` directory (unused 2 MB Vala binary +
  README describing commands that don't exist), and 6
  `docs/transpiler/*.md` v0.1.0 design docs (described the
  deleted impl). The bootstrap chain is now 2-rung — `./amc →
  ./snapshot/amc` — and a clean clone cold-starts in one `gcc`
  invocation. The pre-removal state is preserved at the
  `vala-bootstrap-final` git tag if revival is ever needed.
  Net diff: 30 files removed, ~13 569 lines deleted.

### Roadmap

- **Stdlib gaps — second tier** (PR #214) — new ROADMAP_COMPLET
  section inventorying modules a complete stdlib usually has
  and Amalgame doesn't yet: Audio (miniaudio binding sketch),
  Database (SQLite), Path manipulation, Logging, WebSocket,
  Filesystem watcher, advanced Math (Vec/Mat/BigInt), other
  serialization formats (TOML/YAML/MessagePack).
- **Submit Amalgame to GitHub Linguist** (PR #216) — long-term
  fix for the markdown highlight fallback; we already have the
  TextMate grammar in `editors/vscode/syntaxes/`.

### Tests / infra

- Suite is now **372 PASS / 0 FAIL / 0 SKIP** (+9 from
  `Amalgame.Crypto`).
- `snapshot/amc_lib.c` refreshed twice during the cycle: once
  after Crypto wiring (new resolver globals), once after the
  parser fix (so cold-start clones don't hit the old shift-drop
  on the new `random.am`).

---

## [v0.4.4] — 2026-05-10

The "stdlib trio" release. Three new modules — `Amalgame.Random`,
`Amalgame.Encoding`, `Amalgame.DateTime` — close the most-cited
gaps in everyday Amalgame code. Plus a release script that
automates the version bump → tag flow so the v0.4.0-style
"forgot to bump src/main.am" mistake can't happen again.

### Added

- **`Amalgame.Random`** (PR #200) — instance-based PCG XSH-RR
  64/32 PRNG. `new Random(seed)` for reproducibility,
  `Random.FromSystem()` for crypto-grade entropy via the OS pool
  (`getentropy` on POSIX, `BCryptGenRandom` on Windows).
  Methods: `NextUInt32`, `NextInt`, `IntRange(min, max)`,
  `Float`, `Bool`, `Bytes(n)`. Static `Random.SystemBytes(n)`
  for one-off salt / nonce generation. Replaces the legacy
  `Math.Random` 8-bit-output LCG for new code; the old API
  stays for compatibility.
- **`Amalgame.Encoding`** (PR #201) — three small static
  facades: `Base64` (RFC 4648 + URL-safe variant), `Hex`
  (lower / upper, case-insensitive decode), `Url`
  (percent-encoding per RFC 3986, plus an `EncodeComponent`
  matching JS's `encodeURIComponent`). 100% pure Amalgame, no
  runtime header. Bytes flow as `List<int>` ∈ [0, 255], so
  encoding `Random.Bytes(n)` output works without a cast layer.
- **`Amalgame.DateTime`** (PR #202) — `Instant` (i64 ns since
  1970-01-01 UTC, range ±292 years), `Duration` (signed ns
  interval), `Stopwatch` (monotonic-clock convenience for
  benchmarks). RFC 3339 strict parse + format (UTC only —
  `+HH:MM` offsets rejected; local time / timezones tracked
  as a v2 follow-up).
- **`tools/release.sh`** (PR #203) — one-shot release script
  that bumps the version everywhere, builds + tests + saves a
  snapshot, opens the release → develop and develop → main PRs
  with `gh pr merge --auto --merge`, polls until each lands,
  then tags from main and pushes. `--dry-run` and `--no-tag`
  flags for inspection. Closes the manual-checklist gap that
  caused v0.4.0's missed bump.
- CI Windows build links `-lbcrypt` for `Random.FromSystem()`'s
  `BCryptGenRandom` call. Same flag added to the Windows release
  artifact build.

### Changed

- `runtime/` gains `Amalgame_Random.h` + `Amalgame_DateTime.h`
  next to the existing headers. The PCG step (`Random_PcgOutput`
  / `Random_PcgAdvance`) keeps the multiply unsigned because
  signed overflow is UB on Amalgame's `int` = i64.
- `docs/guide/04-stdlib.md` gains Random, Encoding, and DateTime
  sections — each with the full method table and the sharp-edge
  caveats (modulo bias for IntRange, RFC-strict `+` handling for
  Url.Decode, ±292-year window for Instant).

### Roadmap (parser/cgen findings, not fixed in this release)

Surfaced while writing the trio; workarounds applied inline,
real fixes tracked in `ROADMAP_COMPLET.md`:

- Parser drops `expr >> N` inside a `let` initializer (lowers
  as `i64 x = expr;`). Workaround: division by 2^N.
- Top-level `public fn(...)` outside a class parses but cgen
  never emits a definition. Workaround: hang the helper on a
  class as `public static`.
- Constructors that instantiate a later-declared class trigger
  an implicit-decl conflict because `_new` forward-decls aren't
  emitted before pass2 bodies. Workaround: pass the dependency
  in as a constructor parameter (used by `InstantResult`).
- 25 stale `BastienMOUGET/Amalgame` URLs scattered across
  runtime/ headers, install/, and archive/ — to be swept after
  this release. Tracked under "URL sweep" in Distribution.

### Tests / infra

- Suite is now **363 PASS / 0 FAIL / 0 SKIP** (was 307 at
  v0.4.3). +56 assertions split as +14 Random / +21 Encoding
  / +21 DateTime.
- Snapshot refreshed; `snapshot/amc_lib.c` is ~16 500 lines
  reflecting the three new modules + the resolver builtins
  for the new runtime helpers.

---

## [v0.4.3] — 2026-05-09

The "Json migration completes" release. Phase 2 final + phase 3
of the Amalgame.Json plan (the compiler + LLM commands now go
through the proper parser, not substring tricks), plus
`amc migrate v3` real cost reporting on top.

### Changed

- **`src/lsp.am`** request dispatcher: switched from the ad-hoc
  `JsonStr` / `JsonInt` substring extractors to a single
  `Json.Parse(body)` walked via structured `Get(...).At(...)`
  chains. Each LSP message is now parsed once with a real JSON
  parser; the helpers are gone.
- **`src/migrate.am`** response parsing: each provider now reads
  the actual JSON shape of its response instead of the first
  `"text":"..."` substring trick:
    - Anthropic → `root.content[0].text`
    - OpenAI → `root.choices[0].message.content`
    - Gemini → `root.candidates[0].content.parts[0].text`
  Each path returns an attributed error if the shape doesn't
  match, instead of a silent empty string.
- **`amc migrate` real cost line** (v3): after a successful
  migration, the real per-call cost is printed alongside the
  pre-flight `--dry-run` estimate. No `~` prefix, no heuristic —
  exact tokens billed, pulled from each provider's `usage`
  object:

      [migrate] wrote src/api.am
      [migrate] cost: 6431 in + 982 out = $0.04 (claude-sonnet-4-6)

  CLI shell-out (`claude`, `custom`) still reports nothing since
  the bill is on the user's subscription / local backend.

### Removed

- `lsp.am::EscapeJsonStr` / `JsonStr` / `JsonInt`
- `migrate.am::JsonEscape` / `JsonExtract` / `JsonExtractText`

Six functions, ~150 LoC out. The compiler ships with one JSON
parser (`Amalgame.Json`) instead of two-and-a-half overlapping
substring matchers.

### Fixed

- The earlier-noted "Json.Parse hangs on 16 KB bodies" turned
  out to be a benchmark artefact — bash `${#body}` counts
  characters, not bytes, so the LSP probe shipped a frame that
  under-counted UTF-8 multibyte content by the byte/char delta.
  Real client traffic uses byte-accurate counts and the parser
  handles a typical 16 KB didOpen body in ~37 ms.

### Documentation

- `docs/guide/08-llm-commands.md` — "Cost estimation" renamed to
  "Cost reporting", documents both the pre-flight `--dry-run`
  estimate and the post-flight real cost.
- `docs/proposals/amc-migrate.md` — v3 cost reporting moved from
  "deferred" to "partial". API streaming via SSE remains the
  only deferred v3 item.
- `ROADMAP_COMPLET.md` — `Promote ad-hoc JSON to a real
  Amalgame.Json module` marked resolved.

---

## [v0.4.2] — 2026-05-09

The "stdlib + DX" release. Adds a real JSON module, a project
scaffolder, and IDE-friendly member completion. Suite goes from
**263/263** to **307/307**.

### Added

- **`Amalgame.Json`** — first-class JSON parser, encoder, and
  accessor surface. Strict RFC 8259 with full escape support
  (incl. `\uXXXX` + surrogate pairs). Class-with-tag `JsonValue`
  with `Is*` / `As*` / `Get` / `At` / `Length` / `Keys`. Static
  `Json.Parse` / `Encode` / `EscapeString` / `Of*` factories.
  Lives in the new `src/stdlib/` directory. Replaces 4 ad-hoc
  substring extractors in the compiler in a future PR (phase 2);
  this release ships the library standalone. Direct prep for
  `amc migrate v3` cost reporting via nested `usage.input_tokens`.
  [PR #182, #183]

- **`amc new <name>`** — project scaffolder à la `cargo new`.
  Three templates: `exe` (default — `src/main.am` + a passing
  test), `lib` (public class skeleton), `test` (test bundle
  only). Each generates a working `build.sh` that locates the
  Amalgame runtime via `$AMALGAME_HOME` / install dirs / `which
  amc`, plus README, `.gitignore`. Path-aware: `amc new
  path/to/foo` works (basename is the namespace stem). Refusal
  on existing dirs unless `--force`. [PR #184]

- **LSP member completion** — `obj.<cursor>` now narrows to the
  methods/fields of the receiver type. Two-step receiver
  resolution: global-symbol lookup (covers `Json.<cursor>`,
  user-class names) then a local-decl text scan covering
  `let x = new T(...)` / `let x: T = ...`. Falls back to the
  global list for `this.<cursor>`, chained calls, builtin C
  modules. [PR #185]

- **Editor integration on install** — roadmap item added (auto-
  wire VS Code `.vsix` / Neovim lspconfig snippet / Helix
  `languages.toml` entry on first install). [PR #181]

### Documentation

- `docs/proposals/amalgame-json.md` — full design doc for the
  JSON module: motivation, type design, API surface, 3-phase
  migration plan, testing strategy, decisions captured in review.
  [PR #182]

- `ROADMAP_COMPLET.md` — LSP enum-lookup item marked resolved
  (verified empirically: 0 "Unknown symbol" diagnostics on
  `lexer.am` via `amc lsp`). New typechecker item parked with
  investigation findings on the spurious "Return type mismatch"
  cases. [PR #181, #186]

### Infrastructure

- Repo transferred from `BastienMOUGET/Amalgame` to
  `amalgame-lang/Amalgame`. All code, docs, install scripts, and
  publication recipes updated to the new URL. GitHub redirects
  the old URLs for ~1 year; this lands canonical references
  ahead of that window. [PR #187]

- `tests/run_stdlib_tests.sh` extended with a 5th-arg
  `extra_inputs` so per-module tests can pull in a stdlib source
  file alongside the sample (used by the Json suite). [PR #183]

- `tests/run_amc_new_tests.sh` added and wired into
  `run_all_tests.sh`. 20 assertions: file/dir presence per
  template, end-to-end compile + run for `exe`, error paths.
  [PR #184]

### Known issues (unchanged from v0.4.1)

- Typechecker reports spurious "Return type mismatch" on enum-
  member returns in some `lexer.am` IF-body shapes (37 cases via
  the LSP). Doesn't reproduce on minimal cross-file repros.
  Investigation findings captured in `ROADMAP_COMPLET.md`; next
  step documented (instrument `CheckReturn`).

---

## [v0.4.1] — 2026-05-09

Patch release. The v0.4.0 binary still printed `amc 0.3.6` from
`amc --version` because `src/main.am`'s hardcoded version string
wasn't bumped before the tag. This release fixes the string and
adds a CONTINUATION.md pitfall (#12) so the next contributor
catches it pre-tag.

No other changes — same code as v0.4.0 modulo the version literal
and the snapshot refresh. CONTINUATION.md adds the pitfall.

---

## [v0.4.0] — 2026-05-09

The "LLM-driven workflows" release. Three new subcommands shipped in
one big push: **`amc migrate`** (translate other languages → Amalgame),
**`amc generate`** (write a new `.am` from a natural-language prompt),
**`amc explain`** (read a `.am`, write prose). Plus a batch of
compiler / parser / CGen fixes that the LLM-generated code surfaced
along the way. Suite goes from **187/187** to **263/263**.

### New subcommands

- **`amc migrate <file|dir>`** *(PRs #149, #158, #161, #162, #156, #166, #167, #169, #170)*
  - Auto-detects 21 source languages by extension (TS, JS, Python,
    Java, C#, Go, Rust, C, C++, Kotlin, Swift, Ruby, PHP, …).
  - Provider abstraction with five built-ins:
      - `claude` (Claude Code CLI shell out — default fallback)
      - `claude-api` (Anthropic HTTP via `Http_PostWithHeaders`)
      - `chatgpt` (OpenAI HTTP)
      - `gemini` (Google AI Studio HTTP)
      - `custom` (delegates to `AMC_CUSTOM_PROVIDER_CMD`, reads stdin
        / writes stdout — wraps any local LLM CLI)
  - Provider auto-selection from env: `ANTHROPIC_API_KEY` →
    `claude-api`, `OPENAI_API_KEY` → `chatgpt`, `GEMINI_API_KEY` →
    `gemini`, fallback `claude` (CLI).
  - Directory recursion. Result cache by SHA-256(source + system
    prompt) at `~/.cache/amalgame/migrate/`. Per-file `amc --check`
    validation (suppressible). Source-size cap (default 2000 lines).
  - Cost estimation in `--dry-run` per provider + model.
- **`amc generate "<prompt>"`** *(PR #164)* — same provider stack,
  output to stdout by default. `--stream` flows the response live
  via the claude CLI.
- **`amc explain <file.am>`** *(PR #165)* — reverse direction.
  `--lang <name>` controls the explanation language.

### Compiler — language

- **Lambda v2.5 — non-int signatures** *(PR #142)*. Higher-order
  list operations like `xs.Map(x => x.Name)` over a `List<Class>`
  now work end-to-end. The TypeChecker patches the lambda's `PARAM`
  type from the higher-order callsite (so `x` resolves as `Class`
  inside the body), and the CGen splits `EmitOneLambda` into
  forward + body emission so `lam_N_fn` can dereference fields on
  user struct types.
- **Multi-line method chains** *(PR #154)*. `xs\n    .Filter(...)\n    .Map(...)`
  parses cleanly. `ParsePostfix` peeks past `NEWLINE` when the next
  non-NL token is `.` or `?.`. Statement boundaries (NEWLINE not
  followed by a dot) stay intact.
- **TS-style param syntax** *(PR #152)*. `Foo(id: int)` is accepted
  as an alias for `Foo(int id)`. Same fix also resolves a
  pre-existing infinite-loop on the malformed input. The parser
  param-list while-loops gain a "did the cursor advance?" safety
  belt.
- **CGen auto-qualify implicit field access** *(PR #155)*. Within a
  class method, `Id = id` lowers to `self->Id = id` when `Id` isn't
  a local but is a field of the current class. Lets users from
  C# / TS / Kotlin / Swift skip the explicit `this.` qualifier.

### Stdlib

- **`Env.Get(name)` / `Env.Has(name)`** *(PR #160)*. Wraps
  `Environment_GetVar` / `_HasVar`. Required by the API providers
  to read API keys from the environment.
- **`Http_PostWithHeaders` / `Put` / `Delete` / `Patch` return
  type tracked** *(PR #160)*. `resp.Status` / `.Body` / `.Ok`
  type-check correctly after these calls (previously only
  `Http_Get` / `Post` / `GetWithHeaders` / `GetTimeout` /
  `PostJson` were registered).

### Tooling

- **LSP — workspace-aware diagnostics** *(PR #146)*. `amc lsp`
  now scans every `.am` file under the detected workspace root
  (markers: `.git`, `build_amc.sh`, `package.json`) and feeds them
  to the resolver. Eliminates ~44 false `Unknown symbol` diagnostics
  per file on `src/typechecker.am` (NodeKind, SourceSnippet, …).
  Hover / completion benefit too.
- **`amc migrate --help` / `-h`** *(PR #156)* — the discoverability
  flag actually prints help instead of erroring.

### Distribution

- **Release artifacts now bundle `docs/language/grammar.ebnf` +
  `docs/guide/02-language-tour.md`** under `share/amalgame/docs/...`
  so `amc migrate / generate / explain` find them at
  `<exec-dir>/../share/amalgame/docs/...` post-install. Without
  these, the LLM commands degrade gracefully to an inline
  conventions block.
- `install.sh` copies them to `$PREFIX/share/amalgame/docs/`.
- `build_amc.sh` now links `gen_test` against `-lcurl` (the bundled
  `migrate.am` pulls in curl symbols transitively).

### Documentation

- New chapter **`docs/guide/08-llm-commands.md`** — user-facing
  reference for migrate / generate / explain.
- New design doc **`docs/proposals/amc-migrate.md`** — full v0/v1/v2
  rationale, kept up-to-date with what shipped.
- CONTINUATION.md and ROADMAP_COMPLET.md refreshed.

### Refactor & contributor DX

- **`merge=ours` + `linguist-generated` for generated artefacts**
  *(PR #173)*. `.gitattributes` declares `merge=ours` for
  `snapshot/amc_lib.c`, `snapshot/INFO.md`, and `src/amc_lib.c`
  — the canonical resolution on conflict is "rebuild from
  sources", not three-way merge. Eliminates the manual `--theirs`
  → rebuild → re-snapshot → amend dance that ate ~6 PRs in the
  v0.4 cycle. One-time post-clone setup:
  `git config merge.ours.driver true`. Documented as pitfall #12
  in CONTINUATION.md.
- **CGen: `BoxAsVoid` / `UnboxScalar` helpers extracted** *(PR #174)*.
  The `(void*)(intptr_t)X` boxing dance and symmetric unbox were
  duplicated across ~17 sites; consolidated into two helpers so
  intent reads at a glance. Behavior unchanged — emitted C is
  byte-identical pre / post.
- **Parser-loop audit** *(PR #175)*. Verified that every `while`
  loop calling a sub-parser already has a `lastPos` watchdog
  (added preventively in PR #152). No new hazard found.

### Cleanup

- `use.sh` (pre-self-hosted doc snippet) removed.
- `.github/workflow/` typo directory removed (was duplicate of
  `.github/workflows/`, ignored by GitHub anyway).

---

## [v0.3.6] — 2026-05-08

The "lambdas in the wild" release. Higher-order `List<T>`
methods now take a lambda directly: `xs.Filter / .Map / .Reduce
/ .ForEach / .Any / .All / .CountIf`. Suite goes from 180/180
to **187/187** under `./amc`.

### Stdlib

- **Higher-order List methods** — Filter, Map, Reduce (with a
  `(acc, x) → acc` reducer dispatched through `Closure_call2`),
  ForEach, Any, All, CountIf. Each takes an `AmalgameClosure*`
  so the captured environment travels with the lambda.
  Implemented as static-inlines in `runtime/_runtime.h` and
  `runtime/Amalgame_Collections.h`. The closure struct +
  `_callN` wrappers are hoisted to the top of `_runtime.h` so
  the helpers a few lines down can use them; the
  `AmalgamePredicate` / `AmalgameAction` typedefs from v0.3.4
  are gone (no caller left). *(PR #133)*

### Compiler

- **CGen — closure-as-argument lowering** — `TryEmitListCall`
  recognises the seven new method names and lowers each via a
  new `EmitClosureArg` helper. The lambda is passed inline as a
  GCC compound-statement-expression that allocates the env,
  copies captured locals in, and yields a fresh
  `AmalgameClosure_new(...)`. `EmitLambdaCaptureCopy` is a tiny
  helper around the per-capture line so the self-host codegen
  sees `cap.Name` through a typed parameter (chained
  `.Get(i).Name` would erase to `void*` at the call boundary).
- **CGen — type inference for higher-order returns** —
  `InferTypeFromExpr` knows that a CALL whose member name is
  Filter / Map returns `AmalgameList*`, Any / All return
  `code_bool`, CountIf returns `i64`. So
  `let big = xs.Filter(...)` typechecks without an explicit
  annotation, and `EmitVarDecl` propagates the receiver's
  list-element type to the result so `big.Get(0)` lowers with
  the right cast. *(PR #133)*

### Documentation

- `docs/guide/02-language-tour.md` — Lambdas section gains a
  higher-order `Filter / Map / Reduce` example and a rewritten
  limitations note around the i64 boundary.
- `docs/guide/04-stdlib.md` — List section gains the table of
  higher-order methods.
- `docs/guide/05-runtime-and-interop.md` — new "Closures and
  higher-order calls" section with the AmalgameClosure struct,
  the three `_callN` wrappers, and the GCC compound-statement-
  expression idiom the CGen emits.
- `ROADMAP_COMPLET.md` — Lambda v2.5 split into a `[x]` partial
  (this release) + a `[ ]` non-int signatures pending. *(PR #134)*

### Known limitations (deferred to v2.5 final)

- Lambda arguments and results are still `(i64) → i64` at the
  C level. `xs.Map(x => x.Name)` over a `List<Class>` doesn't
  yet work — needs a TypeChecker layer that infers the lambda
  signature from the formal param at the call site, plus CGen
  to emit non-int `lam_N_fn` signatures.
- String interpolation `"x: {coll.Count()}"` doesn't yet
  propagate the inferred `AmalgameList*` to the embedded call.
  Workaround: stage in named locals before printing.
- ForEach mutating an enclosing `var` doesn't accumulate
  (closures capture by value). Reduce is the right tool.

## [v0.3.5] — 2026-05-08

The "developer experience" release. Three coordinated pieces:
**generic interfaces** with static contract verification,
**LSP hover + global completion** on top of the v0.3.4 diagnostics,
and **lambda v2** (multi-param + block body) on top of v0.3.4
single-param closures. Suite goes from **170/170 to 180/180**
under `./amc`.

### Language

- **Generic interfaces** — `interface IComparable<T> { Compare(T) -> int }`
  now type-parameterise. Classes declared `class Box implements
  IComparable<int>` go through a new `CheckImplementsContract`
  pass in the TypeChecker that walks every implemented interface,
  substitutes its generic params by the concrete args (recursing
  into nested generics like `List<T>` → `List<int>`, preserving
  trailing `?` nullability), and asserts every interface method
  exists on the class with the matching signature. Diagnostics
  point at the offending method/param with expected vs got
  types. Two new fields on `AstNode` (`Str3` for generic params
  CSV, `Str4` for the implements list CSV). No vtable / dynamic
  dispatch yet — duck-typed dispatch as before. *(PR #120)*

- **Lambda v2: multi-param + block body** — `(x, y) => x + y`
  and `x => { let d = x * 2; return d + 1 }` now parse and
  compile. Multi-param routes through new `AmalgameClosure_call2`
  / `_call3` runtime helpers; block bodies emit through `EmitBlock`
  with an `InLambdaBody` flag that boxes `RETURN_STMT` values as
  `void*`. The lambda AST node migrates from `Str=name` to
  `Params: List<PARAM>` to support N parameters. v1 closures
  (capturing single-param expression-bodied) keep their existing
  semantics. Lambdas in argument position with non-int signatures
  (e.g. `xs.Filter(x => x > 0)`) need a lambda-typing layer in
  the TypeChecker — that lands in v2.5. *(PR #129)*

### Tooling

- **`amc lsp` hover + global completion** — the LSP server
  shipped in v0.3.4 (diagnostics-only) now responds to
  `textDocument/hover` and `textDocument/completion` too. Hover
  walks the AST via a new `FindNodeAtPosition` helper to find
  the deepest named node covering the cursor, then returns its
  inferred type as Markdown via the new `TypeChecker.LookupNodeType`.
  Completion lists every global the resolver knows about
  (builtins + user classes / enums / functions), mapped to LSP
  `CompletionItemKind` (Class / Function / Variable). The VS
  Code client picks up the new capabilities automatically via
  the initialize handshake — no client changes. Member completion
  (`obj.<cursor>` narrowed to the receiver's type) and
  goto-definition / signature help remain follow-ups. *(PR #126)*

### Documentation

- **Stdlib expansion roadmap** — `ROADMAP_COMPLET.md` gains a
  "Core stdlib expansion" backlog entry listing eight modules
  everyday Amalgame code currently has to fake or shell out for:
  `DateTime`, `Json`, `Regex`, `Random`, `Encoding`, `Compress`,
  `Crypto`, `Threading`. Plus a new "Stdlib delivery model"
  open design question that lays out five alternatives to the
  current header-only-inline approach. *(PR #121)*

### Internal

- **Typed accessors on FullResolver** — `ProgramCount` /
  `ProgramAt` (used by the contract verifier), then `GlobalCount`
  / `GlobalNameAt` / `GlobalTypeAt` (used by the completion
  provider). Both batches work around the same self-host quirk
  where chained member access through a `List<T>` field emits
  `Programs_Count(...)` instead of `Programs->Count(...)`.
- **`-Wint-conversion` hotfix** — `let pn = ...ProgramCount()`
  in `FindInterface` lost its int type to `void*` erasure across
  the method call boundary. Pinned via explicit `let pn: int`,
  matching the precedent in `src/lsp.am` for `String_Length`
  results. Local builds had ignored the warning; CI on Linux
  and macOS treats it as an error. *(PR #124)*

## [v0.3.4] — 2026-05-08

The "tooling" release. Five new pieces ship together: capturing
closures in the language, two new lint checks, a `Process` stdlib
module, an `amc test` runner, and a minimal LSP server (`amc lsp`)
paired with a VS Code client. Suite goes from **150/150 to
170/170** under `./amc`.

### Language

- **Capturing closures** — `let f = x => x + n` now captures `n`
  (and any other enclosing local) by value at creation time, not
  by textual substitution. Replaces the v0.3.x macro path
  (`#define f(x) (n+x)`) which couldn't carry an environment.
  Pieces: a new `AmalgameClosure { fn, env }` runtime type with
  `Closure_new` / `Closure_call1` helpers; capture analysis in
  the resolver (free identifiers below `LambdaBoundary`); a
  per-lambda `LamEnv_N` struct + `lam_N_fn` top-level function
  emitted by a Phase A walk of CGen Pass 2; identifier-callable
  closures dispatched via `AmalgameClosure_call1` with arg/result
  boxing through `intptr_t`. v1 is single-param, expression-bodied,
  arity-1 `(i64) -> i64`. Multi-param, block bodies, lambdas in
  argument position, and non-int signatures are tracked for v2.
  *(PR #101)*

### Tooling

- **`amc --lint` extensions** — two new categories on top of the
  v0.3.3 unreachable-code check:
  - **Unused locals** (`let x = …` never read; prefix `_` to silence)
  - **Shadowed names** (a `let` reuses a name visible in an
    enclosing scope, including method params)

  The linter keeps its own scope stack with append-only
  `UsedNames` + per-local snapshot, sidestepping the missing
  `List<T>.Set` op. Method/lambda params participate in shadow
  detection but are never warned-on as unused. *(PR #104)*

- **`Process` stdlib module** — `Process.Run(cmd) -> int` and
  `Process.RunCapture(cmd) -> AmalgameProcessResult { Exit,
  Stdout, Stderr }`. Cross-platform via `popen`/`_popen`; exit
  codes decoded with `WEXITSTATUS` / `WTERMSIG` (signals
  surface as `128 + signum`, shell convention). Stderr is
  currently merged into Stdout via shell `2>&1`; v2 will split
  them with a real pipe pair. *(PR #106)*

- **`amc test [<dir>]`** — discovers `*_test.am` files under
  `<dir>` (default `.`), compiles + runs each via `Process`, and
  aggregates `[PASS] <name>` / `[FAIL] <name>: <msg>` /
  `[SKIP] <name>` lines from each child's stdout. Crash-with-no-tags
  shows up as `[FAIL] <crash> exit=N`. Tests stay framework-free
  (just normal `Main`s that print tag lines). Exits non-zero on
  any FAIL or compile error. *(PR #109)*

- **`amc lsp` server + VS Code client** — minimal LSP 3.x server
  speaking JSON-RPC 2.0 over stdio. v1 implements `initialize` /
  `shutdown` / `exit`, `textDocument/didOpen` / `didChange` /
  `didClose` (Full sync), and `textDocument/publishDiagnostics`
  (push on every did{Open,Change}). Diagnostics merge resolver
  and typechecker errors and underline the whole token at the
  error column. JSON handling is ad-hoc — `JsonStr` / `JsonInt`
  helpers find `"<key>"` and read the value, no real parser.
  Hover and completion are out of scope for this release.

  Editor side: `editors/vscode/` (now version 0.2.0) gains an
  `extension.js` that spawns `amc lsp` via `vscode-languageclient`.
  Configurable via `amalgame.serverPath` (default `amc`) and
  `amalgame.enableLsp` settings. `npm install` is a one-time
  prerequisite. *(PR #110)*

### Compiler

- **Lexer-bootstrap CR escape fix** — the bootstrapping amc binary
  lexed `"\r"` as the two chars `\` + `r` instead of a single CR
  (0x0d) byte, which made `EscapeStringForC`'s `String_Replace(s,
  "\r", "\\r")` a no-op and propagated through `lsp.am`'s framing,
  causing the LSP wire to emit literal `\r\n` (5c 72 0a) instead
  of CR LF (0d 0a). VS Code's LanguageClient never matched the
  framing and stayed in Starting forever. Fixed by routing CR
  through `String_FromByte(13)`, which is just an int→byte builtin
  and has no escape-table dependency; same idea for `\x1b` (ESC).
  *(PR #110)*

- **CGen return-type table extended** — `Console_{Read,Flush,Write}`,
  `Process_{Run,RunCapture}`, and the `String_*` int/bool returns
  (`Length`, `IndexOf`, `LastIndexOf`, `ToInt`, `StartsWith`,
  `EndsWith`, `Contains`, `IsEmpty`) gain explicit entries. Fixes
  a pre-existing latent bug: an unannotated
  `let n = String_IndexOf(...)` defaulted to `code_string`, breaking
  arithmetic on the result. *(PR #110)*

- **Resolver structured errors** — new
  `RawErrors: List<ResolverError>` kept in lock-step with the
  formatted `Errors: List<string>`. Exposes `(file, line, col, msg)`
  for downstream tools (the new LSP, future IDE features). The
  formatted-string CLI path is unchanged. *(PR #110)*

### Runtime

- **`Console_ReadBytes(n)`** — reads exactly `n` bytes from stdin.
  Used by the LSP framing reader after parsing `Content-Length`.
- **`Console_Flush()`** — drains stdout. Required by the LSP
  server so the client doesn't block on buffered replies.
- **`AmalgameClosure { fn, env }`** + `AmalgameClosure_new` /
  `AmalgameClosure_call1` — backbone of the new closure feature.
- **`AmalgameProcessResult { Exit, Stdout, Stderr }`** + the
  `Process_Run` / `Process_RunCapture` helpers.

### Tests / infra

- Suite is now **170/170** (108 core + 50 stdlib + 12 fmt) under
  `./amc` — +20 vs v0.3.3. New samples: `tests/samples/closures_capture.am`,
  `lint_unused_shadow.am`, `process_api.am`,
  `test_runner/{arith,mixed}_test.am`. New helpers in
  `run_tests.sh`: `run_amc_test_check`, `run_lsp_check`.

### Docs

- `docs/guide/07-internals.md` gains sections on the **test runner**
  (`amc test`) and the **LSP server** (`amc lsp`, `src/lsp.am`).
  The Linter section is rewritten to cover the new unused / shadow
  checks.
- `docs/language/grammar.{ebnf,md}` — full rewrite. The previous
  files dated from v0.1.0 (back when the language was called *CODE*)
  and described constructs the parser never implemented while
  missing everything added since (try/catch, match-as-expression,
  closures, list comp, null-safety, decorators, generics, …). The
  new EBNF mirrors `src/parser/parser.am` exactly.
- README adds bullets for `--lint`, `amc test`, and `amc lsp`.

---

## [v0.3.3] — 2026-05-08

Quality-of-life follow-up to v0.3.2. Cleans up cosmetic typechecker
noise during multi-file compilation, ships a first `amc --lint`
flag (basic static analysis), and stops the workflow from leaking
a stale v0.3.0 artefact into every release. Suite goes from
149/149 to **150/150**.

### Compiler

- **Multi-file false-positive cleanup** — `build_amc.sh` step 1
  invokes `amc` on ten compiler sources at once, and the
  typechecker was emitting ~93 spurious errors on that path
  (build still went through, but the noise drowned out anything
  real). Three small fixes in `CheckProgram` / `CheckReturn`:
  the per-program filename is now read from `prog.Str2` rather
  than the constructor argument; `ExprType*`, `LocalNames`/
  `Types` and `ScopeStarts` reset between programs; and bare
  `return` (parsed as `Return(_unknown_, …)`) is recognised as
  such instead of being treated as a value-return.
  `build_amc.sh` is now silent.

### Tooling

- **`amc --lint`** — new flag that runs a static-analysis pass on
  top of the parsed AST and emits non-fatal warnings. MVP catches
  unreachable code after `return` / `throw` / `break` / `continue`,
  including inside nested `if` / `while` / `for-in` / `try` bodies.
  The skeleton (`src/linter.am`) is set up to grow more checks
  (unused locals, shadowed names, …) without touching anywhere
  else.

### CI

- **Drop stale v0.3.0 artefact, stop tracking `dist/`** — a
  `dist/amc-0.3.0-linux-x86_64.tar.gz` had been committed during
  v0.3.0 and was being uploaded on top of every later release by
  the workflow's `dist/*.tar.gz` glob. Removed, and `dist/` is now
  in `.gitignore` so future stagings don't sneak back in.

### Tests / infra

- Suite is now **150/150** (88 core + 50 stdlib + 12 fmt) under
  `./amc`. New samples: `tests/samples/lint_test.am`. New helper:
  `run_lint_check` in `run_tests.sh`.

---

## [v0.3.2] — 2026-05-08

Closes every open compiler bug tracked in v0.3.1's `SKIP_SELFHOST`
list, restores `try` / `catch` / `throw` / `finally` end-to-end in
the self-hosted parser, and ships the first round of generic type
inference for `List<T>` and `Map<K,V>`. The full test suite is
**149/149 green with zero SKIPs**.

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
- **Null-safety** — three independent bugs fixed: NodeKey collision
  in the typechecker (Kind is now part of the key), bare `?` was
  dropped by the lexer (added `OP_QMARK`), and `TypeToC("T?")` no
  longer doubles up the C pointer.

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

[v0.5.0]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.0
[v0.5.2]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.2
[v0.5.3]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.3
[v0.5.4]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.4
[v0.5.5]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.5
[v0.5.6]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.6
[v0.6.0]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.0
[v0.6.1]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.1
[v0.6.2]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.2
[v0.6.3]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.3
[v0.6.4]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.4
[v0.7.0]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.7.0
[v0.7.1]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.7.1
[v0.7.2]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.7.2
[v0.7.3]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.7.3
[v0.4.17]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.17
[v0.4.16]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.16
[v0.4.15]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.15
[v0.4.14]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.14
[v0.4.13]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.13
[v0.4.12]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.12
[v0.4.11]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.11
[v0.4.10]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.10
[v0.4.9]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.9
[v0.4.8]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.8
[v0.4.7]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.7
[v0.4.6]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.6
[v0.4.5]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.5
[v0.4.4]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.4
[v0.4.3]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.3
[v0.4.2]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.2
[v0.4.1]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.1
[v0.4.0]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.0
[v0.3.6]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.6
[v0.3.5]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.5
[v0.3.4]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.4
[v0.3.3]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.3
[v0.3.2]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.2
