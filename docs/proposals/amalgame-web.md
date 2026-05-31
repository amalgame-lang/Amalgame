# Proposal: `Amalgame.Web` — TLS, HTTP and the Mosaic web server

**Status:** Foundation shipped 2026-05-18 onward. Router, sessions,
security middlewares, filesystem routing, livereload, and a **native
pure-AM ACME client (http-01)** are shipped.

> ⚠️ **Shipped vs. roadmap — read before quoting this in marketing.**
> **Shipped:** Router, Sessions, security headers (CSP/HSTS/CORS/CSRF/
> rate-limit), static serving, filesystem routing, native ACME client
> (`amalgame-tls/acme.am`, **http-01 only**), single-binary builds.
> **NOT yet shipped / partial (do not claim as fact):**
> - tls-alpn-01 and dns-01 ACME challenges.
> - Native TLS termination **fully wired to the Mosaic HTTP/1.1
>   router in production**. The reference live demo (`amalgame-live`)
>   is currently fronted by an external Node/greenlock proxy — see its
>   README. Until a real Mosaic instance serves its own public HTTPS,
>   the "zero nginx / zero proxy, single binary" pitch is a goal, not
>   a demonstrated fact.
> - ACME auto-renew **without a restart** (the running TLS context
>   still holds the old cert until the service restarts).
> - HTTP/2 wired into the H1 `WebApp` handler, Phoenix-style pub/sub,
>   `dlopen` hot-reload, multi-site supervisor — all roadmap.

Section 21 below is the authoritative roadmap for the remaining work;
it supersedes the original "v0.1 through v1.0 versions" plan as the
architecture evolved during implementation (notably: `mosaic` became
its own repo, WebSocket landed in `amalgame-net-http` rather than
`amalgame-web`).

This document remains the contract for `amalgame-tls`,
`amalgame-net-http`, `amalgame-web` and `mosaic`.

**Author:** v0.8.x cycle, post `ui-web v0.0.4` ship. Driven by the need
to give Amalgame a credible server story to match the desktop story
delivered by [`amalgame-ui-web`](./amalgame-ui-web.md).

**Tracking:** TBD — issues and PRs will be linked here as work begins.

**Related proposals:**
- [`amalgame-ui-web.md`](./amalgame-ui-web.md) — desktop UI story (the
  *client* counterpart to this server story).
- [`amalgame-package-manager.md`](./amalgame-package-manager.md) — how
  `amalgame-tls`, `amalgame-net-http`, `amalgame-web` are installed.

---

## 1. Problem

Amalgame ships a compiler, a package manager, a desktop UI toolkit, and
~15 external packages, but **has no story for building web servers**.
A modern language ecosystem without a productive web framework is
incomplete: web servers are by far the most common deployment target
for backend code in 2026.

Two prior datapoints in the ecosystem hint at what is missing:

- `amalgame-net-websocket v0.1` (extracted from amc's bundled runtime
  in v0.7.8) ships a **WebSocket client**, no server. `ws://` plain TCP
  only, no `wss://` (TLS deferred).
- `amalgame-crypto` ships SHA-256 and HMAC-SHA-256 primitives, suitable
  for HMAC tokens and password verification, but no TLS.

There is no way today to:
- Serve HTTPS from an Amalgame binary.
- Accept HTTP requests with a developer-friendly routing API.
- Host dynamic `.am` code as HTTP endpoints with hot-reload during
  development.
