# 4 · Standard library

The stdlib is a thin façade over the C runtime headers in `runtime/`.
All static methods are accessible by their `Class.Method(args)` form
and are mapped to `Class_Method(args)` in the emitted C — so what's
listed here is exactly what's linked.

This chapter documents the public API. For implementation details
see the headers themselves: `runtime/Amalgame_*.h`.

## Console — terminal IO

`runtime/Amalgame_Console.h`

| Method                                     | Effect                                  |
| ------------------------------------------ | --------------------------------------- |
| `Console.WriteLine(s: string)`             | print s + `\n` to stdout                |
| `Console.Write(s: string)`                 | print s to stdout (no newline)          |
| `Console.WriteError(s: string)`            | print s + `\n` to stderr                |
| `Console.ReadLine() : string`              | read a line from stdin (no trailing `\n`) |
| `Console.Clear()`                          | clear the terminal (ANSI sequence)      |

```kotlin
Console.WriteLine("Hi")
Console.WriteError("oops")
let name = Console.ReadLine()
Console.WriteLine("Hello, " + name + "!")
```

## String — text manipulation

`runtime/Amalgame_String.h` · canonical declarations in [stdlib/strings.am](../../stdlib/strings.am)

### Inspection

| Method                                    | Returns | Notes                       |
| ----------------------------------------- | ------- | --------------------------- |
| `String.Length(s)`                        | int     | byte length (not codepoints)|
| `String.IsEmpty(s)`                       | bool    | true for `""` and `null`    |
| `String.IsWhitespace(s)`                  | bool    | true for empty / all-space  |

### Search

| `String.Contains(s, sub)`                 | bool    |                             |
| `String.StartsWith(s, prefix)`            | bool    |                             |
| `String.EndsWith(s, suffix)`              | bool    |                             |
| `String.IndexOf(s, sub)`                  | int     | -1 if not found             |
| `String.LastIndexOf(s, sub)`              | int     | -1 if not found             |

### Slicing & access

| `String.Substring(s, start, len)`         | string  | clamped to bounds           |
| `String.From(s, start)`                   | string  | suffix from index `start`   |
| `String.Until(s, end)`                    | string  | prefix up to index `end`    |
| `String.CharAt(s, i)`                     | string  | one-byte string             |

### Transformation

| `String.ToUpper(s)`                       | string  | ASCII (no Unicode tables)   |
| `String.ToLower(s)`                       | string  | ASCII                       |
| `String.Trim(s)`                          | string  | strip leading + trailing WS |
| `String.TrimStart(s)`                     | string  |                             |
| `String.TrimEnd(s)`                       | string  |                             |
| `String.Replace(s, old, new)`             | string  | replace all occurrences     |
| `String.Repeat(s, n)`                     | string  | concatenate s n times       |
| `String.PadLeft(s, len, ch)`              | string  | (when implemented)          |
| `String.PadRight(s, len, ch)`             | string  | (when implemented)          |

### Splitting / joining

| `String.Split(s, sep) : List<string>`     |         | empty sep → list of one elem|
| `String.Join(parts, sep) : string`        |         | inverse of Split            |

### Conversion

| `String.ToInt(s) : int`                   |         | 0 on parse failure          |
| `String.ToFloat(s) : float`               |         | 0.0 on failure              |
| `String.ToBool(s) : bool`                 |         | true for `"true"` and `"1"` |
| `String.FromInt(n) : string`              |         | decimal                     |
| `String.FromFloat(n) : string`            |         | `%g` format                 |
| `String.FromBool(b) : string`             |         | `"true"` / `"false"`        |
| `String.FromByte(b) : string`             |         | one-byte string from 0..255 |
| `String.FromCodepoint(cp) : string`       |         | UTF-8 encoded codepoint     |

```kotlin
let s = "  Hello, World!  "
let trimmed = String.Trim(s)
Console.WriteLine(String.ToUpper(trimmed))           // → HELLO, WORLD!
let parts = String.Split(trimmed, ", ")              // → ["Hello", "World!"]
Console.WriteLine(String.FromInt(String.Length(s))) // → 19
```

## File / Path — filesystem

`runtime/Amalgame_IO.h`

### Reading

| `File.ReadAll(path) : string`             | full file contents              |
| `File.ReadLine(path, n) : string`         | nth line (when implemented)     |
| `File.Exists(path) : bool`                |                                 |
| `File.Size(path) : int`                   | bytes                           |

### Writing

| `File.WriteAll(path, contents)`           | overwrite                       |
| `File.AppendAll(path, contents)`          | append                          |
| `File.WriteLines(path, lines: List<string>)` | overwrite, line-per-line     |
| `File.OpenWrite(path)`                    | open a global stream (used by gen_test for fast multi-MB output) |
| `File.StreamLine(line: string)`           | append a line to the open stream|
| `File.CloseWrite()`                       | close the stream                |
| `File.Delete(path) : bool`                |                                 |

### Path helpers — legacy flat API

