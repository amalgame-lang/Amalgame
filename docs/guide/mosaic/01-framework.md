# Web framework (Mosaic)

> **Scope.** This chapter covers **Mosaic**, the server-side web
> framework shipped as the `amalgame-web` package (HTTP routing,
> middleware, sessions, static files, HTTPS). It is *not* the same as
> [`amalgame-ui-web`](09-ui-web/README.md), which is a desktop GUI
> binding over an OS webview. Different package, different use case.
>
> Everything below is from `amalgame-web v0.28.0` and its dependencies
> as shipped. Method signatures are quoted from the package facades.

Mosaic is a small, explicit web framework: you create a `WebApp`,
attach route handlers as closures, optionally wrap them in middleware
(security headers, CORS, CSRF, rate limiting, sessions, static files),
and start a server. There is no global state, no annotation magic, no
hidden reflection — a handler is just a `Closure<WebContext, HttpResponse>`.

---

> **Getting it / configuring it.** This page is the framework tour.
> For setup see [**Installation**](02-installation.md); for the full
> TOML / env reference see [**Configuration**](03-configuration.md).

---

## Hello, server

```amalgame
import Amalgame.Collections
import Amalgame.String
import Amalgame.Net.Http
import Amalgame.Web

public class Program {
    public static void Main(string[] args) {
        let app = new WebApp()

        app.Get("/", ctx => HttpResponse.New().Text("Hello from Mosaic"))

        app.Serve(8080)
    }
}
```

`WebApp.Serve(port)` runs a serial, single-connection-at-a-time server
— ideal for development. It blocks. The four imports are the standard
set: `Collections` and `String` for the stdlib types your handlers
touch, `Net.Http` for `HttpResponse`, and `Web` for `WebApp`.

> `new WebApp()` and `WebApp.New()` are equivalent — the static `New()`
> just calls the constructor. Use whichever reads better; this chapter
> uses `new WebApp()`.

---

## Routing

Register a handler per HTTP method. Each returns the `WebApp` so calls
chain:

```amalgame
public WebApp Get(string path, Closure<WebContext, HttpResponse> handler)
public WebApp Post(string path, Closure<WebContext, HttpResponse> handler)
public WebApp Put(string path, Closure<WebContext, HttpResponse> handler)
public WebApp Patch(string path, Closure<WebContext, HttpResponse> handler)
public WebApp Delete(string path, Closure<WebContext, HttpResponse> handler)
```

Paths support two capture forms:

- `:name` — a single segment. `/users/:id` matches `/users/42`,
  capturing `id = "42"`.
- `*name` — a wildcard tail. `/files/*path` matches `/files/css/app.css`,
  capturing `path = "css/app.css"` (slashes included).

> **Order matters.** Routes are matched first-registered-first. Register
> static paths before parameterized ones — put `/users/me` before
> `/users/:id`, or the `:id` route will swallow `me`.

```amalgame
app.Get("/users/:id", ctx => {
    let id: string = ctx.Param("id")
    return HttpResponse.New().Json("{\"id\":\"" + id + "\"}")
})
```

---

## Reading the request — `WebContext`

The handler receives a `WebContext`. The fields and methods you'll use:

| Member | Kind | What it gives you |
|--------|------|-------------------|
| `ctx.Param(name)` | method | a `:name` path segment |
| `ctx.QueryParam(name)` | method | a `?name=…` query value |
| `ctx.Query` | field `Map<string,string>` | all query params |
| `ctx.Body` | field `string` | raw request body (text) |
| `ctx.Method` | field `string` | `"GET"`, `"POST"`, … |
| `ctx.Path` | field `string` | path without the query string |
| `ctx.State` | field `Map<string,string>` | app-level shared state (read) |
| `ctx.Session` | field `Session` | the session, or null if none |
| `ctx.IsMultipart()` | method | true for a `multipart/form-data` body |
| `ctx.Multipart()` | method | parse uploads → a `Multipart` |

```amalgame
app.Post("/echo", ctx => {
    let body: string = ctx.Body
    return HttpResponse.New().Status(201).Text(body)
})
```

`ctx.Body` is the raw text body — handy for JSON / form posts. For
**file uploads** (`multipart/form-data`), use `ctx.Multipart()`, which
parses the raw bytes (binary-safe, unlike `ctx.Body` which stops at the
first NUL):