- Issue TLS certificates automatically (ACME / Let's Encrypt).

This proposal fills that gap with three new packages and one new
command-line tool.

## 2. Goals & non-goals

### Goals (v1.0)

- Ship a **production-credible TLS stack** built on industry-standard
  primitives (OpenSSL 3.x), not embedded/IoT libraries.
- Ship an **HTTP/1.1 + HTTP/2** client and server, with body parsing,
  streaming responses, WebSocket upgrade.
- Ship a **productive web framework** ("Mosaic") with:
  - Filesystem-based routing (`app/users/[id].am` → `/users/:id`)
  - Programmatic routing (`.Get("/path", handler)`)
  - Composable middleware pipeline
  - Static file serving with ETag/range/compression
  - Sessions (in-memory / JSON file / Redis)
  - Built-in security middleware (CSRF, CORS, rate limit, slow loris)
  - ACME / Let's Encrypt automatic HTTPS
  - WebSocket server endpoint integration
  - Observability (structured logs, /metrics, /healthz)
  - Graceful shutdown
  - Three execution modes: DEV (hot reload), PROD modular (`.so`),
    PROD all-in-one (single binary)
- A `mosaic` CLI binary that scaffolds, develops, builds, and serves
  Mosaic apps — written in Amalgame, compiled by `amc`.

### Non-goals (deferred to v1.x+)

- HTTP/3 (QUIC) — too volatile across implementations in 2026.
- Multi-site / virtual host hosting from a single Mosaic process —
  use an external reverse proxy (nginx / Caddy / Traefik) for
  multi-tenant. One Mosaic binary serves one site.
- Process-isolated workers (one OS process per request / per route).
  Concurrency is in-process thread pool.
- A built-in templating language with parser-level extensions
  (JSX-like literals). Functional helpers in v0.1; JSX in v1.0+ if
  parser support lands.
- Built-in ORM. Use existing DB packages (`amalgame-database-sqlite`,
  `amalgame-database-postgresql`, etc.) directly.
- HTTP/2 server push (deprecated in practice — Chrome removed it).
- Pre-built binary distribution of `mosaic` per OS/arch. Build from
  source on install. May be revisited if install time exceeds 5 min.

## 3. Industry survey (2026)

| Stack | Strengths to borrow |
|---|---|
| **Caddy** (Go) | Automatic HTTPS via ACME, config-as-code, hot reload |
| **Nginx** | Master/worker, `sendfile()` zero-copy, mime types |
| **Go `net/http`** | Minimal `Handler` interface, `context.Context`, graceful shutdown |
| **Express / Fastify / Hono** (JS) | Middleware chain, body auto-parse, fluent API |
| **Next.js / Astro / SvelteKit** | Filesystem routing, hot module reload, layouts |
| **PHP-FPM** | One file = one endpoint mental model, opcache after first compile |
| **Phoenix** (Elixir) | Channels (pub/sub over WS), LiveView, supervisor robustness |
| **ASP.NET Core / Kestrel** | Middleware pipeline, DI, Razor pages, libuv server |
| **htmx + Hotwire** | Server-rendered HTML over the wire, no SPA tax |
| **h2o** | `picohttpparser` (~200 lines, fastest HTTP/1.1 parser) |
| **OpenSSL / LibreSSL** | The de-facto industrial TLS stack |
| **nghttp2** | The industrial HTTP/2 library (nginx, curl, Apache, Envoy) |

Non-negotiable UX features distilled from this survey:

1. Automatic HTTPS (ACME).
2. Filesystem-based routing as the primary developer ergonomic.
3. Composable middleware pipeline.
4. Hot reload during development.
5. Structured logging, metrics endpoint, health endpoint built-in.
6. Schema validation for request bodies.
7. Graceful shutdown.
8. WebSocket on the same router.
9. Performant static file serving with ETag/range/compression.
10. Production binary is a single artifact (Docker-friendly).

## 4. Architecture: three packages

We follow the existing Amalgame convention of small, focused packages
that compose (see `regex`, `crypto`, `compress`, `net-websocket`):

```
amalgame-tls          (binding C-header, TLS primitives)
   └── amalgame-net-http   (HTTP/1.1 + HTTP/2 parser, client, server primitives)
          └── amalgame-web    (Mosaic framework: router, middleware, .am routes)
```

Rationale: a user who wants to write a CLI that does HTTPS requests
should be able to `import Amalgame.Net.Http` without pulling in the
full framework. A user who wants TLS for a custom protocol (MQTT,
SMTP, custom TCP) should be able to `import Amalgame.Tls` without
HTTP. The framework sits on top.

### 4.1 `amalgame-tls`

**Purpose:** TLS 1.2 / 1.3 client and server primitives. Wraps
**OpenSSL 3.x** as a C-header binding. **LibreSSL** is a drop-in
alternative (identical API surface and symbol names) and the binding
auto-detects via `pkg-config` at build time.

**Why OpenSSL, not mbedTLS / BearSSL / wolfSSL / rustls:**

- **OpenSSL** is what every credible web server runs on in production:
  nginx, Apache, curl, Node.js (via crypto), Python's `ssl`, Ruby
  `OpenSSL`, PostgreSQL TLS, HAProxy. It is the industrial default.
- **ABI stable** on the 3.x LTS branch.
- **Packaged everywhere**: every Linux distro, macOS via Homebrew,
  Windows via vcpkg / msys2.
- **mbedTLS / BearSSL** are positioned as *embedded* libraries (the
  "mbed" prefix references ARM Mbed). Technically usable on servers
  but the wrong marketing signal for a production web framework.
- **BoringSSL** has no stable ABI — it is an internal Google fork.
- **wolfSSL** is dual GPL/commercial and oriented toward FIPS
  customers, not general-purpose servers.
- **rustls** is excellent but requires a Rust toolchain at the binding
  layer (rustls-ffi), adding friction for AM contributors. May be
  revisited in v1.x.

**Migration purity:** per [[feedback-migration-purity]], the C bridge
holds only what cannot be expressed in pure AM — handshake state
machine, AEAD ciphers, certificate parsing (X.509 / ASN.1) — i.e. the
parts that delegate to OpenSSL. Hostname verification, PEM parsing
helpers, base64 encoding, ACME ALPN-01 logic are pure AM.

**Public surface:**

```amalgame
namespace Amalgame.Tls

public enum TlsVersion { Tls12, Tls13 }

public class TlsConfig {
    public static TlsConfig Default()
    public TlsConfig WithCertFile(certPath: string, keyPath: string)
    public TlsConfig WithCertBytes(certPem: string, keyPem: string)
    public TlsConfig WithAcme(domain: string, email: string, cacheDir: string)
    public TlsConfig WithClientAuth(caBundlePath: string)
    public TlsConfig WithMinVersion(v: TlsVersion)
    public TlsConfig WithAlpn(protos: List<string>)         // ["h2", "http/1.1"]
    public TlsConfig WithSessionTickets(enabled: bool)
    public TlsConfig WithSni(handler: Function)             // SNI → cert resolver
}

public class TlsContext {
    public static TlsContext Server(cfg: TlsConfig)
    public static TlsContext Client(cfg: TlsConfig)
}

public class TlsStream {
    public static TlsStream Wrap(fd: int, ctx: TlsContext, isServer: bool)
    public int Read(buf: List<int>, max: int)
    public int Write(buf: List<int>)
    public void Close()
    public string PeerCertSubject()
    public string AlpnProto()                                // negotiated proto
    public string TlsVersion()
    public string CipherSuite()
}

// ACME / Let's Encrypt automatic certificate management.
public class AcmeManager {
    public static AcmeManager New(email: string, cacheDir: string, staging: bool)
    public void AddDomain(domain: string)
    public TlsConfig BuildTlsConfig()
    public void StartRenewLoop()                             // background thread
}
```

**Manifest excerpt (`amalgame.toml`):**

```toml
[package]
name              = "amalgame-tls"
version           = "0.1.0"
license           = "Apache-2.0"
description       = "TLS 1.2/1.3 client+server (OpenSSL 3.x binding) with ACME."
required-amalgame = ">=0.8.13"

[stdlib]
class     = "Tls"
header    = "runtime/Amalgame_Tls.h"
namespace = "Amalgame.Tls"

[build]
external = ["openssl >= 3.0"]            # resolved via pkg-config at build time
```

### 4.2 `amalgame-net-http`

**Purpose:** HTTP/1.1 and HTTP/2 client and server primitives. Parsing
is pure AM (string parsing is Amalgame's strength); raw socket I/O is
in `@c {}` (BSD sockets / WinSock); TLS is delegated to `amalgame-tls`;
HTTP/2 frame parsing delegates to **nghttp2** (MIT, ABI-stable,
packaged everywhere).

**Why nghttp2:** the same rationale as OpenSSL. It is what nginx,
curl, Apache `mod_http2`, Envoy, h2o all use. Writing an HTTP/2
implementation from scratch is months of work and a perpetual security
liability.

**Public surface (server-side):**

```amalgame
namespace Amalgame.Net.Http

public class HttpRequest {
    public string Method                     // "GET" / "POST" / ...
    public string Path                       // "/users/42"
    public Map<string, string> Headers
    public Map<string, string> Query         // parsed from Path
    public List<int> Body                    // raw bytes
    public string BodyText()                 // UTF-8 decode
    public Map<string, string> Form()        // urlencoded
    public Map<string, any> Json()           // application/json
    public Map<string, MultipartFile> Multipart()
    public string Cookie(name: string)
    public string RemoteAddr
    public bool IsSecure
    public string HttpVersion                // "HTTP/1.1" or "HTTP/2"
}

public class HttpResponse {
    public static HttpResponse New()
    public HttpResponse Status(code: int)
    public HttpResponse Header(name: string, value: string)
    public HttpResponse Text(s: string)
    public HttpResponse Json(obj: any)
    public HttpResponse Html(s: string)
    public HttpResponse File(path: string)               // streamed, supports Range
    public HttpResponse Bytes(buf: List<int>)
    public HttpResponse Redirect(url: string, permanent: bool)
    public HttpResponse SetCookie(c: Cookie)
    public HttpResponse Stream(producer: Function)       // chunked / SSE
}

public class Cookie {
    public string Name
    public string Value
    public string Domain
    public string Path
    public int MaxAgeSec
    public bool HttpOnly                                  // default true
    public bool Secure                                    // default true if HTTPS
    public string SameSite                                // "Lax" default
}

// Low-level server, used by amalgame-web internally. Public so library
// users can build their own server framework if they want.
public class HttpServer {
    public static HttpServer Listen(addr: string, tls: TlsConfig?)
    public void Serve(handler: Function)                 // Fn: HttpRequest → HttpResponse
    public void Shutdown(timeoutMs: int)
}

// HTTP client, full-featured (TLS, h2, redirects, keep-alive pool).
public class HttpClient {
    public static HttpResponse Get(url: string)
    public static HttpResponse Post(url: string, body: List<int>, contentType: string)
    public HttpRequestBuilder Request(method: string, url: string)
}
```

**Limits enforced by the parser** (configurable, with sane defaults):

| Limit | Default |
|---|---|
| Max request line length | 8 KB |
| Max header count | 100 |
| Max header total size | 64 KB |
| Max body size (in memory) | 10 MB |
| Max body size (streamed to disk) | 1 GB |
| Header timeout | 10 s |
| Body timeout | 30 s |
| Idle keep-alive timeout | 60 s |
| Max keep-alive requests / conn | 1000 |

### 4.3 `amalgame-web` (Mosaic framework)

**Purpose:** the productive layer on top of `amalgame-net-http`.

**Public surface (excerpt):**

```amalgame
namespace Amalgame.Web

public class WebApp {
    public static WebApp New()

    // Programmatic routing
    public WebApp Get(path: string, handler: Function)
    public WebApp Post(path: string, handler: Function)
    public WebApp Put(path: string, handler: Function)
    public WebApp Patch(path: string, handler: Function)
    public WebApp Delete(path: string, handler: Function)
    public WebApp WebSocket(path: string, handler: Function)
    public WebApp Group(prefix: string, build: Function)
    public WebApp Mount(prefix: string, sub: WebApp)

    // Filesystem-based routing
    public WebApp Routes(dir: string)                          // ex: "./app"
    public WebApp Static(prefix: string, dir: string)          // ex: "/public", "./public"

    // Middleware pipeline (order matters: first added = outermost)
    public WebApp Use(middleware: Function)                    // Fn: (Request, Next) → Response

    // Built-in middleware bundles
    public WebApp WithLogging(logger: Logger)
    public WebApp WithCors(cfg: CorsConfig)
    public WebApp WithCsrf(cfg: CsrfConfig)
    public WebApp WithSessions(store: SessionStore)
    public WebApp WithRateLimit(rps: int, burst: int)
    public WebApp WithCompression()                            // gzip + brotli
    public WebApp WithSecurityHeaders()                        // HSTS, X-Frame-*, CSP basic
    public WebApp WithMetrics(path: string)                    // "/metrics" prometheus
    public WebApp WithHealth(livePath: string, readyPath: string)
    public WebApp WithRequestId()                              // injects X-Request-ID

    // State injection (DB pools, config, etc.) — accessible via ctx.State(name)
    public WebApp WithState(name: string, value: any)

    // Listen (blocking until shutdown signal)
    public void Listen(addr: string, tls: TlsConfig?)
    public void ListenAcme(domain: string, email: string)      // bind :80+:443, ACME on
    public void Shutdown(timeoutMs: int)                        // from another thread / signal handler
}

public class WebContext {
    public HttpRequest Request
    public string Param(name: string)                          // route params: [id], [...slug]
    public any State(name: string)
    public Session Session()                                    // requires WithSessions
    public Logger Log
    public string RequestId
}
```

**Project layout convention:**

```
mon-app/
├── amalgame.toml            # package manifest
├── mosaic.toml              # server config (optional, defaults baked in)
├── main.am                  # entry point (optional — mosaic synthesizes one if absent)
├── app/                     # filesystem routes
│   ├── index.am             → GET /
│   ├── about.am             → GET /about
│   ├── _middleware.am       → middleware for all routes under app/
│   ├── _layout.am           → layout wrapper (HTML routes only)
│   ├── users/
│   │   ├── index.am         → GET /users
│   │   ├── [id].am          → GET /users/:id
│   │   └── [id]/posts.am    → GET /users/:id/posts
│   └── api/
│       ├── login.am         → POST /api/login (method from exported Fn name)
│       └── [...path].am     → catch-all route
├── lib/                     # shared AM code (classes, helpers)
│   ├── db.am
│   ├── auth.am
│   └── models/user.am
├── public/                  # static assets, served as-is
│   ├── style.css
│   └── images/
└── data/                    # runtime state (gitignored)
    ├── sessions/            # one JSON file per session
    └── acme/                # ACME certs cache
```

**Route file convention** — each `.am` file under `app/` exports
handler functions named by HTTP method:

```amalgame
// app/users/[id].am
namespace App.Users.Detail

import Amalgame.Web
import App.Lib.Db
import App.Lib.Models.User

public Function Get(req: HttpRequest, ctx: WebContext): HttpResponse {
    let id: string = ctx.Param("id")
    let user: User = Db.FindUser(id)
    if (user == null) {
        return HttpResponse.New().Status(404).Text("User not found")
    }
    return HttpResponse.New().Json(user)
}

public Function Delete(req: HttpRequest, ctx: WebContext): HttpResponse {
    let id: string = ctx.Param("id")
    Db.DeleteUser(id)
    return HttpResponse.New().Status(204)
}
```

**Path parameter syntax:**
- `[id]` → required single segment, captured as `ctx.Param("id")`
- `[...slug]` → catch-all, captured as `ctx.Param("slug")` containing
  `"a/b/c"` joined string
- `_middleware.am` → applies to siblings and descendants
- `_layout.am` → HTML wrapper for descendants

## 5. Execution modes

Three modes, selected by CLI command:

### 5.1 DEV — `mosaic dev`

- Filesystem watcher (`amalgame-io-filewatcher`) on `app/` and `lib/`.
- On startup → `mosaic` calls `amc compile-shared app/` → compiles
  the **entire `app/` tree into a single `.amweb-cache/app.so`**
  (≈500 ms–2 s cold compile depending on project size).
- Requests → `dlopen` the cached `app.so`, dispatch to the matching
  handler by route.
- File change in `app/` or `lib/` → watcher triggers full recompile
  of `app.so`; new `.so` is swapped in atomically on next request.
- Pretty error page: stack trace in AM (not C), source snippet, suggested
  fix on common errors.
- Request log to stdout in human-readable format.
- TLS optional (`--tls-cert`/`--tls-key`) or self-signed via
  `--tls-dev` (localhost cert auto-generated).
- ACME **disabled** in dev mode to avoid Let's Encrypt rate limits.
- `--open` flag: opens the browser automatically at startup (opt-in).

### 5.2 PROD modular — `mosaic serve`

- Reads `dist/` (produced by `mosaic build --modular`):
  ```
  dist/
  ├── mosaic                    # server binary
  ├── app.so                    # entire app/ compiled as one shared lib
  ├── public/                   # assets, served verbatim
  └── mosaic.toml               # server config
  ```
- `app.so` is pre-compiled by the CI/build pipeline; **no runtime
  compilation**.
- File watcher disabled (security: writing to `dist/app.so` would
  not auto-reload).
- Hot reload available via `SIGHUP` or `mosaic reload` command — swaps
  `app.so` atomically by re-`dlopen`ing.
- Use cases:
  - Self-hosted deployments with frequent partial updates
  - Shipping a patch by replacing `app.so` + SIGHUP
  - Separation of server binary and application logic

### 5.3 PROD all-in-one — `mosaic build --mono` → custom binary

- Single statically-linked binary, named after the project (not
  `mosaic`).
- All routes, `lib/`, static assets (embedded), and the Mosaic server
  itself are linked together.
- **No `dlopen` at runtime**, no `.so` files, no filesystem dependency
  beyond `data/`.
- Run: `./bin/<appname>` (or whatever name `amalgame.toml` declares).
- Use cases:
  - Docker / Kubernetes deployments (`FROM scratch` + `COPY bin/app /`)
  - Maximum security (no dynamic code loading)
  - Minimum operational surface (one file to scp, one process to run)

**Both PROD artifacts are produced by `mosaic build`:**

```bash
mosaic build              # produces both ./dist/ (modular) and ./bin/<name> (mono)
mosaic build --modular    # only ./dist/
mosaic build --mono       # only ./bin/<name>
```

## 6. The `mosaic` CLI and the `amc ↔ mosaic` pipeline

`amc` is the Amalgame compiler. It transpiles `.am` to C and compiles
C to `.o` / `.so` / executables. It knows nothing about HTTP.

`mosaic` is the orchestrator CLI delivered by the `amalgame-web`
package. It is itself written in Amalgame and compiled by `amc`. It
invokes `amc` as a subprocess to compile route files and link
artifacts.

```
┌──────────────────────────────────────────────────────────────────┐
│   amc                                                            │
│   ├─ Transpiler / compiler (AM → C → native)                     │
│   └─ Package manager (amc package add ...)                       │
│              ▲                                                    │
│              │ invokes via subprocess                             │
│              │                                                    │
│   mosaic                                                          │
│   ├─ mosaic new <name>      → scaffold project                   │
│   ├─ mosaic dev [--open]    → DEV mode (filewatcher + hot dlopen)│
│   ├─ mosaic build           → both PROD artifacts                │
│   │     [--modular | --mono]                                      │
│   ├─ mosaic serve           → PROD modular runtime                │
│   ├─ mosaic reload          → SIGHUP to running mosaic            │
│   ├─ mosaic routes          → list discovered routes              │
│   ├─ mosaic cache clean     → wipe .amweb-cache                   │
│   ├─ mosaic acme renew      → force ACME renew now                │
│   ├─ mosaic supervisor <dir>→ launch all sites in <dir>/ (v1.0)  │
│   └─ mosaic version                                                │
└──────────────────────────────────────────────────────────────────┘
```

**Install pipeline:**

```bash
amc package add web
# 1. Resolves amalgame-web → also installs amalgame-tls + amalgame-net-http
#    (and external C deps: openssl, nghttp2 — resolved via pkg-config)
# 2. Fetches package sources from github.com/amalgame-lang/amalgame-web
# 3. Builds the `mosaic` CLI from source (amc compiles ~30s to 2 min one-shot)
# 4. Places `mosaic` in ~/.amalgame/bin/ (already in PATH via shell hook)
```

**Distribution model (Option A — compile from source, definitive):**

The `mosaic` binary is **always built from source** at install time.
We do not maintain pre-built binaries. Rationale:

1. The user already has `amc` and a working C toolchain — building
   Mosaic adds no new prerequisite.
2. The Amalgame ecosystem convention is "source + amc on install"
   (cf. `regex`, `compress` packages with C build steps). A pre-built
   binary for one package would be the sole exception.
3. Initial install cost (30 s to 2 min, one-shot) is comparable to
   `npm install` or `cargo install` for projects of similar scope.
4. Maintaining per-OS-per-arch CI release pipelines, code signing
   (macOS notarization, Windows Authenticode), and the dual-path
   logic for "binary missing → fall back to source" carries a real
   recurring cost for a marginal one-time UX gain.
5. Compiling Mosaic in front of the user is live evidence that the
   framework — written entirely in Amalgame — compiles from clean
   source on their machine. This is a feature, not a wart.

Binary distribution may be revisited if (a) Mosaic's compile time
exceeds 5 min on a typical machine, or (b) install friction blocks
adoption in a measurable way. Until then: source only.

## 7. Concurrency model

**One thread per active request, worker pool, blocking I/O.**
This is the Go `net/http` model, simplified.

```
                    ┌─────────────────────┐
                    │  acceptor thread    │
                    │  loop: accept()     │
                    └──────────┬──────────┘
                               │ push socket
                               ▼
                    ┌─────────────────────┐
                    │  bounded MPSC queue │
                    │  (backpressure)     │
                    └──────────┬──────────┘
                               │ pop
                ┌──────────────┼──────────────┐
                ▼              ▼              ▼
        ┌───────────┐  ┌───────────┐  ┌───────────┐
        │ worker 1  │  │ worker 2  │  │ worker N  │
        │ parse req │  │ parse req │  │ parse req │
        │ route     │  │ route     │  │ route     │
        │ handle    │  │ handle    │  │ handle    │
        │ respond   │  │ respond   │  │ respond   │
        └───────────┘  └───────────┘  └───────────┘
```

- N defaults to `2 × NumCpus`, configurable via `mosaic.toml`.
- Queue capacity defaults to `N × 4`; when full, server returns 503
  immediately rather than risk OOM.
- Built on `amalgame-threading` (already in the ecosystem).
- Graceful shutdown: acceptor stops accepting → workers drain queue
  → workers exit when their current request finishes → process exits.
- Per-request memory: ~100 KB (thread stack + request heap). 256
  active workers ≈ 25 MB for concurrency itself; plus ~20-50 MB
  process baseline.

**Why not event loop (Node.js model):**

- Amalgame has no `async`/`await`. Adding it is a major language
  change ("function colors" plague the codebase).
- A single CPU core would be used; multi-core requires cluster/worker
  pattern → complication leaking through the framework API.
- A blocking handler (heavy DB query, file I/O) stalls the entire
  server.
- The industry trend in 2024-2026 (Axum, Hono on Bun, Phoenix
  LiveView, ASP.NET Minimal API, Spring WebFlux) is thread-pool or
  hybrid models, not pure event loop.

**State sharing rules (must be documented prominently):**

- `HttpRequest` and `WebContext` are per-request, never shared.
- Globals injected via `WithState()` must be thread-safe by their own
  construction (mutex internal, atomic, or immutable). The framework
  does not protect them.
- Provided thread-safe primitives: DB connection pools (in DB
  packages), `MemorySessionStore` (mutex-guarded map), `RateLimiter`.
- Anti-pattern: route file declares a global mutable `var counter:
  int = 0`. Documented as forbidden. Linter check considered for
  v1.x.

## 8. Cache strategy

```
mon-app/
└── .amweb-cache/                  # gitignored
    ├── meta.json                  # source tree hash → app.so path
    ├── app.so                     # entire app/ compiled as one .so
    └── tmp/                       # in-flight compilation
```

- Default location: `./.amweb-cache/` in the project root.
- Override: `AMWEB_CACHE_DIR=/var/cache/mosaic/<app>` env var (useful
  for multi-instance deployments where the project dir is read-only).
- `meta.json` stores the aggregate SHA-256 of the entire `app/` tree
  **and `lib/`**. Any change to any source file triggers a full
  recompile of `app.so` — simple and correct.
- Compilation is atomic: build into `tmp/app.so`, then `rename()`.
  A partially-written `.so` is never loaded.
- `mosaic cache clean` wipes the cache. `mosaic dev --no-cache`
  forces full recompile.
- Per-file invalidation (one `.so` per route) may be revisited in
  v1.x if compile time on large projects becomes a bottleneck.

## 9. Sessions

Three backends shipped, selected at app boot:

```amalgame
app.WithSessions(MemorySessionStore.New())            // dev / micro-traffic
app.WithSessions(JsonFileSessionStore.New("data/sessions/"))  // single-node prod default
app.WithSessions(RedisSessionStore.New(redisUrl))     // multi-node prod
```

| Backend | Storage | Concurrency | Use case |
|---|---|---|---|
| `MemorySessionStore` | in-process map | mutex-guarded | dev, tests, single-node low-traffic |
| `JsonFileSessionStore` | **one JSON file per session ID** in a directory | per-file `flock()`, no global mutex | **default prod single-node** |
| `RedisSessionStore` | Redis hashes | network-atomic | multi-node prod |

**Why one JSON file per session, not a single `sessions.json`:**

- A single file requires a global write mutex → contention spike under
  concurrent writes.
- One file per session ID localizes locking → no global hot path.
- Lifecycle cleanup is trivial: `find data/sessions -mtime +30 -delete`.
- Debuggable with `cat` / `jq`.

**Why not SQLite by default:** overkill for sessions. Recommended only
when the app already uses SQLite for other reasons.

**Session API:**

```amalgame
public class Session {
    public string Id
    public any Get(key: string)
    public void Set(key: string, value: any)
    public void Delete(key: string)
    public void Clear()
    public void Regenerate()                           // new ID, anti-fixation
    public int MaxAgeSec
}
```

Sessions use a `Secure`, `HttpOnly`, `SameSite=Lax` cookie by default.
The session ID is a 256-bit random value (from `amalgame-random`),
base64url-encoded.

## 10. Templating

**v0.1 — v0.5: functional HTML helpers.** No parser changes to AM.

```amalgame
import Amalgame.Web.Html
// imports: Tag, Text, Fragment, Escape, Attr

public Function Get(req: HttpRequest, ctx: WebContext): HttpResponse {
    let users: List<User> = Db.AllUsers()
    let html: HtmlNode = Tag("html", [],
        Tag("head", [], Tag("title", [], Text("Users"))),
        Tag("body", [],
            Tag("h1", [], Text("All users")),
            Tag("ul", [],
                users.Map(fn(u) {
                    return Tag("li", [], Text(u.Name))
                })
            )
        )
    )
    return HttpResponse.New().Html(html.Render())
}
```

- `Text(...)` auto-escapes (XSS-safe by default).
- Raw HTML opt-in via `RawHtml(s)`.
- Components are just functions returning `HtmlNode`.
- This is the [lit-html](https://lit.dev/) / [Hyperscript](https://github.com/hyperhype/hyperscript) pattern.

**v1.0+ — JSX-like literals (if amc parser extension lands):**

```amalgame
public Component UserCard(user: User): HtmlNode {
    return <div class="card">
             <h2>{user.Name}</h2>
             <p>{user.Email}</p>
           </div>
}
```

This requires extending the AM parser to recognize `<tag>...</tag>`
expressions and lower them to `Html.Tag(...)` calls. Significant work
in amc itself; tracked as a separate proposal if/when prioritized.

**htmx-first stance.** The framework documentation and examples
favor server-rendered HTML with htmx for interactivity over building
a JSON API consumed by a separate SPA. Reasoning:

- Productive: no separate frontend build.
- Performant: HTML over the wire is small, network-cacheable.
- Aligned with the framework's strengths: SSR + middleware + sessions.
- For users who want SPA, the JSON API path is fully supported.

## 11. Security

### 11.1 Built-in middleware (always available, opt-in via `.With*()`):

| Concern | Middleware | Default |
|---|---|---|
| CSRF | `WithCsrf()` | double-submit cookie + per-form token |
| CORS | `WithCors(cfg)` | deny by default; explicit allow list |
| Security headers | `WithSecurityHeaders()` | HSTS, X-Frame-Options=DENY, X-Content-Type-Options=nosniff, Referrer-Policy=strict-origin-when-cross-origin, basic CSP |
| Rate limit | `WithRateLimit(rps, burst)` | per-IP token bucket |
| Compression | `WithCompression()` | gzip + brotli (Accept-Encoding negotiated) |
| Body size | inherent (parser limit) | 10 MB request body |
| Request ID | `WithRequestId()` | injects/echoes X-Request-ID |

### 11.2 Anti-DDoS — layered defense

The framework provides L7 (application-layer) defenses. L3/L4
defenses are the operator's responsibility (kernel sysctl, firewall,
CDN).

**L7 built-in:**

- **Rate limiting** per IP — `WithRateLimit(rps: 100, burst: 200)`.
- **Connection limits** per IP — max N concurrent connections from
  the same address; further connections rejected with TCP RST.
- **Slow loris protection** — header timeout (10 s default), body
  timeout (30 s), idle keep-alive timeout (60 s). A client sending
  bytes too slowly gets disconnected.
- **Size limits** — request line, header count, header total, URL
  length, body size. Enforced by the parser, not the application.
- **Backpressure** — when the worker queue is full, return 503
  immediately. Never let the queue grow unbounded into OOM.
- **Connection ageing** — keep-alive connections forcibly closed
  after N requests or X minutes, to bound resource hoarding.
- **Trusted proxy header allow-list** — `X-Forwarded-For` only
  trusted from explicitly whitelisted IPs (configurable). Otherwise
  spoofing client IP is trivial.

**L7 advanced — opt-in middleware (v1.0+):**

- Bot detection (proof-of-work / JS challenge).
- Geo-IP blocking via MaxMind GeoLite2.
- IP reputation list (AbuseIPDB, Spamhaus).
- Honeypot endpoints (`/.env`, `/wp-admin`, `/.git/config`) → auto-ban.

**L3/L4 — operator's responsibility, documented in README:**

- SYN cookies (`net.ipv4.tcp_syncookies=1`)
- `nf_conntrack` limits
- Per-source connection rate limit via `iptables` / `nftables`
- For volumetric attacks (>10 Gbps): CDN (Cloudflare, Fastly) is
  the only realistic defense, no application-layer framework
  competes.

### 11.3 Cookie security

Default for all cookies set via `HttpResponse.SetCookie()`:

- `Secure` — set automatically when the request is HTTPS.
- `HttpOnly` — true by default. Opt out explicitly if needed.
- `SameSite=Lax` — default. `Strict` or `None` available.
- Default `Path=/`.

### 11.4 Password hashing

`amalgame-crypto` does not exist yet (see §21 phase 2 item 2.1). When
it lands, it must ship **Argon2id** alongside HMAC + asymmetric
primitives. SHA-based hashing is not acceptable for password
storage.

Tracked as critical precondition in §21.3 — every auth flow and the
native ACME implementation block on it.

## 12. Observability

- **Structured logging** via `amalgame-logging` (already in
  ecosystem). Default fields per request: `request_id`, `method`,
  `path`, `status`, `duration_ms`, `remote_addr`, `user_agent`,
  `referer`.
- **Metrics endpoint** `/metrics` in Prometheus exposition format:
  - `http_requests_total{method,path,status}`
  - `http_request_duration_seconds_bucket{...}` (histogram)
  - `http_requests_in_flight`
  - `mosaic_workers_busy`
  - `mosaic_queue_depth`
  - `tls_handshakes_total{result}`
- **Health endpoints:**
  - `/healthz` (liveness — process is running)
  - `/readyz` (readiness — DB connectable, dependencies up)
- **Request ID** — `WithRequestId()` injects/echoes `X-Request-ID`,
  propagates into logs and `ctx.RequestId`.
- **OpenTelemetry** — out of scope for v1.0, but the request ID
  propagation is designed to be compatible with OTel trace IDs in
  the future.

## 13. WebSocket

Implementation strategy: **extend `amalgame-net-websocket`** to add
server-side framing. The existing client-side implementation already
contains the protocol primitives (SHA-1 + Base64 handshake, frame
encoding). Adding the server path is mechanical, no third-party lib.

Mosaic integration: WebSocket endpoints register on the same router
as HTTP routes.

```amalgame
app.WebSocket("/chat", fn(ws: WebSocketConn, ctx: WebContext) {
    while (ws.IsConnected()) {
        let msg: string = ws.RecvText()
        ws.SendText("echo: " + msg)
    }
})
```

**Built-in pub/sub channel system (Phoenix-like, lightweight):**

```amalgame
let chat: Channel = app.Channel("/chat")             // a topic

app.WebSocket("/chat", fn(ws, ctx) {
    chat.Subscribe(ws)                                // ws now receives broadcasts
    while (ws.IsConnected()) {
        let msg: string = ws.RecvText()
        chat.Broadcast(msg)                           // sent to all subscribers
    }
})
```

Scope v0.3:

- Text + binary frames.
- Per-message-deflate extension.
- Heartbeat (ping/pong) and idle disconnect.
- Pub/sub broadcasting within a single process (multi-node pub/sub
  via Redis Pub/Sub deferred to v1.x).

## 14. Reverse proxy

`amalgame-web` includes a minimal but credible reverse-proxy mode for
the common case where you want Mosaic to proxy a few routes to
upstream services without needing nginx.

```amalgame
app.Proxy("/api/*",  "http://backend-1:8080")
app.ProxyPool("/api/*",
              ["http://b1:8080", "http://b2:8080", "http://b3:8080"],
              LoadBalance.RoundRobin)
app.Proxy("/ws/*",   "ws://backend:9000")              // WebSocket proxy
```

Features:
- HTTP/1.1 and HTTP/2 proxying.
- Path-based dispatch (Host-based dispatch is out of scope, see §16).
- Load balancing strategies: round-robin, least-connections, IP-hash.
- Active health checks (periodic ping; remove unhealthy backends
  from rotation).
- Circuit breaker pattern (open after N consecutive failures).
- Header rewriting (`X-Forwarded-For`, `X-Forwarded-Proto`,
  `X-Forwarded-Host` auto-set).
- TLS termination at the proxy (frontend HTTPS, backend HTTP allowed).

Out of scope:
- Caching reverse proxy (Varnish replacement) — v1.x.
- Streaming media (HLS/DASH segmentation) — v1.x.
- WAF rules — v1.x.

## 15. ACME / Let's Encrypt

Automatic HTTPS, implemented purely in Amalgame on top of
`amalgame-tls` and `amalgame-net-http` client. **No third-party ACME
client dependency.**

```amalgame
app.ListenAcme("example.com", "admin@example.com")
// Mosaic now:
//   1. Binds :80 (HTTP, for tls-alpn-01 challenge fallback) and :443.
//   2. On first :443 request for example.com:
//      a. Generates account key + CSR.
//      b. Solicits Let's Encrypt over ACME v2.
//      c. Solves tls-alpn-01 challenge in-process (no HTTP-01 indirection).
//      d. Receives cert, caches to data/acme/example.com/.
//      e. Completes the original handshake.
//   3. Background thread renews 30 days before expiry.
```

Supported challenge types:
- **tls-alpn-01** (preferred, in-process, no extra port management).
- **http-01** (fallback if tls-alpn-01 fails; requires :80 access).
- **dns-01** (out of scope v1.0 — requires DNS API integration).

Multi-domain: `app.ListenAcme(["example.com", "www.example.com"], ...)`
issues one cert with SAN for all listed domains.

Staging / production: env var `ACME_STAGING=1` uses Let's Encrypt
staging endpoint (rate-limit-free, certs untrusted) for testing.

## 16. Multi-site / hosting decision

**Decision: one Mosaic binary serves one site. Multi-site is out of
scope.**

Rationale:
- One-site-per-binary keeps the mental model simple and aligns with
  the Docker/k8s deployment idiom (one image, one service, one
  domain).
- Multi-tenant hosting in a single process introduces blast-radius
  problems (one site crashes them all), isolation concerns, and
  significant additional configuration surface.
- Users who need multi-site hosting can run multiple Mosaic
  processes behind a reverse proxy (nginx, Caddy, Traefik). This is
  the prevailing industry pattern and is well-documented elsewhere.
- Users who need lightweight multi-domain on a single binary (e.g.
  `www.example.com` and `api.example.com` served from the same
  app) can use a single Mosaic app with path/host-based routing
  inside their app code — no special "vhost" feature needed.

Process-isolated multi-tenant hosting may be revisited in v1.x+ if
demand materializes.

## 17. Production deployment

### 17.1 Directory-per-site pattern

Each production site is an independent directory containing its binary
(or `app.so`), config, and assets. Sites run as independent processes
behind a reverse proxy (nginx, Caddy, Traefik) routing by hostname.

```
/opt/sites/
├── blog/
│   ├── blog           ← mono binary (mosaic build --mono)
│   └── mosaic.toml    ← listen: :3001
├── api/
│   ├── api
│   └── mosaic.toml    ← listen: :3002
└── shop/
    ├── app.so         ← modular build (mosaic build --modular)
    ├── mosaic
    └── mosaic.toml    ← listen: :3003
```

Each site crashes and restarts independently. Adding or removing a site
is as simple as adding or removing a directory.

### 17.2 OS service integration — `amalgame-service`

The existing `amalgame-service` package covers both production OS
targets. No new tooling is required.

**Linux (systemd):**

```bash
mosaic build --mono
sudo ./install.sh      # registers + starts the systemd unit
sudo journalctl -fu my-app
```

Generated `.service` unit:

```ini
[Service]
ExecStart=/opt/sites/my-app/my-app
WorkingDirectory=/opt/sites/my-app
Restart=on-failure
```

**Windows (Windows Service Control Manager):**

```powershell
.\build.ps1
Start-Process powershell -Verb runAs .\install.ps1
# Registers via sc.exe, starts the Windows Service natively.
# No NSSM or third-party wrapper required (amalgame-service ≥ v0.2.0).
```

### 17.3 `mosaic supervisor` (v1.0 — convenience)

For dev multi-site and hosting contexts without systemd (macOS,
lightweight VPS):

```bash
mosaic supervisor /opt/sites/
# Scans subdirectories, launches each site's binary / mosaic serve,
# restarts on crash, SIGHUP for reload, aggregated logs to stdout.
```

This is a convenience wrapper — for serious production Linux/Windows
deployments, prefer `amalgame-service` + systemd/SCM.

## 18. Configuration

Two config files coexist:

**`amalgame.toml`** — package manifest (same as every AM project).
Describes the package, its name, version, dependencies.

**`mosaic.toml`** — server runtime config (optional; defaults apply
if absent):

```toml
[server]
listen     = [":3000"]                    # or [":80", ":443"] in prod
workers    = 0                             # 0 = 2*cpus
queue_size = 0                             # 0 = workers*4

[tls]
mode       = "acme"                        # "acme" | "files" | "off"
acme_email = "admin@example.com"
acme_cache = "./data/acme"
domains    = ["example.com", "www.example.com"]
# OR for mode = "files":
# cert_file = "./certs/server.pem"
# key_file  = "./certs/server.key"

[sessions]
backend     = "json_file"                  # "memory" | "json_file" | "redis"
dir         = "./data/sessions"
# url         = "redis://localhost:6379"     for redis backend
max_age_sec = 86400

[security]
rate_limit_rps   = 100
rate_limit_burst = 200
trusted_proxies  = ["10.0.0.0/8", "127.0.0.1"]

[logging]
level  = "info"
format = "json"                            # "json" | "text"

[limits]
max_body_size_mb       = 10
header_timeout_sec     = 10
body_timeout_sec       = 30
idle_keepalive_sec     = 60
```

Env vars override config file (`MOSAIC_LISTEN=:8080`,
`MOSAIC_TLS_MODE=acme`, etc.).

CLI flags override env vars.

**Full per-key reference** (including env-var names, types, defaults
and library mappings for every section above and the planned
`[security.headers]` / `[security.cors]` / `[security.csrf]` /
`[security.rate_limit]` / `[logging]` / `[limits]` tables): see
[`docs/mosaic-configuration.md`](../mosaic-configuration.md).

## 19. Naming conventions

| Layer | Name |
|---|---|
| amc package | `amalgame-web` (+ `amalgame-tls`, `amalgame-net-http`) |
| AM namespace | `Amalgame.Web` (+ `Amalgame.Tls`, `Amalgame.Net.Http`) |
| CLI binary | **`mosaic`** |
| Repository slug | `amalgame-lang/amalgame-web` (+ tls, net-http) |
| Marketing / talk title | "Mosaic — the Amalgame web framework" |

"Mosaic" is used only for marketing surface (repo README headline,
blog posts, conference talks). Code, manifests, namespaces, doc
references all use `Amalgame.Web`. The name nods thematically to
"Amalgame" (both terms evoke assembled pieces).

## 20. External dependencies

| Dep | Package providing | License | Resolved via |
|---|---|---|---|
| OpenSSL 3.x (or LibreSSL) | `amalgame-tls` | Apache-2.0 (OpenBSD-style for LibreSSL) | pkg-config at build |
| nghttp2 | `amalgame-net-http` | MIT | pkg-config at build |
| (zlib via amalgame-compress) | for gzip compression | zlib | already in ecosystem |

Build-time toolchain (already required by amc): GCC or Clang, pkg-config.

No vendored copies of OpenSSL or nghttp2. They are linked dynamically
against the system libraries. Distros ship them; on macOS we depend
on Homebrew (`brew install openssl@3 nghttp2`); on Windows on msys2
or vcpkg.

This is identical to how every other production HTTP/TLS stack
(curl, nginx, Node.js) works.

## 21. Roadmap

### 21.1 Shipped

Two intensive days (2026-05-18 and 2026-05-19) took the stack from
"nothing yet" through "HTTPS + HTTP/2 + WebSocket + filesystem routing
+ livereload, end-to-end".

| Package / Version | Highlights | Date |
|---|---|---|
| `amc` v0.8.29 → v0.8.34 | Multi-class manifest; first-class `Closure`; libcurl ejection; nested-MEMBER closure dispatch; PkgClassMangledPrefix multi-class fix; full Mac/Win release | 2026-05-18..19 |
| `amalgame-tls` v0.1.0 → v0.2.0 | OpenSSL 3.x binding (multi-OS) + raw-byte read/write helpers + **ACME wrapper** (`Acme.EnsureCert` via certbot --standalone) + `Acme.ChallengeServer` for webroot-mode | 2026-05-18..19 |
| `amalgame-net-curl` v0.1.0 → v0.1.1 | Thin libcurl binding (Get/Post/Put/Delete/Patch) — companion to net-http for outbound calls | 2026-05-18 |
| `amalgame-net-http` v0.1.0 → **v0.4.1** | HTTP/1.1 pure-AM parser; **`Http1.Serve`** (browser-friendly); **`Http2.Serve`** h2c via nghttp2 binding; **`Https.Serve`** HTTPS + ALPN h2; **`Ws.Serve`** WebSocket (RFC 6455); **`Wss.Serve`** WebSocket over TLS | 2026-05-18..19 |
| `amalgame-web` v0.1.0 → v0.2.1 | Router (`:param` + `*splat`); Route + Closure handlers; `MemorySessionStore`; `WebContext`. Pure runtime library — the build tool moved out (see Decisions §23). | 2026-05-18..19 |
| `mosaic` v0.1.0 → v0.3.1 | **Standalone repo** (build tool, not a package). `tools/mosaic-routes.sh` (Next.js-style filesystem routing, `[id]`/`[...slug]` syntax). `tools/mosaic-build.sh` (regen routes + amc + gcc). `tools/mosaic-dev.sh` (filewatcher + auto-rebuild + restart). **Livereload daemon** (compiled at first run, broadcasts WS "reload" → browser auto-refresh on every save). | 2026-05-19 |

Live verified: 4-route filesystem-routed Mosaic app served from
`mosaic-fs-demo/` in HTTP/1.1, HTTP/2 (h2c), HTTPS+ALPN h2 and
WebSocket. Browser livereload working end-to-end. All packages
registered on `amalgame-lang/packages-index`; all CIs green
against amc v0.8.34.

### 21.2 Remaining work — 4 phases

Effort estimates assume focused full-time work. Total ~8-12 weeks
to reach feature parity with mainstream web stacks
(Rails / Django / Phoenix / SvelteKit) **plus** production-grade
security.

#### Phase 1 — Production hardening (~2-3 weeks)

Blocking for any real prod use. Without these, security audits
fail and throughput is misery.

| # | Item | Effort | Depends |
|---|---|---|---|
| 1.1 | **Security pack** in `amalgame-web` — `WithSecurityHeaders` / `WithCors` / `WithCsrf` / `WithRateLimit` / `WithRequestId` middlewares (see §11) | 2-3 d | — |
| 1.2 | **Fix command injection** in `tls`'s `Acme.EnsureCert` — replace `system(3)` with `execve(argv)` so user-supplied `domain` cannot inject shell metacharacters | 1 h | — |
| 1.3 | **Anti-injection guards** — CRLF in `HttpResponse.Header(name, value)`, open-redirect in `Redirect(url)`, SSRF in `HttpClient.Get(url)` (refuse RFC1918 by default with explicit opt-out) | 1 d | — |
| 1.4 | **Slowloris timeouts** — `SO_RCVTIMEO` / `SO_SNDTIMEO` on accept; per-connection idle timeout | 0.5 d | — |
| 1.5 | **Configurable limits** — body / header / URL length tunable, not hardcoded | 0.5 d | — |
| 1.6 | **`amalgame-threading`** package — pthread bindings (mutex, cond var, thread create/join) | 2 d | — |
| 1.7 | **Worker pool** in `Http*.Serve` / `Ws*.Serve` — N concurrent connections instead of 1 | 1-2 d | 1.6 |
| 1.8 | **HTTP/1.1 keep-alive** in `Http1.Serve` | 1 d | — |
| 1.9 | **IPv6 (dual-stack)** — bind `AF_INET6` with `IPV6_V6ONLY=0` everywhere | 0.5 d | — |
| 1.10 | **Graceful shutdown** — SIGTERM handler drains in-flight requests before exit | 0.5 d | 1.6 |

#### Phase 2 — Real-world apps (~3-4 weeks)

Past phase 1, the stack can serve traffic safely. Phase 2 turns it
into something that runs real apps.

| # | Item | Effort | Depends |
|---|---|---|---|
| 2.1 | **`amalgame-crypto`** — Argon2id, HMAC (HS256/384/512), RSA, ECDSA, Ed25519, CSPRNG. Precondition for auth + native ACME + session signing | 2-3 d | — |
| 2.2 | **`amalgame-web-auth`** (or `web/auth` namespace) — Basic + Bearer/JWT + API key + session login flow middlewares | 2 d | 2.1 |
| 2.3 | **OAuth 2.0 / OIDC client** — Google / GitHub / etc. SSO | 2 d | 2.2 |
| 2.4 | **WebAuthn / passkeys** — passwordless auth | 3-4 d | 2.1 |
| 2.5 | **TOTP / 2FA** — RFC 6238 (HMAC-SHA1 over time counter) | 0.5 d | 2.1 |
| 2.6 | **`amalgame-database-sqlite`** — start small with file-based DB | 2 d | — |
| 2.7 | **`amalgame-database-postgres`** — libpq binding for prod | 3-4 d | — |
| 2.8 | **Migration framework** — schema versioning + apply/rollback | 1 d | 2.6 / 2.7 |
| 2.9 | **`amalgame-template`** — HTML template engine with **auto-escape** (closes the default-XSS hole) | 2-3 d | — |
| 2.10 | **`amalgame-validation`** — typed input schemas | 1-2 d | — |
| 2.11 | **`JsonFileSessionStore` / `RedisSessionStore`** in `amalgame-web` | 1 d | 2.7 (Redis) |
| 2.12 | **`router.Ws(path, handler)`** — register WS routes through the Router instead of side-binding | 1 d | — |
| 2.13 | **Multipart parser** — `HttpRequest.File("avatar")` for upload | 1.5 d | — |

#### Phase 3 — UX + ops (~1-2 weeks)

Quality of life. Better dev experience, smoother prod deploys.

| # | Item | Effort | Depends |
|---|---|---|---|
| 3.1 | **Structured access logs** — JSON or Common Log Format | 1 d | — |
| 3.2 | **`/metrics` endpoint** — Prometheus exposition format | 1 d | — |
| 3.3 | **OpenTelemetry tracing** — request spans propagated through middleware chain | 2 d | — |
| 3.4 | **`/healthz` + `/readyz`** middleware | 0.5 d | — |
| 3.5 | **systemd `Type=notify` + watchdog** integration | 0.5 d | — |
| 3.6 | **`mosaic new <template>`** — scaffolding | 1 d | — |
| 3.7 | **dlopen hot-swap** in `mosaic dev` — `app.so` rebuild + hot reload so in-flight WS connections survive | 3-4 d | 1.6 |
| 3.8 | **Native `mosaic` binary** — rewrite the bash scripts as `src/*.am`, compile via amc into a single binary | 1-2 d | — |
| 3.9 | **Source mapping** — amc errors point at the user's `app/X.am`, not the generated `_routes.am` | 2 d | 3.7 (cleaner with dlopen path) |
| 3.10 | **`mosaic test`** — harness for integration tests against a running app | 1-2 d | — |
| 3.11 | **Docker / systemd templates** — drop-in deployment recipes | 0.5 d | — |

#### Phase 4 — Polish + breadth (~2-3 weeks)

Past phase 3, Mosaic is feature-complete for serious deployments.
Phase 4 fills in the long tail.

| # | Item | Effort | Depends |
|---|---|---|---|
| 4.1 | **Native pure-AM ACME** (RFC 8555) — replaces the certbot wrapper. JWS account signing + order/auth/finalize state machine + CSR DER + cert chain parsing. `Acme.EnsureCert` API stays stable. | 2-3 d | 2.1 |
| 4.2 | **`Acme.WithDirectory(url)`** — other CAs (Buypass, ZeroSSL, Google Trust Services) | 0.5 d | 4.1 |
| 4.3 | **DNS-01 challenges + DNS provider bindings** — wildcards via Cloudflare / Route53 / OVH / Gandi | 3-4 d | 4.1 |
| 4.4 | **mTLS client cert auth** in `TlsConfig` + surfaced as `WebContext.ClientCert` | 1 d | — |
| 4.5 | **OCSP stapling** — TLS server attaches cached OCSP responses | 1 d | — |
| 4.6 | **HTTP/1.1 fallback in `Https.Serve`** — ALPN advertises both `h2` and `http/1.1`, dispatch by negotiated proto | 1 d | — |
| 4.7 | **Static file middleware** — `sendfile(2)` + built-in MIME DB + `Cache-Control` + `ETag` / `If-None-Match` | 1-2 d | — |
| 4.8 | **gzip / brotli compression** — output negotiated via `Accept-Encoding` | 1 d | — |
| 4.9 | **Range requests (206 Partial Content)** — video / big-file resume | 1 d | — |
| 4.10 | **Server-Sent Events (SSE)** — lightweight unidirectional push | 1 d | — |
| 4.11 | **WebSocket binary frames** — AM-side `WsConn.SendBinary` / `ReceiveBinary` (List<int>) | 0.5 d | — |
| 4.12 | **WebSocket fragmentation** — multi-frame messages (currently rejected) | 1 d | — |
| 4.13 | **WebSocket subprotocol negotiation** — `Sec-WebSocket-Protocol` | 0.5 d | — |
| 4.14 | **HTTP/3 / QUIC** — likely via ngtcp2 or msquic when implementations stabilise | future | — |
| 4.15 | **`amalgame-email`** — SMTP client (transactional mail) | 1-2 d | — |
| 4.16 | **`amalgame-queue`** — background jobs over DB / Redis | 2 d | 2.6 |
| 4.17 | **`amalgame-cron`** — scheduled tasks | 1 d | — |
| 4.18 | **`amalgame-i18n`** — locale-aware responses, Accept-Language | 1 d | — |
| 4.19 | **`amalgame-openapi`** — generate `swagger.json` from registered routes | 2 d | — |
| 4.20 | **`Closure<A, R>` typed closures** in amc — removes the `-Wno-int-conversion` workaround currently in `mosaic-build.sh` | 1-2 d | — |
| 4.21 | **WAF rule engine** — opt-in middleware with anti-bot, SQL/XSS pattern detection | 2 d | — |

### 21.3 Critical preconditions

Three packages underpin most of phases 1-4:

1. **`amalgame-threading`** (1.6) — without it, every server runs 1
   connection at a time. Unusable past a demo.
2. **`amalgame-crypto`** (2.1) — without it: no auth, no JWT, no
   session signing, no native ACME.
3. **`amalgame-template`** (2.9) — without it, users hand-concat HTML
   strings, which is both ergonomically miserable and an XSS hole
   by default.

Shipping these three early unblocks everything else.

### 21.4 Suggested order (first ~25 days of focused work)

1. **1.2 + 1.3** — injection fixes (sec-critical, cheap)
2. **1.6 + 1.7** — threading + worker pool (unblocks concurrency)
3. **1.1** — security headers / CORS / CSRF / rate limit (audit-passable)
4. **2.1** — `amalgame-crypto` (unblocks auth + JWT + ACME native)
5. **2.6** — SQLite (unblocks persistence)
6. **2.9** — template engine + auto-escape (real apps + anti-XSS)
7. **2.2** — auth Basic + Bearer + sessions (MVP login flow)
8. **3.1** — access logs (debuggable in prod)
9. **3.4 + 3.5** — `/healthz` + systemd (deployable)
10. **4.1** — native ACME (removes certbot dependency)

After step 10 the stack is genuinely prod-ready: routing, auth,
sessions, DB, templates, security middleware, HTTPS auto-renew,
observability, systemd integration. The rest (phase 4 polish,
extra DBs, OAuth/WebAuthn, etc.) is breadth.

## 22. Future work (post-v1.0)

Items that are intentionally **not** in §21's phases — too speculative,
too niche, or blocked on out-of-scope language work.

- **JSX-like template literals** in the amc parser. Requires a
  separate proposal for the syntax + a parser extension. v2.0+
  candidate.
- **Multi-site hosting in a single Mosaic process** (one process,
  multiple virtual hosts behind a single port). Industry pattern
  is one process per site behind a reverse proxy; this option
  remains for resource-constrained shared-hosting scenarios.
  Separate proposal if demand surfaces.
- **Process-isolated workers** — one OS process per route, Phusion
  Passenger model. Stronger crash isolation than the thread-pool
  model, at the cost of complexity and memory.
- **Built-in caching reverse proxy** — Varnish replacement integrated
  with Mosaic. Out of scope until Mosaic's own reverse proxy
  (phase-deferred — was originally web v0.6) ships.
- **Cluster mode** — multi-host shared state via Redis or a
  coordination service. Application-layer feature; outside the
  framework's primary remit.
- **Built-in admin UI** — à la Caddy admin API or Traefik dashboard.
  Would expose Mosaic's runtime stats / routes / certs / sessions
  through a `/_admin` endpoint with auth.
- **Multi-protocol fast-CGI / SCGI / WAI gateways** — for embedding
  Mosaic inside other web stacks. Unlikely to ever be a priority.

(Items previously listed here — HTTP/3, WAF, DNS-01 wildcards,
native ACME, pre-built mosaic binary, OpenTelemetry full — have
moved into §21's phase 3 or 4.)

## 23. Decisions log (record of rejected paths)

1. **mbedTLS / BearSSL / wolfSSL rejected** in favor of OpenSSL 3.x:
   industry credibility, ABI stability, ubiquitous packaging.
2. **rustls rejected for v1.0** because rustls-ffi adds a Rust
   toolchain prerequisite for AM contributors. Possible revisit v1.x.
3. **Event loop (Node.js) model rejected** in favor of thread pool:
   no `async`/`await` in AM, multi-core natively, simpler debugging,
   industry trend toward thread/hybrid models since 2020.
4. **Sub-process CGI rejected**: forks per request cap throughput at
   a few hundred req/s, kills the keep-alive story, makes DB pooling
   per-request impossible.
5. **Custom in-process JIT rejected**: amc is a transpiler, no
   interpreter. Building one is months of work for marginal gain
   over the dlopen-`.so` model.
6. **SQLite session store dropped as default** in favor of one JSON
   file per session: simpler, debuggable, no SQLite dependency if
   the app doesn't use SQLite for anything else.
7. **Multi-site hosting in one process** rejected for v1.0: complexity
   not justified by the small portion of users it serves; external
   reverse proxy is the industry pattern.
8. **Pre-built binary distribution rejected** for v1.0: maintenance
   burden of per-OS/arch CI pipelines, code signing, and dual-path
   install logic outweighs the 1-2 min install time saved.
9. **HTTP/3 (QUIC) deferred to v1.x**: implementations volatile in
   2026, HTTP/2 covers the practical needs.
10. **Built-in templating with JSX literals deferred** to v1.0+:
    requires amc parser extension, which is a separate work item.
11. **HTTP/2 included in `amalgame-net-http v0.1`** (not a separate
    v0.2): shipping HTTP/1.1-only first would mean half the ecosystem
    uses an incomplete stack. nghttp2 is added upfront; total cost is
    one extra week, outcome is a complete HTTP layer from day one.
12. **Argon2id in `amalgame-crypto`** (not a sister package): consistent
    with existing SHA-256/HMAC surface, no extra `amc package add` step
    for users who need password hashing.
13. **WebSocket server in `amalgame-net-websocket`** (not `amalgame-web`):
    the SHA-1+Base64 handshake and frame encoding are already there;
    adding server-side framing is ~200 lines and achieves client+server
    parity in one package.
14. **Single `app.so` per project** (not one `.so` per route file):
    simpler compilation pipeline, one `dlopen` call, adequate for v0.x.
    Per-file invalidation may be revisited in v1.x if large-project
    compile times become a bottleneck.
15. **Production deployment via `amalgame-service`**: the existing
    `amalgame-service` package already handles Linux (systemd unit) and
    Windows (native SCM via `sc.exe`). No new tooling needed for
    production OS-service integration.
16. **`mosaic supervisor` as v1.0 convenience** (not primary deployment
    tool): systemd and Windows SCM cover serious production cases.
    `mosaic supervisor <dir>` targets dev multi-site and lightweight
    hosting contexts (macOS, VPS without systemd).
17. **Directory-per-site deployment pattern**: each production site lives
    in its own directory with its binary (or `app.so`), `mosaic.toml`,
    and `public/`. Sites run as independent processes, isolated crash
    domains, behind a reverse proxy routing by hostname.

### Decisions taken during implementation (2026-05-19)

18. **`mosaic` is a separate repo, not a sub-package of `amalgame-web`.**
    The build tool's lifecycle (CLI ergonomics, scaffolding,
    dev-server iteration) is fundamentally different from the
    runtime library's (stable API contract, prod-grade fixes).
    Industry precedent matches: SvelteKit lib + `vite`, Rails gem +
    `rails` binary historically. `amalgame-lang/mosaic` ships the
    binary (or scripts, for now); `amalgame-web` stays a pure-AM
    package consumed by `amc package add web`. Split done at
    `mosaic v0.1.0` / `web v0.2.1`.
19. **Filesystem-routing convention** — flat `namespace App` with
    `class Page` collisions resolved by source-rewriting (the
    generator renames each `class Page` to `<PathPrefix>_Page` while
    concatenating into `_routes.am`). Pure dotted-namespace dispatch
    (`App.Routes.Users.Id.Page.GET(ctx)`) is **not supported by amc**
    today, so the per-file-namespace alternative would require an
    amc parser extension. Source-mapping loss (errors point at the
    generated file, not the user's `app/X.am`) is the price; will
    be recovered in phase 3.7-3.9 via dlopen.
20. **WebSocket server lives in `amalgame-net-http`, not `amalgame-web`
    or a new `amalgame-net-websocket`**. The WS upgrade is HTTP/1.1;
    the frame parser is small (~250 LoC including PING/PONG/CLOSE);
    sharing the package keeps one TLS code path for the `wss://`
    variant. `web` exposes WS to user code via
    `router.Ws(path, handler)` in phase 2 (item 2.12).
21. **ACME shipped as a `certbot` subprocess wrapper, not a native
    RFC 8555 implementation**, in `tls v0.2.0`. Rationale: ~200 LoC
    of wrapping over a battle-tested client, production-usable today.
    The native implementation (item 4.1) is ~2-3 days of JWS +
    state machine work, deferred to `tls v0.3.0`. The
    `Acme.EnsureCert(domain, email, dir)` API is stable across the
    swap — only the implementation changes.
22. **HTTPS server advertises ALPN `h2` only** (no HTTP/1.1 fallback)
    in `net-http v0.3.0`. Browsers from 2015+ all speak h2; the
    fallback path lands in phase 4 (item 4.6) when there's user
    pressure for legacy clients. Simplifies the v0.3 implementation
    by ~30%.

## 24. Decisions (formerly open questions — all resolved 2026-05-18)

These were the open architectural questions before implementation started.
For decisions taken during implementation (2026-05-19, e.g., splitting
`mosaic` into its own repo, deferring native ACME), see §23 entries 18+.


1. **HTTP/2 from v0.1** — `amalgame-net-http v0.1` ships HTTP/1.1 +
   HTTP/2 together. No separate v0.2 for HTTP/2.
2. **OpenSSL multi-OS** — detected via `__has_include` + common path
   probing (Homebrew `/opt/homebrew/opt/openssl@3/include` on macOS,
   system path on Linux, msys2/vcpkg on Windows). OpenSSL ≥ 3.0 is
   the floor. `AMALGAME_CFLAGS` env var for non-standard installs.
3. **Argon2id in `amalgame-crypto`** — added directly alongside
   SHA-256/HMAC, no sister package. Precondition for `amalgame-web v0.5`.
4. **WebSocket server in `amalgame-net-websocket`** — extend to v0.2.
   If the server framing turns out complex, fallback to `amalgame-web`.
5. **Single `app.so` per project** — one `.so` for the entire `app/`
   tree. Per-file granularity deferred to v1.x.
6. **Config file format: TOML** — `mosaic.toml`, consistent with
   `amalgame.toml`. No JSON, no YAML.
7. **`mosaic dev --open`** — opt-in flag that opens the browser at
   startup. Disabled by default.

## 25. Appendix: complete example app

A minimal but realistic Mosaic app.

### `amalgame.toml`

```toml
[package]
name              = "todo-app"
version           = "0.1.0"
description       = "A todo list demo built with Mosaic."
required-amalgame = ">=0.9.0"

[dependencies]
amalgame-web              = "^0.5"
amalgame-database-sqlite  = "^0.3"
```

### `mosaic.toml`

```toml
[server]
listen = [":3000"]

[sessions]
backend = "json_file"
dir     = "./data/sessions"
```

### `main.am` (optional — synthesized by mosaic if absent)

```amalgame
namespace App

import Amalgame.Web
import Amalgame.Database.Sqlite
import App.Lib.Db

class Program {
    public static void Main() {
        let db: SqliteConn = Sqlite.Open("./data/todo.db")
        Db.Migrate(db)

        WebApp.New()
            .WithLogging(Logger.New())
            .WithRequestId()
            .WithSessions(JsonFileSessionStore.New("./data/sessions/"))
            .WithCsrf(CsrfConfig.Default())
            .WithSecurityHeaders()
            .WithCompression()
            .WithRateLimit(100, 200)
            .WithState("db", db)
            .Routes("./app")
            .Static("/public", "./public")
            .Listen(":3000", null)
    }
}
```

### `app/index.am`

```amalgame
namespace App.Index

import Amalgame.Web
import Amalgame.Web.Html
import App.Lib.Db

public Function Get(req: HttpRequest, ctx: WebContext): HttpResponse {
    let db: SqliteConn = ctx.State("db")
    let todos: List<Todo> = Db.AllTodos(db)

    let html: HtmlNode = Tag("html", [],
        Tag("body", [],
            Tag("h1", [], Text("Todos")),
            Tag("ul", [],
                todos.Map(fn(t) { return Tag("li", [], Text(t.Title)) })
            ),
            Tag("form", [Attr("method", "POST"), Attr("action", "/todos")],
                Tag("input", [Attr("name", "title")]),
                Tag("button", [], Text("Add"))
            )
        )
    )
    return HttpResponse.New().Html(html.Render())
}
```

### `app/todos.am`

```amalgame
namespace App.Todos

import Amalgame.Web
import App.Lib.Db

public Function Post(req: HttpRequest, ctx: WebContext): HttpResponse {
    let form: Map<string, string> = req.Form()
    let title: string = form.Get("title")
    let db: SqliteConn = ctx.State("db")
    Db.InsertTodo(db, title)
    return HttpResponse.New().Redirect("/", false)
}
```

### `lib/db.am`

```amalgame
namespace App.Lib.Db

import Amalgame.Database.Sqlite

public class Todo {
    public int Id
    public string Title
}

public Function Migrate(db: SqliteConn): void {
    db.Exec("CREATE TABLE IF NOT EXISTS todos (id INTEGER PRIMARY KEY, title TEXT)")
}

public Function AllTodos(db: SqliteConn): List<Todo> {
    return db.Query("SELECT id, title FROM todos ORDER BY id DESC")
              .Map(fn(row) {
                  let t: Todo = new Todo()
                  t.Id = row.GetInt("id")
                  t.Title = row.GetString("title")
                  return t
              })
}

public Function InsertTodo(db: SqliteConn, title: string): void {
    db.Exec("INSERT INTO todos (title) VALUES (?)", [title])
}
```

### Workflow

```bash
mosaic new todo-app           # scaffold
cd todo-app
mosaic dev                    # http://localhost:3000 with hot reload
# ... edit code, refresh browser, repeat ...

# Ship to production:
mosaic build --mono           # produces ./bin/todo-app (single binary, ~10 MB)
scp ./bin/todo-app prod:/opt/todo-app/
ssh prod "/opt/todo-app/todo-app"

# Or modular for self-hosted with partial updates:
mosaic build --modular
rsync -a ./dist/ prod:/opt/todo-app/
ssh prod "cd /opt/todo-app && ./mosaic serve"
```

---

## 26. Acceptance criteria for v1.0

This proposal is considered complete when:

- [ ] `amalgame-tls`, `amalgame-net-http`, `amalgame-web` are all
      published with version ≥ 1.0.
- [ ] `mosaic new`, `mosaic dev`, `mosaic build`, `mosaic serve`
      all work end-to-end on Linux, macOS, Windows.
- [ ] A scaffolded `mosaic new` project ships with sample routes,
      static files, sessions, and CSRF wired in.
- [ ] HTTP/1.1 + HTTP/2 verified with `curl --http2`, `nghttp`,
      Chrome DevTools.
- [ ] TLS verified with `openssl s_client` and a public Qualys SSL
      Labs scan (target: A or A+ grade).
- [ ] ACME flow verified end-to-end against Let's Encrypt staging
      and production.
- [ ] Load test: 10k req/s sustained on a modern 8-core machine,
      static + dynamic mix.
- [ ] Documentation: README, getting-started tutorial, API reference
      generated from doc comments, security best-practices guide,
      deployment guide (Docker, systemd, behind nginx).
- [ ] Mosaic itself is dogfooded: `mosaic` CLI builds successfully
      from source on first install in under 5 min.
- [ ] At least one production user (could be Amalgame's own
      `amalgame-lang.org` website).