The raw `Path_*` runtime functions are still callable for backwards
compat. New code should use the `namespace Amalgame.Path` facade
instead — see the dedicated [Path](#path--cross-platform-path-manipulation)
section below.

```kotlin
let cfg = File.ReadAll("config.txt")
File.AppendAll("log.txt", "[startup]\n")
let lines = String.Split(cfg, "\n")
File.WriteLines("clean.txt", lines)
```

## Path — cross-platform path manipulation

`namespace Amalgame.Path` exposes a `public class Path` facade
mirroring Python's `pathlib` / Rust's `Path` semantics. Pure
string operations — none of the methods touch the filesystem.
For path lookup that needs to follow symlinks or check existence,
use `File.*` from the previous section.

Separator handling: every method accepts both `/` and `\` on
every platform, mirroring how Windows kernel APIs themselves
behave. Output paths use `/` as the canonical separator (Windows
accepts it everywhere); call `Path.Sep()` if you need the
platform-native byte for shell-out commands or registry strings.

```kotlin
import Amalgame.Path

let cfg: string = Path.Combine("/etc/app", "config.toml")  // "/etc/app/config.toml"
let dir: string = Path.Directory(cfg)                       // "/etc/app"
let ext: string = Path.Extension(cfg)                       // ".toml"
let stem: string = Path.Stem(cfg)                            // "config"
let isAbs: bool = Path.IsAbsolute(cfg)                       // true
let norm: string = Path.Normalize("a/b/../c/./d")            // "a/c/d"
```

| Method                              | Returns  | Notes                                                                       |
|-------------------------------------|----------|-----------------------------------------------------------------------------|
| `Path.Combine(a: string, b: string)`| `string` | Joins with `/`, idempotent on trailing-slash inputs                         |
| `Path.Extension(p: string)`         | `string` | `.gz` for `report.tar.gz` (last extension only), `""` if none               |
| `Path.Filename(p: string)`          | `string` | Last path component (`/a/b/c.txt` → `c.txt`)                                |
| `Path.Directory(p: string)`         | `string` | Parent directory (`/a/b/c` → `/a/b`, bare `c` → `.`)                        |
| `Path.Stem(p: string)`              | `string` | Filename minus last extension (`report.tar.gz` → `report.tar`)              |
| `Path.IsAbsolute(p: string)`        | `bool`   | True for POSIX `/...` or Windows `<drive>:` / UNC roots                     |
| `Path.Normalize(p: string)`         | `string` | Lexical canonical form (collapse `//`, resolve `.`/`..`); empty → `"."`     |
| `Path.Sep()`                        | `string` | `"/"` on POSIX, `"\\"` on Windows                                           |

**Normalize semantics** mirror Go's `filepath.Clean`:

```kotlin
Path.Normalize("a/b/../c")    // "a/c"
Path.Normalize("./a//b")      // "a/b"
Path.Normalize("/usr/../etc") // "/etc"
Path.Normalize("../../foo")   // "../../foo"  (preserved when relative)
Path.Normalize("")            // "."
Path.Normalize("/")           // "/"
```

It does **not** touch the filesystem — so `..` past a symlink may
resolve incorrectly relative to the real path. Use a filesystem-
resolving helper if you need that.

## Math — arithmetic

`runtime/Amalgame_Math.h`

| Method                       | Returns | Notes                       |
| ---------------------------- | ------- | --------------------------- |
| `Math.Sqrt(x: float)`        | float   | `sqrt`                      |
| `Math.Abs(x: float)`         | float   |                             |
| `Math.AbsI(x: int)`          | int     |                             |
| `Math.Pow(x: float, y: float)` | float | `pow`                       |
| `Math.PowI(x: int, y: int)`  | int     |                             |
| `Math.Floor(x: float)`       | float   |                             |
| `Math.Ceil(x: float)`        | float   |                             |
| `Math.Round(x: float)`       | float   | round-half-away-from-zero   |
| `Math.MaxI(a, b) : int`      |         |                             |
| `Math.MinI(a, b) : int`      |         |                             |
| `Math.MaxF(a, b) : float`    |         |                             |
| `Math.MinF(a, b) : float`    |         |                             |
| `Math.ClampI(x, lo, hi)`     | int     |                             |
| `Math.Gcd(a, b)`             | int     |                             |
| `Math.IsPrime(n)`            | bool    |                             |
| `Math.IsFinite(x)`           | bool    |                             |
| `Math.IsNaN(x)`              | bool    |                             |
| `Math.SeedRandom(seed)`      | void    |                             |
| `Math.Random()`              | float   | [0.0, 1.0)                  |
| `Math.RandomInt(lo, hi)`     | int     | [lo, hi]                    |

```kotlin
Math.SeedRandom(42)
let dice = Math.RandomInt(1, 6)
let h = Math.Sqrt(3.0 * 3.0 + 4.0 * 4.0)
```

## Collections — List, Map, Set

`runtime/Amalgame_Collections.h`

### List<T>

| `let xs = new List<int>()`           | constructor                      |
| `xs.Add(x)`                          | append                           |
| `xs.Get(i) : T`                      | indexed access                   |
| `xs.Count() : int`                   | length                           |
| `xs.IsEmpty() : bool`                |                                  |
| `xs.Remove(item) : bool`             | by value (pointer equality)      |
| `xs.RemoveAt(i)`                     | by index                         |
| `xs.Clear()`                         | empty the list                   |
| `xs.Reserve(n)`                      | grow capacity                    |

Higher-order methods (since v0.3.6, take a lambda):

| `xs.Filter(pred) : List<T>`          | keep items where `pred(x)` true  |
| `xs.Map(fn) : List<T>`               | apply `fn` to each item          |
| `xs.Reduce(init, fn) : U`            | left-fold `fn(acc, x) → acc`     |
| `xs.ForEach(action)`                 | run `action(x)` for each item    |
| `xs.Any(pred) : bool`                | true if any item satisfies pred  |
| `xs.All(pred) : bool`                | true if all items satisfy pred   |
| `xs.CountIf(pred) : int`             | count of items satisfying pred   |

```kotlin
let xs = new List<string>()
xs.Add("a") ; xs.Add("b") ; xs.Add("c")
for i in 0..xs.Count() {
    Console.WriteLine(xs.Get(i))
}

// Higher-order
let nums = new List<int>()
nums.Add(1) ; nums.Add(2) ; nums.Add(3) ; nums.Add(4)
let big   = nums.Filter(x => x > 2)              // [3, 4]
let times = nums.Map(x => x * 10)                // [10, 20, 30, 40]
let total = nums.Reduce(0, (acc, x) => acc + x)  // 10
```

Lambda values are still `(i64) → i64` at the C boundary in v0.3.6
— the closure boxes through `intptr_t`. Predicates that return
`bool` work because `false`/`true` round-trip through int (0 / 1).
Non-int signatures (e.g. `xs.Map(x => x.Name)` where the result
is `List<string>`) need the lambda-typing layer that's tracked
for a later release.

### Map<K,V>

| `let m = new Map<string,int>()`      |                                  |
| `m.Set(k, v)`                        | insert or overwrite              |
| `m.Get(k) : V`                       | NULL if missing                  |
| `m.Has(k) : bool`                    |                                  |
| `m.Remove(k) : bool`                 |                                  |
| `m.Size() : int`                     |                                  |

### Set<T>

| `let s = new Set<int>()`             |                                  |
| `s.Add(x)`                           | idempotent                       |
| `s.Contains(x) : bool`               |                                  |
| `s.Remove(x) : bool`                 |                                  |
| `s.Size() : int`                     |                                  |

> Generic element types are erased to `void*` at the C level (see
> chapter 5). Boxing for primitive types uses `(void*)(intptr_t)v`.

## Net — HTTP and TCP

`runtime/Amalgame_Net.h` · backed by libcurl + POSIX sockets

### HTTP

| `Http.Get(url: string) : HttpResponse`              |             |
| `Http.Post(url, body) : HttpResponse`               |             |
| `Http.GetWithHeaders(url, headers) : HttpResponse`  |             |
| `Http.GetTimeout(url, ms) : HttpResponse`           |             |
| `Http.PostJson(url, jsonBody) : HttpResponse`       |             |

`HttpResponse` exposes `Status`, `Body`, `Headers`.

### TCP

| `TcpServer.Listen(port) : TcpServer`     | bind + listen      |
| `TcpServer.Accept() : TcpConn`           | block + accept     |
| `TcpServer.IsListening() : bool`         |                    |
| `TcpServer.Stop()`                       |                    |
| `TcpConn.Send(data: string) : bool`      |                    |
| `TcpConn.Receive() : string`             |                    |
| `TcpConn.Close() : bool`                 |                    |

> Net is the most experimental subset — APIs may evolve.

## Args / Exit — process

Set up at `int main()` time and accessible from Amalgame:

| `Args.Count() : int`            | argc                     |
| `Args.Get(i) : string`          | argv[i] (i=0 is program) |
| `Exit.Set(n: int)`              | mark process exit status |
| `Exit.Get() : int`              | read current exit status |

```kotlin
public static void Main(string[] args) {
    let n = Args.Count()
    Console.WriteLine("argc = {String.FromInt(n)}")
    var i = 1
    while (i < n) {
        Console.WriteLine(Args.Get(i))
        i = i + 1
    }
    Exit.Set(0)
}
```

The `args: string[]` parameter is a vestigial signature — use
`Args.Count()` / `Args.Get(i)` instead.

## Json — parsing, encoding, accessors

`src/stdlib/json.am` · pure-Amalgame implementation, recursive-descent

Strict RFC 8259 parser + encoder + a `JsonValue` accessor surface.
Used internally by `amc lsp`, `amc migrate`, `amc generate`,
`amc explain` to read API responses; available to user code under
the `Amalgame.Json` namespace.

```kotlin
import Amalgame.Json

let body = "{\"users\":[{\"name\":\"Alice\",\"age\":30}]}"
let r = Json.Parse(body)
if (r.Ok) {
    let root: JsonValue  = r.Value
    let users: JsonValue = root.Get("users")
    let u0: JsonValue    = users.At(0)
    let name: JsonValue  = u0.Get("name")
    Console.WriteLine(name.AsString())     // → Alice
}
```

| `Json.Parse(s) : JsonResult`                    | parse, returns ok/err |
| `Json.Encode(v: JsonValue) : string`            | compact serialize     |
| `Json.EscapeString(s) : string`                 | escape for embedding  |
| `Json.NullValue() / OfBool / OfInt / OfFloat`   | factory constructors  |
| `Json.OfString / OfArray`                       | factory constructors  |

`JsonValue` carries one of seven kinds (`Null`, `Bool`, `Int`,
`Float`, `String`, `Array`, `Object`):

| `v.IsNull() / IsBool() / IsInt() / IsFloat() / IsString() / IsArray() / IsObject()` |
| `v.AsBool() / AsInt() / AsFloat() / AsString() / AsArray()`         |
| `v.Get(key: string) : JsonValue`        | object access (Null on miss)  |
| `v.Has(key: string) : bool`             | object key existence          |
| `v.Keys() : List<string>`               | iteration order = insertion   |
| `v.At(i: int) : JsonValue`              | array access (Null on miss)   |
| `v.Length() : int`                      | array len / object key count  |

> Codegen note: a chain like `r.Value.Get("k").AsString()` currently
> mashes through cgen because `obj.Field.Method()` is lowered as a
> name-concat (`Value_Get`). Extract intermediate typed locals
> (`let v: JsonValue = r.Value; let kn: JsonValue = v.Get("k");
> kn.AsString()`) until the codegen fix lands. Same workaround as
> the JSON test sample in `tests/samples/stdlib_json.am`.

## Random — PRNG and OS entropy

`Amalgame.Random` exposes a small, statistically strong PRNG
(PCG XSH-RR 64/32) plus a passthrough to the OS entropy pool for
crypto-grade needs. The PRNG step itself runs in the C runtime
(`runtime/Amalgame_Random.h`) — the deliberate `uint64`
wrap-around it relies on is well-defined unsigned but UB on
Amalgame's signed `int`, so we keep the multiply behind a
runtime helper.

```kotlin
import Amalgame.Random
import Amalgame.Collections

// Reproducible: same seed → same stream
let r1 = new Random(42)
let n  = r1.NextInt()

// Unguessable: 16 bytes pulled from the OS entropy pool
let r2 = Random.FromSystem()
let f  = r2.Float()                  // [0.0, 1.0)
let d6 = r2.IntRange(1, 7)           // [1, 7) = a six-sided die
let coin = r2.Bool()
let bs: List<int> = r2.Bytes(32)     // 32 bytes ∈ [0, 255]

// Shortcut when you don't want to keep a Random instance around
let salt: List<int> = Random.SystemBytes(16)
```

### Constructor + factories

| Form                       | Purpose                                          |
|----------------------------|--------------------------------------------------|
| `new Random(seed)`         | Reproducible stream from a 64-bit seed.          |
| `Random.FromSystem()`      | Unguessable seed from 16 bytes of OS entropy.    |
| `Random.SystemBytes(n)`    | Static — n crypto-grade bytes ∈ [0, 255]. No instance needed. |

### Methods on a Random instance

| Method                  | Returns                              |
|-------------------------|--------------------------------------|
| `NextUInt32()`          | `int` ∈ [0, 2^32 - 1]                |
| `NextInt()`             | `int`, full 64-bit range             |
| `IntRange(min, max)`    | `int` ∈ [min, max) (half-open)       |
| `Float()`               | `float` ∈ [0.0, 1.0)                 |
| `Bool()`                | `bool`                               |
| `Bytes(n)`              | `List<int>`, n entries ∈ [0, 255]    |

**Reproducibility.** Two `new Random(seed)` instances with the
same seed always produce identical streams across runs and across
OSes. Use that for tests, simulations, and any "deterministic
shuffle" needs.

**OS entropy backend.** `FromSystem()` and `SystemBytes()` use:

- POSIX (Linux, macOS, BSDs): `getentropy(3)` — falls back to
  reading `/dev/urandom` if `getentropy` is unavailable.
- Windows: `BCryptGenRandom` with the system-preferred RNG.

If both paths fail (rare: chrooted POSIX without `/dev/urandom`,
or no Windows crypto provider), the buffer is filled with zeros
rather than returning a partial fill — the caller sees a
deterministic-but-useless result instead of garbage.

**Modulo bias in `IntRange`.** `IntRange(min, max)` uses simple
modulo on a 32-bit draw. The bias is undetectable for ranges much
smaller than 2^32 (i.e. anything you'd actually pass at the
language level). Callers needing unbiased output for very large
spans can layer rejection sampling on top of `NextUInt32()`.

> The legacy `Math.SeedRandom` / `Math.Random` / `Math.RandomInt`
> helpers stay for compatibility but are not recommended — they
> use a single global state, an 8-bit-output LCG, and have no
> crypto path. New code should reach for `Amalgame.Random`.

## Encoding — Base64, Hex, percent-encoding

`Amalgame.Encoding` ships three small static facades for the
bread-and-butter byte/text transcodings: `Base64` (RFC 4648),
`Hex`, and `Url` (percent-encoding per RFC 3986). All three are
pure Amalgame — no runtime header, no syscalls.

```kotlin
import Amalgame.Encoding
import Amalgame.Collections

// ── Base64 ──
let bytes: List<int> = new List<int>()
bytes.Add(72); bytes.Add(105)               // "Hi"
let s: string = Base64.Encode(bytes)        // "SGk="
let back: List<int> = Base64.Decode(s)      // [72, 105]

// URL-safe variant (swaps + / for - _)
let token: string = Base64.EncodeUrl(bytes) // "SGk="
let raw:   List<int> = Base64.DecodeUrl(token)

// ── Hex ──
let h: string = Hex.Encode(bytes)            // "4869" (lowercase)
let H: string = Hex.EncodeUpper(bytes)       // "4869"
let bytes2: List<int> = Hex.Decode("48 69")  // [] (rejects spaces)
let bytes3: List<int> = Hex.Decode("4869")   // [72, 105]

// ── Url (percent-encoding) ──
Url.Encode("hello world")           // "hello%20world"
Url.Encode("a/b?c=1")               // "a/b?c=1"   — path-safe chars kept
Url.EncodeComponent("a/b?c=1")      // "a%2Fb%3Fc%3D1"
Url.Decode("a%2Fb%3Fc%3D1")         // "a/b?c=1"
```

### Base64

| Method                | Returns                     |
|-----------------------|-----------------------------|
| `Encode(bytes)`       | `string` — RFC 4648 §4 alphabet, `=` padding |
| `Decode(s)`           | `List<int>` — empty on invalid chars |
| `EncodeUrl(bytes)`    | `string` — URL-safe alphabet (`-_` for `+/`) |
| `DecodeUrl(s)`        | `List<int>` — counterpart of EncodeUrl |
| `IsValid(s)`          | `bool` — true if `s` decodes cleanly under standard alphabet |

Both decoders accept padded *or* unpadded input. Whitespace
inside the encoded body is **not** stripped — strict mode by
default. Trim or pre-clean upstream if your input may have
embedded newlines (e.g. PEM blocks).

### Hex

| Method               | Returns                                |
|----------------------|----------------------------------------|
| `Encode(bytes)`      | `string` — lowercase, no separator     |
| `EncodeUpper(bytes)` | `string` — uppercase                   |
| `Decode(s)`          | `List<int>` — accepts mixed case; empty on odd-length or invalid char |
| `IsValid(s)`         | `bool` — even-length + all hex chars   |

### Url

| Method                  | Returns                                   |
|-------------------------|-------------------------------------------|
| `Encode(s)`             | `string` — preserves the unreserved set + path-safe gen/sub-delims (`/?#&=+:@,;`) |
| `EncodeComponent(s)`    | `string` — also escapes the path-safe set; matches JS `encodeURIComponent` |
| `Decode(s)`             | `string` — RFC-strict; `+` is **not** mapped to space |

`Decode` is intentionally strict on `+`. Form-encoded payloads
(`application/x-www-form-urlencoded`) are a separate spec layered
on top — pre-substitute `+` → space yourself if you're parsing a
form body, or wait for a dedicated `Form` decoder.

> The byte → char round-trips here all assume `bytes` ∈ [0, 255]
> per entry. `Random.Bytes(n)` and `Random.SystemBytes(n)` already
> return values in that range, so encoding entropy as Base64/Hex
> works without an extra cast layer.

## DateTime — Instant, Duration, Stopwatch

`Amalgame.DateTime` ships three small classes — `Instant`,
`Duration`, and `Stopwatch` — plus an `InstantResult` shape for
parse fallout. v1 is **UTC-only**: explicit `+HH:MM` offsets in
ISO 8601 input are rejected, and there is no concept of local
time, named timezones, or DST. The roadmap tracks those as
follow-ups; the current API is small enough that adding a
parallel `LocalTime` later won't churn existing call sites.

```kotlin
import Amalgame.DateTime

let now: Instant = Instant.Now()
Console.WriteLine(now.Format())          // 2026-05-09T22:30:00.123456789Z

// ISO 8601 round-trip (UTC subset)
let r: InstantResult = Instant.Parse("2026-01-15T08:00:00.5Z")
if (r.Ok) {
    let v: Instant = r.Value
    Console.WriteLine(v.UnixSeconds())   // 1768464000
}

// Arithmetic with Duration
let later: Instant = now.Add(Duration.FromMinutes(30))
let elapsed: Duration = later.Since(now)
Console.WriteLine(elapsed.Format())      // 30m0s

// Stopwatch over the monotonic clock — immune to wall-clock jumps
let sw: Stopwatch = new Stopwatch()
heavyComputation()
Console.WriteLine("took " + sw.Elapsed().Format())
```

### Instant

| Method                       | Returns           | Notes                              |
|------------------------------|-------------------|------------------------------------|
| `new Instant(nanos)`         | `Instant`         | Direct ns-since-epoch constructor  |
| `Instant.Now()`              | `Instant`         | Wall clock (NTP-affected)          |
| `Instant.FromUnixSeconds(s)` | `Instant`         |                                    |
| `Instant.FromUnixMillis(ms)` | `Instant`         |                                    |
| `Instant.FromUnixNanos(ns)`  | `Instant`         |                                    |
| `Instant.Parse(s)`           | `InstantResult`   | RFC 3339 / ISO 8601, UTC only      |
| `UnixSeconds()`              | `int`             |                                    |
| `UnixMillis()`               | `int`             |                                    |
| `UnixNanos()`                | `int`             |                                    |
| `Format()`                   | `string`          | Fixed 9-digit fraction + `Z`       |
| `Add(d)` / `Subtract(d)`     | `Instant`         | Shift by Duration                  |
| `Since(other)`               | `Duration`        | `this - other`                     |
| `IsBefore` / `IsAfter` / `Equals` | `bool`       |                                    |

### Duration

| Method                             | Returns       |
|------------------------------------|---------------|
| `new Duration(nanos)`              | `Duration`    |
| `Duration.FromSeconds / Millis / Minutes / Hours / Days` | `Duration` |
| `Nanos / Millis / Seconds`         | `int`         |
| `SecondsFloat()`                   | `float`       |
| `Plus / Minus / Times / Negate`    | `Duration`    |
| `IsZero / IsNegative`              | `bool`        |
| `Format()`                         | `string`      |

`Format()` emits Go-style human readable shorthand: `0s`,
`500ns`, `1us`, `1ms`, `5s`, `2m5s`, `1h2m5s`. Negative
durations get a leading `-`.

### Stopwatch

| Method                  | Returns       |
|-------------------------|---------------|
| `new Stopwatch()`       | captures the monotonic clock |
| `Elapsed()`             | `Duration` since construction |
| `Reset()`               | `Duration` since last start, then rewinds to now |

Use Stopwatch — not `Instant.Now()` differences — for measuring
elapsed time. The monotonic clock is unaffected by NTP
adjustments and manual clock changes.

### Limitations to know about

- **UTC only.** No timezones, no local-time conversion, no DST
  handling. Track 1: a future `LocalTime` companion class.
- **Parse strict on `Z`.** Inputs like `2026-05-09T22:30:00+02:00`
  return `Ok=false`. If you need offset support, normalize to
  UTC at the source for now.
- **Leap seconds.** A `:60` second in parse input is accepted
  (per ISO 8601) but clamped to `:59` internally — strict
  leap-second handling needs a UTC↔TAI table that we don't
  ship. In practice, almost no public timestamp source will
  feed you a `:60` value anyway (most use UTC-SLS or smear).
- **i64 nanoseconds.** Range is roughly **1678-09-21 → 2262-04-11**.
  Outside that window, arithmetic silently wraps. Fine for the
  decade or two ahead; the API can grow a `Date` (no time of
  day, larger range) if applications outside this window become
  a thing.

## Crypto — SHA-256 and HMAC-SHA-256

`namespace Amalgame.Crypto` exposes two static facades — `Sha256`
for the bare hash and `Hmac` for keyed authentication. Both go
straight to `runtime/Amalgame_Crypto.h`, which holds the SHA-256
compression function (FIPS 180-4 §6.2) and the HMAC ipad/opad
construction (RFC 2104) in pure C. No external crypto library
dependency — the runtime header is self-contained, ~150 lines.

Bytes flow through as `List<int>` with each entry in `[0, 255]`
(the runtime masks to 8 bits anyway, so any int list works).
String-input convenience methods hash the UTF-8 byte
representation directly. All hex output is lowercase (RFC 4648
hex is case-insensitive on decode, so this matches the de-facto
modern convention).

```kotlin
import Amalgame.Crypto

// ── SHA-256 ─────────────────────────────────────────
let h1: string = Sha256.OfString("hello")        // hex (most common)
let raw: List<int> = Sha256.Bytes(some_bytes)    // 32 raw bytes
let h2: string = Sha256.Hex(some_bytes)          // hex of bytes input

// ── HMAC-SHA-256 ────────────────────────────────────
let mac: string = Hmac.Sha256OfStrings(api_key, payload)
let mac_raw: List<int> = Hmac.Sha256(key_bytes, msg_bytes)
let mac_hex: string = Hmac.Sha256Hex(key_bytes, msg_bytes)
```

### Sha256

| Method                          | Returns        | Notes                                     |
|---------------------------------|----------------|-------------------------------------------|
| `Sha256.Bytes(data: List<int>)` | `List<int>`    | Raw 32-byte digest                        |
| `Sha256.Hex(data: List<int>)`   | `string`       | Lowercase hex of the 32-byte digest       |
| `Sha256.OfString(s: string)`    | `string`       | UTF-8 bytes of `s` → digest → hex         |

### Hmac

| Method                                                | Returns      |
|-------------------------------------------------------|--------------|
| `Hmac.Sha256(key: List<int>, msg: List<int>)`         | `List<int>`  |
| `Hmac.Sha256Hex(key: List<int>, msg: List<int>)`      | `string`     |
| `Hmac.Sha256OfStrings(key: string, msg: string)`      | `string`     |

Keys longer than the SHA-256 block size (64 bytes) are hashed down
to 32 bytes per RFC 2104 §2; keys shorter than 64 are zero-padded.
Either is handled transparently inside the runtime.

### Caveats

- **SHA-256 only for now.** No SHA-1 (insecure for new uses anyway),
  SHA-512, MD5, or Blake. Add as needed.
- **Constant-time comparison** of MAC values is the caller's
  responsibility. Amalgame's `string ==` and `List<int>` comparison
  short-circuit on the first byte mismatch, which leaks timing
  information against a determined attacker. For verifying signatures
  / MACs against untrusted input, compare via a manual byte-by-byte
  loop that always runs to completion.

## Logging — leveled stderr + optional file sink

`namespace Amalgame.Logging` exposes a `public class Log` facade
with four levels (Debug, Info, Warn, Error). Emits to stderr with
a UTC ISO 8601 timestamp + fixed-width label, plus an optional
file sink that appends every line. Configuration is process-wide
singleton state held in the runtime (same pattern as `Exit.Set`
/ `Exit.Get`).

```kotlin
import Amalgame.Logging

Log.SetMinLevel("info")             // default; "debug"/"info"/"warn"/"error"
Log.SetFile("/var/log/app.log")     // optional; empty string disables

Log.Debug("got 42 items")           // suppressed unless min level <= debug
Log.Info("server ready on :8080")
Log.Warn("retry attempt 3")
Log.Error("connection refused")
```

Output looks like:

```
2026-05-10T20:42:33Z INFO  server ready on :8080
2026-05-10T20:42:35Z WARN  retry attempt 3
2026-05-10T20:42:38Z ERROR connection refused
```

| Method                              | Returns  | Notes                                                            |
|-------------------------------------|----------|------------------------------------------------------------------|
| `Log.SetMinLevel(name: string)`     | `void`   | "debug" / "info" / "warn" / "error"; case-insensitive on 1st char|
| `Log.GetMinLevel()`                 | `string` | Lower-case canonical name of the current minimum                 |
| `Log.SetFile(path: string)`         | `void`   | Empty string disables the file sink                              |
| `Log.GetFile()`                     | `string` | Empty string when no sink                                        |
| `Log.Debug(msg: string)`            | `void`   |                                                                  |
| `Log.Info(msg: string)`             | `void`   |                                                                  |
| `Log.Warn(msg: string)`             | `void`   |                                                                  |
| `Log.Error(msg: string)`            | `void`   |                                                                  |

**Level name parsing** is case-insensitive on the first letter —
`"DEBUG"`, `"Debug"`, `"debug"` all map to the same code. Unknown
names default to `"info"` silently; logging itself should never
crash a process.

**File sink** reopens on each emit — slower than holding a stream
open but robust against external log rotation, which is the
typical production setup. Single-process, thread-unsafe v1 — fine
for CLIs and single-threaded servers. A mutex is the v2 ask once
a real multi-threaded user lands.

## Service — long-running process primitives

`namespace Amalgame.Service` wraps POSIX `signal()` + `nanosleep()`
(Linux/macOS) and `SetConsoleCtrlHandler` + `Sleep()` (Windows)
behind one unified surface. Callers don't branch on platform.

The typical service loop pattern:

```kotlin
import Amalgame.Service
import Amalgame.Logging

public class Program {
    public static int Main(List<string> args) {
        Log.SetMinLevel("info")
        Log.Info("starting")
        Service.Install()
        while (!Service.ShouldStop()) {
            // one unit of work
            Service.Sleep(5000)
        }
        Log.Info("shutting down cleanly")
        return 0
    }
}
```

| Method                       | Returns | Notes                                                                       |
|------------------------------|---------|-----------------------------------------------------------------------------|
| `Service.Install()`          | `void`  | Register SIGTERM/SIGINT handler (or `SetConsoleCtrlHandler` on Windows)     |
| `Service.ShouldStop()`       | `bool`  | True after the OS has delivered a shutdown signal                           |
| `Service.RequestStop()`      | `void`  | Programmatic shutdown trigger — flips the same flag                         |
| `Service.Sleep(ms: int)`     | `void`  | Returns early when `ShouldStop()` becomes true                              |

**Shutdown semantics**: the flag is `sig_atomic_t` on POSIX and
`LONG` + `InterlockedExchange` on Windows. Async-signal-safe on
POSIX, atomic on Windows; no mutex needed for v1 single-process
scope. POSIX `Service.Sleep` uses `nanosleep` and is naturally
interruptible by signal delivery via EINTR. The Windows path
slices into 100ms chunks (no portable equivalent of nanosleep-
with-EINTR for console signals).

**Reinstalling** — calling `Service.Install()` more than once is
safe; the OS just replaces the existing handler.

## Database.SQLite — embedded SQL via opt-in package

`namespace Amalgame.Database.SQLite` is a SQLite 3 binding shipped
as an opt-in external package: **`amalgame-lang/amalgame-database-sqlite`**.
Since v0.5 it's no longer bundled with the compiler — install via:

```bash
amc add github.com/amalgame-lang/amalgame-database-sqlite@v0.1.0
```

The package vendors the SQLite amalgamation (public-domain
upstream), so no `libsqlite3-dev` system package is required on
any of Linux / macOS / Windows — the source compiles directly
into the user binary at link time.

The namespace nests under `Amalgame.Database.<Engine>` so sibling
backends (DuckDB, Postgres, MySQL) can land as their own packages
without conflicting class names. SQLite v0.1.0 is the first
inaugural package; siblings tracked in the roadmap.

```kotlin
import Amalgame.Database.SQLite
import Amalgame.Collections

let db = SQLite.Open(":memory:")   // or a real file path
if (!SQLite.IsOpen(db)) {
    Console.WriteError("open failed: " + SQLite.LastError(db))
    return
}

SQLite.Exec(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)")
SQLite.Exec(db, "INSERT INTO users (name, age) VALUES ('Alice', 30)")
SQLite.Exec(db, "INSERT INTO users (name, age) VALUES ('Bob', 25)")
let id: int = SQLite.LastInsertId(db)            // 2

// Outer list has no annotation because the parser rejects nested
// generics (`List<List<string>>`). Inner list is annotated so the
// cgen can resolve `.Get(int)` to the typed result.
let rows = SQLite.QueryAll(db, "SELECT id, name FROM users ORDER BY id")
let firstRow: List<string> = rows.Get(0)
let firstName: string = firstRow.Get(1)          // "Alice"

SQLite.Close(db)
```

| Method                                | Returns                | Notes                                                              |
|---------------------------------------|------------------------|--------------------------------------------------------------------|
| `SQLite.Open(path: string)`           | `AmalgameSQLite*`      | `:memory:` for transient in-memory; any path for on-disk           |
| `SQLite.IsOpen(db)`                   | `bool`                 | Checks the handle is live                                          |
| `SQLite.Close(db)`                    | `void`                 | Idempotent; no-op on already-closed handles                        |
| `SQLite.Exec(db, sql: string)`        | `bool`                 | No-result SQL (DDL / DML / PRAGMA). Returns false on error         |
| `SQLite.QueryAll(db, sql: string)`    | `List<List<string>>`   | Rows × columns as text. Empty list on no rows OR error             |
| `SQLite.LastInsertId(db)`             | `int`                  | Rowid of the most recent INSERT on this handle                     |
| `SQLite.Changes(db)`                  | `int`                  | Row count of the most recent INSERT/UPDATE/DELETE                  |
| `SQLite.LastError(db)`                | `string`               | Snapshot of the most recent error message; empty if none           |

**Result shape:** `QueryAll` returns every column value as text
(via `sqlite3_column_text`). NULL columns become empty strings.
Convert numerics in Amalgame as needed (`String_ToInt(row.Get(0))`).

**Linking:** after `amc add`, the package's vendored `sqlite3.c`
lives at
`~/.amalgame/packages/github.com/amalgame-lang/amalgame-database-sqlite/<tag>_<sha>/runtime/Amalgame_Database/sqlite/sqlite3.c`.
Pre-compile it once and link with the generated user `.c`:

```
PKG=~/.amalgame/packages/github.com/amalgame-lang/amalgame-database-sqlite/v0.1.0_*
gcc -O2 -Iruntime -w -c $PKG/runtime/Amalgame_Database/sqlite/sqlite3.c -o sqlite3.o
gcc -O2 -Iruntime your_program.c sqlite3.o -lgc -lm -lcurl -ldl -lpthread -o your_program
```

The `amc test` runner automates this on Database tests; a future
`amc build` will wire the link step automatically based on each
package's `sources` field in its manifest.

**v1 limitations:**

- **No parameter binding.** All SQL is passed as raw strings, so
  callers MUST escape any user input by hand or stick to fully-
  trusted SQL. Prepared-statement `?` binding is the v2 ask.
- **Text-only columns.** Numeric types are returned as their
  string representation. Typed accessors (`AsInt(0)` / `AsBytes(0)`)
  ride alongside `?` binding in v2.
- **No explicit transactions.** Run `BEGIN` / `COMMIT` / `ROLLBACK`
  via `Exec` for now; a `db.Begin()` / `Commit()` API lands in v2.

## What's missing

- Bigger Math (trig, logs)
- Async/iter/streaming abstractions over collections
- Local time / timezones (deferred from DateTime v1)
- Regex
- Sibling Database backends (DuckDB / Postgres / MySQL) — same
  namespace family as SQLite, vendored amalgamation for the
  ones that ship one (DuckDB), dynamic-link for the network-
  bound ones (Postgres / MySQL).
- A package manager and ecosystem

These are tracked in [ROADMAP_COMPLET.md](../../ROADMAP_COMPLET.md).
