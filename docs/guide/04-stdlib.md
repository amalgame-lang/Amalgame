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

```amalgame
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

```amalgame
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

### Path helpers

| `Path.Combine(a, b) : string`             | join with `/`                   |
| `Path.GetExtension(p) : string`           | `.ext`                          |
| `Path.GetFilename(p) : string`            | basename                        |
| `Path.GetDirectory(p) : string`           | dirname                         |

```amalgame
let cfg = File.ReadAll("config.txt")
File.AppendAll("log.txt", "[startup]\n")
let lines = String.Split(cfg, "\n")
File.WriteLines("clean.txt", lines)
```

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

```amalgame
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

```amalgame
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

```amalgame
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

```amalgame
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

## What's missing

- Bigger Math (trig, logs)
- Async/iter/streaming abstractions over collections
- Date/Time
- Regex
- Random (crypto-grade + seeded PRNG)
- Encoding (Base64, hex, URL)
- Process spawning beyond `Args` / `Exit` (basic `Process.Run`
  / `Process.RunCapture` already in)
- A package manager and ecosystem

These are tracked in [ROADMAP_COMPLET.md](../../ROADMAP_COMPLET.md).