```amalgame
app.Post("/upload", ctx => {
    let mp = ctx.Multipart()
    let avatar = mp.File("avatar")          // an UploadedFile, or null
    if (avatar != null) {
        avatar.SaveTo("/var/uploads/" + avatar.Filename)
    }
    let caption: string = mp.Field("caption")   // a non-file text field
    return HttpResponse.New().Text("got " + String_FromInt(mp.AllFiles().Count()) + " file(s)")
})
```

An `UploadedFile` exposes `Name` / `Filename` / `ContentType` /
`Size()` / `Bytes()` (a `List<int>`) / `Text()` / `SaveTo(path)`. The
parser comes from `amalgame-net-http` (`Multipart` / `UploadedFile`) and
honours the server's `max_body_bytes` cap, so oversized uploads are
rejected before they reach the handler.

---

## Building the response — `HttpResponse`

`HttpResponse` (from `amalgame-net-http`) is a chainable builder:

```amalgame
public HttpResponse Status(int code)              // default 200
public HttpResponse Text(string s)                // Content-Type: text/plain
public HttpResponse Html(string s)                // Content-Type: text/html
public HttpResponse Json(string jsonText)         // Content-Type: application/json
public HttpResponse Header(string name, string value)   // add/override (CR/LF-safe)
public HttpResponse Redirect(string url, bool permanent) // 302, or 308 if permanent
public HttpResponse SetCookie(Cookie cookie)      // Set-Cookie header
public HttpResponse File(string path)             // binary-safe file body
public HttpResponse Bytes(List<int> data, string ct)  // in-memory binary body
```

```amalgame
app.Get("/teapot", ctx =>
    HttpResponse.New()
        .Status(418)
        .Header("X-Powered-By", "Mosaic")
        .Json("{\"short_and_stout\":true}"))
```

`Json` takes a string — Mosaic does not impose a JSON encoder, so build
the payload with the `Amalgame.Formats.Json` stdlib (see
[04-stdlib.md](04-stdlib.md)) or string-build small literals as above.

`Bytes(data, ct)` ships an in-memory binary body (a `List<int>`) with an
explicit length — for dynamically generated images, gzip output, etc.;
all custom headers are preserved on the wire.

---

## Templates — `Template` (auto-escaping)

`HttpResponse.Html(s)` stores `s` verbatim, so hand-built HTML that
interpolates user data is an XSS hole. The `Template` engine
(`Amalgame.Web`, pure-Amalgame) closes it by **escaping every value by
default**. Render straight from the context:

```amalgame
app.Get("/users/:id", ctx => {
    var data = new Map<string, string>()
    data.Set("name", ctx.Param("id"))     // even if it contains <script>, it's escaped
    data.Set("admin", "true")
    return ctx.Render("views/user.html", data)
})
```

`ctx.Render(path, data)` reads the template file and returns a ready
`text/html` response; `ctx.RenderString(src, data)` renders an inline
string. The Mustache-like syntax:

| Tag | Effect |
|-----|--------|
| `{{x}}` | value of `x`, **HTML-escaped** (`& < > " ' /`) |
| `{{{x}}}` / `{{&x}}` | value of `x`, **raw** (opt out of escaping) |
| `{{#x}}…{{/x}}` | section: rendered once if `x` is truthy |
| `{{^x}}…{{/x}}` | inverted section: rendered if `x` is falsy |
| `{{>name}}` | a registered partial |
| `{{! … }}` | comment (dropped) |

Over a `Map<string,string>`, a key is *truthy* when present and not in
`{ "", "false", "0" }`. `Template.HtmlEscape(s)` is also available
directly for escaping a value in hand-built markup. (List iteration —
repeating a section per row — needs a richer context and is on the
roadmap; sections are boolean for now.)

---

## Middleware

Middleware is attached to the app with `With…` builders and runs around
every handler. All shipped pieces:

```amalgame
let app = new WebApp()
    .WithSecurityHeaders(SecurityHeaders.StrictApi())
    .WithCors(Cors.AllowAll())          // dev only — wildcard origin
    .WithCsrf(Csrf.Default())
    .WithRateLimit(RateLimit.PerIp(100, 60))   // 100 req / 60 s / IP
    .WithStatic(Static.New("/assets", "./public"))
    .WithCompress(Compression.Default())       // gzip eligible text responses
    .WithLogging(new LogConfig().WithAccessLog(true))
    .WithState("appName", "demo")
```

