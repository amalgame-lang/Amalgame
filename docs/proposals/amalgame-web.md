# Proposal: `Amalgame.Web` — TLS, HTTP and the Mosaic web server

**Status:** Draft 2026-05-17. Not yet implemented. This document is the
contract against which `amalgame-tls`, `amalgame-net-http` and
`amalgame-web` will be built. It supersedes any prior informal
discussion of HTTP/TLS in Amalgame.

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
- First request to a route → `mosaic` calls `amc compile-shared
  app/<route>.am` → produces `.amweb-cache/routes/<route>.so` (≈100-500
  ms cold compile).
- Subsequent requests → `dlopen` the cached `.so`, invoke handler.
- File change → invalidate cache entry, next request recompiles.
- Pretty error page: stack trace in AM (not C), source snippet, suggested
  fix on common errors.
- Request log to stdout in human-readable format.
- TLS optional (`--tls-cert`/`--tls-key`) or self-signed via
  `--tls-dev` (localhost cert auto-generated).
- ACME **disabled** in dev mode to avoid Let's Encrypt rate limits.

### 5.2 PROD modular — `mosaic serve`

- Reads `dist/` (produced by `mosaic build --modular`):
  ```
  dist/
  ├── mosaic                    # server binary
  ├── routes/
  │   ├── index.so
  │   ├── users__id.so
  │   └── ...
  ├── lib/
  │   └── *.so                  # optional shared libs
  ├── public/                   # assets, served verbatim
  └── mosaic.toml               # server config
  ```
- All `.so` files are pre-compiled by the CI/build pipeline; **no
  runtime compilation**.
- File watcher disabled (security: writing to `dist/routes/*.so` would
  not auto-reload).
- Hot reload available via `SIGHUP` or `mosaic reload` command — swaps
  `.so` atomically by re-`dlopen`ing.
- Use cases:
  - Self-hosted deployments with frequent partial updates
  - Shipping a patch by rsync of a single `.so`
  - Plugin-style architectures (third-party `.so` extensions, opt-in)

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
│   ├─ mosaic dev             → DEV mode (filewatcher + hot dlopen)│
│   ├─ mosaic build           → both PROD artifacts                │
│   │     [--modular | --mono]                                      │
│   ├─ mosaic serve           → PROD modular runtime                │
│   ├─ mosaic reload          → SIGHUP to running mosaic            │
│   ├─ mosaic routes          → list discovered routes              │
│   ├─ mosaic cache clean     → wipe .amweb-cache                   │
│   ├─ mosaic acme renew      → force ACME renew now                │
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
    ├── meta.json                  # source hash → .so path map
    ├── routes/
    │   ├── index.so
    │   ├── users__id.so           # [id].am → users__id.so (slug-safe)
    │   ├── api__login.so
    │   └── ...
    ├── lib/
    │   └── shared.so              # optional, when lib/ is large
    └── tmp/                       # in-flight compilations