### Security headers

`SecurityHeaders.StrictHtml()` gives a strict Content-Security-Policy,
`X-Frame-Options: DENY`, `nosniff`, etc. — for HTML apps.
`SecurityHeaders.StrictApi()` is the JSON-API variant (`nosniff` +
referrer policy, no framing rules). Refine with `.WithHsts(...)`,
`.WithCsp(...)`, `.WithFrameOptions(...)` and friends. The handler's own
headers win over the defaults.

### CORS

`Cors.Strict()` (no origins until you add them), `Cors.AllowAll()`
(wildcard — **development only**), or `Cors.Disabled()`. Tune with
`.WithAllowedOrigins(list)`, `.WithAllowedMethods(list)`,
`.WithAllowCredentials(bool)`, `.WithMaxAge(seconds)`.

### CSRF

`Csrf.Default()` is a double-submit-cookie guard: a 256-bit token,
`Secure` + `SameSite=Lax`, validated on unsafe methods (POST/PUT/…) and
skipped on GET/HEAD/OPTIONS. `Csrf.Disabled()` turns it off.

### Rate limiting

`RateLimit.PerIp(maxRequests, windowSec)` — a fixed window per client
IP, returning `429` with `Retry-After` when exceeded. `.WithBackend("redis")`
+ `.WithRedisHost(...)` shares the limit across instances; the default
`"memory"` backend is per-process.

### Compression (gzip)

`Compression.Default()` gzip-compresses eligible text responses when the
client sends `Accept-Encoding: gzip`, setting `Content-Encoding: gzip` +
`Vary: Accept-Encoding`. It's the **last** response step and only fires
for compressible types (`text/*`, JSON, JS, SVG, XML) above a size
threshold (1 KB) — small bodies, already-encoded responses, and `.File`
responses are left untouched (serve a pre-compressed `.gz` next to the
asset for those — see *Static files*). `Compression.Strict()` lowers the
threshold and adds `application/wasm`; tune with `.WithMinSize(n)` /
`.WithType(mime)`. Backed by the `amalgame-compress` (zlib) package.

---

## Sessions

Three stores ship, all exposing the same `Session` (`Set`/`Get`/`Has`/
`Delete`/`Clear`). Pick by deployment shape:

### In-memory (dev / single process)

```amalgame
let store = new MemorySessionStore()
let s: Session = store.Create("session-id-here")
s.Set("user", "alice")
let who: string = s.Get("user")     // "alice"
let cookie: Cookie = store.MakeCookie("session-id-here")  // for resp.SetCookie(...)
```

Lost on restart, single machine only. Thread-safe under `ServeMt`.

### Signed cookie (stateless, scales horizontally)

```amalgame
let store = new SignedCookieSessionStore("a-long-random-secret")
    .WithEncrypted(true)            // AES-256-GCM; default false = HMAC-only
let value: string = store.Encode(s) // the cookie value
let back: Session  = store.Decode(value)
```

The cookie *is* the session — no server storage. Signed-only is
tamper-proof but readable by the client; `.WithEncrypted(true)` makes it
confidential too. (v0.1 limitation: keys/values can't contain `&`, `=`
or `.` yet — no escaping. Fine for ids and flags.)

### Redis (distributed, persistent)

```amalgame
let store = new RedisSessionStore()
    .WithHost("127.0.0.1").WithPort(6379)
    .WithKeyPrefix("sess:").WithMaxAgeSec(3600)
let s: Session = store.Create("session-id-here")
store.Save(s)                        // persist mutations
```

Requires a reachable Redis (`>= 5.0`).

---

## Static files

```amalgame
app.WithStatic(Static.New("/assets", "./public").WithCacheMaxAge(3600))
```

Serves `./public/app.css` at `/assets/app.css`. Path-traversal is
blocked, directories return `403` (no auto-index), misses fall through
to the router. MIME type is set by extension.

Caching + delivery (v0.27.0):

- **Conditional GET** — `ETag` + `Last-Modified` are emitted; a matching
  `If-None-Match` or `If-Modified-Since` returns `304 Not Modified`.
- **Range requests** — `Accept-Ranges: bytes` is advertised; a `Range:`
  header is served as `206 Partial Content` (single range: `0-1023`,
  `1024-`, or suffix `-512`), with `416` + `Content-Range: bytes */N`
  when unsatisfiable. Good for video / large-file resume.
- **Pre-compressed variants** — when the client accepts gzip and a
  `<file>.gz` sits next to the asset, it's served with
  `Content-Encoding: gzip` (skipped for Range requests). This is the
  file complement to the in-memory `Compression` middleware.

Register specific prefixes before general ones if they overlap.

---

## Serving: HTTP and HTTPS

Mosaic ships several server entry points. Each takes a port and blocks:

```amalgame
public int Serve(int port)        // serial, one connection at a time — dev
public int ServeMt(int port)      // one OS thread per connection
public int ServeAsync(int port)   // one thread, N fibers (Linux/epoll only)
```

Each has a `…With(port, cfg)` variant taking an
`AmalgameNetHttpServerConfig` — use it to enable HTTP/1.1 keep-alive via
`.WithIdleTimeoutSec(seconds)`. (The plain `Serve`/`ServeMt` forms are
one-request-per-connection.)

For TLS, terminate in-process (shipped v0.14.0+):

```amalgame
public int ServeHttps(int port, string certPath, string keyPath)
public int ServeHttpsMt(int port, string certPath, string keyPath)
```

These need OpenSSL 3.x (or LibreSSL) on the system. For automatic
Let's Encrypt certificates, pair with `amalgame-tls`'s ACME support.

---

## Server-Sent Events (SSE)

For one-way live push (progress, notifications, tickers) without the
weight of WebSocket, register an SSE route. The handler holds the
connection and pushes frames until the client disconnects:

```amalgame
app.Sse("/events", sc => {
    var n: int = 0
    while (sc.Send("tick " + String_FromInt(n))) {   // false when the peer is gone
        n = n + 1
        DateTime.SleepSeconds(1)
    }
    return 0
})
```

The handler gets an `SseConn` (from `amalgame-net-http`):
`Send(data)` (a `data:` event; false signals the client left),
`SendEvent(event, data, id)`, `Comment(s)` (a `: …` heartbeat),
`Retry(ms)`, and `Close()`. The browser side is one line:

```js
new EventSource("/events").onmessage = e => console.log(e.data)
```

SSE routes are matched (GET + exact path) **before** the normal
request → response pipeline, so they bypass the response middlewares
(they own the connection). Under `ServeMt` each stream holds its worker
thread for its lifetime — fine for tens/hundreds of clients; use
`ServeAsync` for high fan-out.

---

## A complete example

Path params, a JSON route, a POST body, and API security headers — this
is a single self-contained program:

```amalgame
import Amalgame.Collections
import Amalgame.String
import Amalgame.Net.Http
import Amalgame.Web

public class Program {
    public static void Main(string[] args) {
        let app = new WebApp()

        app.Get("/", ctx => HttpResponse.New().Text("Hello from Mosaic"))

        app.Get("/users/:id", ctx => {
            let id: string = ctx.Param("id")
            return HttpResponse.New().Json("{\"id\":\"" + id + "\"}")
        })

        app.Post("/echo", ctx => {
            let body: string = ctx.Body
            return HttpResponse.New().Status(201).Text(body)
        })

        app.WithSecurityHeaders(SecurityHeaders.StrictApi())
        app.WithCors(Cors.AllowAll())

        app.Serve(8080)
    }
}
```

---

## Gotchas

- **`amalgame-net-http` must be `>= 0.11.1`.** Older versions drop every
  custom response header on the wire — your `Set-Cookie`, CSP, CORS and
  CSRF cookies silently vanish. `amc package add web` resolves a correct
  floor; don't pin an older net-http by hand.
- **Route order is significant** — static before parameterized, specific
  static-file prefixes before general ones.
- **`ctx.Body` is text** (truncates at a NUL) — for binary uploads use
  `ctx.Multipart()`, which walks the raw bytes.
- **`Cors.AllowAll()` is for development.** In production list explicit
  origins with `Cors.Strict().WithAllowedOrigins([...])`.
- **HTTPS needs system OpenSSL 3.x.** The `ServeHttps*` entry points
  link against it.

For configuration patterns (toml/env/flag layering) see
[mosaic-configuration.md](https://github.com/amalgame-lang/Amalgame/blob/main/docs/mosaic-configuration.md).
For the HTTP types themselves, see the `amalgame-net-http` package README.