```

- Default location: `./.amweb-cache/` in the project root.
- Override: `AMWEB_CACHE_DIR=/var/cache/mosaic/<app>` env var (useful
  for multi-instance deployments where the project dir is read-only).
- `meta.json` stores SHA-256 of each source file **and its transitive
  imports under `lib/`**. A change in `lib/db.am` invalidates all
  routes that import it (not just `lib/db.am`'s direct consumers).
- Compilation is atomic: build into `tmp/`, then `rename()` into
  `routes/`. A partially-written `.so` is never loaded.
- `mosaic cache clean` wipes the cache. `mosaic dev --no-cache`
  forces full recompile.

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

Currently `amalgame-crypto` ships SHA-256 / HMAC-SHA-256. **Argon2id**
should be added to `amalgame-crypto` (or a sister package
`amalgame-crypto-pwhash`) before Mosaic v0.5 ships. SHA-based hashing
is not acceptable for password storage.

Tracked as a precondition; not in this proposal's direct scope.

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

## 17. Configuration

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

## 18. Naming conventions

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

## 19. External dependencies

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

## 20. Roadmap

| Version | Scope | Estimated effort |
|---|---|---|
| `amalgame-tls v0.1` | OpenSSL binding, TlsConfig static (file-based certs), client+server stream, ALPN | 3-4 days |
| `amalgame-net-http v0.1` | HTTP/1.1 parser (pure AM), client+server, body parsing (json/form/multipart), keep-alive | 4-5 days |
| `amalgame-net-http v0.2` | HTTP/2 via nghttp2 binding | 3-4 days |
| `amalgame-web v0.1` | WebApp, programmatic routing, middleware pipeline, static serve, MemorySessionStore | 3 days |
| `amalgame-web v0.2` | Filesystem-based routing + DEV mode (dlopen + filewatcher), pretty errors | 4 days |
| `amalgame-web v0.3` | WebSocket server (extends net-websocket), SSE, channels pub/sub | 2-3 days |
| `amalgame-tls v0.2` | ACME / Let's Encrypt automatic certs (tls-alpn-01 + http-01) | 4-5 days |
| `amalgame-web v0.4` | Security middleware (CSRF, CORS, rate limit, security headers, slow loris), JsonFileSessionStore, /metrics, /healthz | 3 days |
| `amalgame-web v0.5` | PROD modular mode (`.so` deployment), PROD all-in-one mode (`mosaic build --mono`), graceful shutdown | 3-4 days |
| `amalgame-web v0.6` | Reverse proxy (HTTP/1.1+2, load balancing, health checks, circuit breaker) | 3 days |
| `amalgame-web v0.7` | RedisSessionStore, OpenTelemetry-compat request IDs, advanced rate limit (sliding window) | 2 days |
| `amalgame-web v1.0` | Polish, docs, stable API, scaffolder template `amc new --template web-app` | 1 week |

**Total v0.1 → v1.0 ≈ 7-8 weeks** of focused work for the framework
itself. Argon2id in `amalgame-crypto` is a precondition for v0.5 and
must land before then.

**Roadmap dependencies (must land first):**

1. Argon2id in `amalgame-crypto` (precondition for v0.5).
2. WebSocket server-side framing in `amalgame-net-websocket`
   (precondition for `amalgame-web v0.3`).
3. amc support for **package binary delivery** if we ever switch
   away from Option A — not required for v1.0.

## 21. Future work (post-v1.0)

- **JSX-like template literals** in amc parser (separate proposal).
- **Multi-site hosting** in a single Mosaic process (separate proposal
  if demand).
- **Process-isolated workers** (one OS process per route, Phusion
  Passenger model).
- **HTTP/3 (QUIC)** via ngtcp2 or msquic.
- **Built-in caching reverse proxy** (Varnish replacement).
- **WAF rule engine** (ModSecurity-compatible rules).
- **Cluster mode** with shared state via Redis or a coordination
  service.
- **OpenTelemetry full tracing** (spans, exporters).
- **Pre-built `mosaic` binary distribution** if compile time becomes
  prohibitive.
- **DNS-01 ACME challenge** for wildcard certs.
- **Built-in admin UI** (à la Caddy admin API, à la Traefik
  dashboard).

## 22. Decisions log (record of rejected paths)

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

## 23. Open questions (must be answered before code starts)

1. **HTTP/2 from v0.1 or v0.2?** Current plan: v0.2 to ship `amalgame-net-http v0.1`
   faster. Decide whether v0.1 of the framework needs HTTP/2 or can
   ship HTTP/1.1-only first.
2. **OpenSSL version pin?** OpenSSL 3.0 LTS is the floor; some macOS
   Homebrew defaults still ship OpenSSL 1.1.1 in transitive bottle
   chains. Document and require ≥ 3.0.
3. **Argon2id package home?** Add to `amalgame-crypto` directly, or
   spin a sister package `amalgame-crypto-pwhash`? Tendency: into
   `amalgame-crypto` to keep the surface coherent.
4. **WebSocket server in `amalgame-net-websocket` or `amalgame-web`?**
   Cleaner in `amalgame-net-websocket` (parity with client), but
   `amalgame-web` is the only consumer. Tendency: extend
   `amalgame-net-websocket` to v0.2.
5. **Per-route `.so` granularity** — one `.so` per `.am` file is the
   default, but should we offer a "merge mode" (one `.so` per
   directory) for projects with hundreds of small route files?
   Defer to v1.x measurement.
6. **Config file format**: TOML for `mosaic.toml` (consistent with
   `amalgame.toml`). Confirmed — no JSON or YAML.
7. **Should `mosaic dev` open the browser automatically?** Nice
   touch (Vite does this). Behind a flag `--open`? Tendency: yes,
   `--open` flag opt-in.

## 24. Appendix: complete example app

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

## 25. Acceptance criteria for v1.0

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
