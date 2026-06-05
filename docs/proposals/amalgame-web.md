# Proposal: `Amalgame.Web` — TLS, HTTP and the Mosaic web server

**Status:** Foundation shipped 2026-05-18 onward; the stack is now
**production-credible for single-node apps**. Most of Phase 1
(hardening) and Phase 2 (real-world apps) is done. Current versions
(**2026-06-04**): `amalgame-tls` v0.3.3, `amalgame-net-http` v0.20.0,
`amalgame-web` v0.32.0, `mosaic` v0.7.0, plus shipped preconditions
`amalgame-threading` v0.1.0, `amalgame-async` v0.3.0, `amalgame-crypto`
v0.4.0, `amalgame-net-proxy` v0.2.1, `amalgame-net-smtp` v0.2.4.

> ⚠️ **Shipped vs. roadmap — read before quoting this in marketing.**
> Verified against the repos on **2026-06-04**.
> **Shipped:** Router, Sessions (memory / signed+encrypted cookie /
> Redis), security middlewares (CSP/HSTS/CORS/CSRF/rate-limit/request-id),
> anti-injection guards (CRLF / open-redirect / SSRF), configurable
> limits, slowloris timeouts, HTTP/1.1 keep-alive, concurrency
> (`Http1.ServeMt` threads + `Http1.ServeAsync` fibers), graceful
> shutdown, static serving (ETag / Last-Modified / Range / `.gz` /
> gzip), SSE, WebSocket (text + subprotocols), multipart, template
> engine with auto-escape + context filters, Basic + JWT auth, OAuth2
> (GitHub/Google), reverse proxy + load-balancing (`amalgame-net-proxy`),
> native pure-AM ACME client (**http-01 only**), auto-renew thread,
> single-binary builds.
> **NOT yet shipped / partial (do not claim as fact):**
> - tls-alpn-01 and dns-01 ACME challenges (http-01 only). Other CAs
>   than Let's Encrypt not supported (no `Acme.WithDirectory`).
> - Native TLS termination **fully wired to the Mosaic router in
>   production**. The reference live demo (`amalgame-live`) was fronted
>   by an external Node/greenlock proxy — migrating the production
>   sites to Mosaic-native HTTPS is the open Phase-3 ACME item
>   (deadline ~2026-07-27, see [[roadmap-acme-autorenew-timer]]).
> - Argon2id password hashing (crypto v0.4.0 ships **scrypt** + ES256/
>   RS256/AES-GCM/HMAC-SHA256/CSPRNG, not Argon2id, Ed25519, HS384/512).
> - Phase 2 auth/data depth shipped 2026-06-05: TOTP/2FA ✅ crypto
>   v0.5.0; PKCE ✅ web v0.34.0; OIDC ✅ web v0.35.0; typed input
>   validation ✅ web v0.36.0; DB migrations ✅ database-migrate v0.1.0;
>   WebAuthn/passkeys ✅ amalgame-webauthn v0.1.0.
> - `router.Ws()` (WebSocket still side-bound via `Ws.Serve`, not a
>   first-class route), WebSocket binary frames + fragmentation.
> - `/metrics`, OpenTelemetry, `/healthz` middleware, hot-reload,
>   source mapping, 100%-Amalgame `mosaic` CLI — all roadmap.

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

> 📌 **Update 2026-06-04:** the "no async/await" premise below is now
> partly outdated. amc gained `async`/`await` sugar (v0.8.70) and
> `amalgame-async` v0.3.0 ships a Fiber/Channel/Scheduler runtime on
> epoll. The server therefore offers **both** paths: `Http1.ServeMt`
> (thread pool, this section) **and** `Http1.ServeAsync` (fiber/event-
> loop, net-http v0.9.1). The thread-pool model below remains the
> default and the rationale for keeping it (simplicity, multi-core,
> blocking-handler tolerance) still holds; the fiber path is the
> high-connection-count alternative.

**Why thread pool remains the default (vs. a pure event loop):**

- The async path exists but a pure event-loop-only design would force
  "function colors" through every handler; the thread pool keeps
  ordinary blocking handlers simple.
- A single CPU core would be used; multi-core requires cluster/worker
  pattern → complication leaking through the framework API.
- A blocking handler (heavy DB query, file I/O) stalls a pure event
  loop; the thread pool tolerates it.
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

`amalgame-crypto` **shipped** (v0.4.0): SHA-256, HMAC-SHA-256,
AES-256-GCM, ES256 (P-256 ECDSA), RS256 (RSA-2048), CSPRNG, and
**scrypt** password hashing (constant-time verify). scrypt is an
acceptable memory-hard KDF for password storage; **Argon2id is still
the preferred target** and remains a TODO (deferred pending an
OpenSSL 3.2+ `EVP_KDF` or `libargon2` backend) — see §21 item 2.1.
Also not yet shipped: Ed25519, HMAC-HS384/HS512, ECDSA P-384/P-521.

The auth flows (Basic + JWT-HS256, §21 item 2.2) and the native ACME
implementation (item 4.1) are **unblocked and shipped** on top of this
surface.

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

> ✅ **Shipped** as `amalgame-net-proxy` v0.2.1 (a sibling package,
> not folded into `amalgame-web`), wired into Mosaic via
> `mosaic_server.am` (`AddHandler()` plugs a proxy/LB in as a site).
> **Done:** longest-prefix path routing, load balancing (round-robin,
> IP-hash sticky, least-connections), `X-Forwarded-For` injection,
> hop-by-hop header stripping. **TODO:** active health checks, circuit
> breaker, caching reverse proxy. The API below (`app.Proxy` /
> `app.ProxyPool`) is the intended ergonomic; current surface is the
> `amalgame-net-proxy` `UpstreamPool` + `AddHandler` form.

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

> ✅ **Shipped** as a native pure-AM RFC 8555 client in `amalgame-tls`
> v0.3.0+ (`acme.am` / `AcmeNative`): JWS account signing, order/auth/
> finalize state machine, CSR DER, cert-chain parsing, plus
> `CertDaysRemaining` / `NeedsRenewal` helpers (v0.3.3) and a background
> auto-renew thread. **Reality check on the sketch below:** only the
> **http-01** challenge is implemented — the tls-alpn-01 "preferred,
> in-process" flow described next is **not** shipped, and dns-01 is
> out of scope. Only Let's Encrypt is supported (no `Acme.WithDirectory`
> for other CAs yet).

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
- **http-01** — ✅ **the only one shipped** (requires :80 access). The
  native client serves the challenge token over HTTP.
- **tls-alpn-01** — roadmap (would remove the :80 requirement).
- **dns-01** — roadmap item 4.3 (requires DNS API integration; needed
  for wildcards).

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

> 📌 The table below is the **2026-05-18..19 origin sprint** (historical).
> For the **current per-item status of all remaining work as verified
> against the repos on 2026-06-04**, see the Status columns in §21.2.
> Headline: Phase 1 is ~done, Phase 2 mostly done, Phases 3–4 are the
> bulk of what's left.

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

#### Phase 1 — Production hardening (~2-3 weeks) — ✅ DONE

Blocking for any real prod use. Without these, security audits
fail and throughput is misery. **10/10 shipped** (IPv6 dual-stack
listener landed in net-http v0.21.0).

| # | Item | Status | Evidence (2026-06-04) |
|---|---|---|---|
| 1.1 | **Security pack** in `amalgame-web` — `WithSecurityHeaders` / `WithCors` / `WithCsrf` / `WithRateLimit` / `WithRequestId` middlewares (see §11) | ✅ shipped | `security_headers.am` + `cors.am` + `csrf.am` + `rate_limit.am` (web v0.4.0→v0.7.0) |
| 1.2 | **Fix command injection** in `tls`'s `Acme.EnsureCert` — replace `system(3)` with `execve(argv)` | ✅ moot/shipped | ACME is now a native pure-AM client (`acme.am`), no shell subprocess at all |
| 1.3 | **Anti-injection guards** — CRLF in `HttpResponse.Header()`, open-redirect in `Redirect()`, SSRF in `HttpClient.Get()` (refuse RFC1918 by default) | ✅ shipped | `HttpResponse.Header()` CR/LF check, `RedirectLocal()`, `HttpClient.GetGuarded()` + `HostIsPublic()` (net-http v0.19.0) |
| 1.4 | **Slowloris timeouts** — `SO_RCVTIMEO` / `SO_SNDTIMEO`; per-connection idle timeout | ✅ shipped | `Amalgame_Net_Http.h` sets SO_RCVTIMEO/SNDTIMEO per header/body timeout |
| 1.5 | **Configurable limits** — body / header / URL length tunable | ✅ shipped | `HttpServerConfig.WithMaxBodyBytes/WithMaxHeaderBytes/WithMaxUrlBytes` |
| 1.6 | **`amalgame-threading`** package — mutex, cond var, thread create/join | ✅ shipped | v0.1.0 — Mutex, Channel, Thread (Spawn/Join/Sleep) |
| 1.7 | **Worker pool** — N concurrent connections instead of 1 | ✅ shipped | `Http1.ServeMt` (threads, net-http v0.6.0) + `Http1.ServeAsync` (fibers, v0.9.1, on `amalgame-async` v0.3.0) |
| 1.8 | **HTTP/1.1 keep-alive** in `Http1.Serve` | ✅ shipped | `keep_alive` + `ResetForReuse()` in H1Conn (net-http v0.5.0+) |
| 1.9 | **IPv6 (dual-stack)** — bind `AF_INET6` with `IPV6_V6ONLY=0` everywhere | ✅ shipped | net-http v0.21.0: H1 + HTTPS-H1 listeners (incl. async) bind dual-stack, AF_INET fallback; v4-mapped peers normalized to plain IPv4 (RemoteAddr semantics preserved). Live audit: `::1` accepted + `127.0.0.1` normalized. WS/H2 listeners still IPv4 (tracked) |
| 1.10 | **Graceful shutdown** — SIGTERM drains in-flight requests | ✅ shipped | `Http1_InstallShutdownSignals()` + `Http1_IsStopping()` + per-conn fiber drain (net-http v0.8.0+, v0.13.3 handshake fix) |

#### Phase 2 — Real-world apps (~3-4 weeks) — 🟢 mostly DONE

Past phase 1, the stack can serve traffic safely. Phase 2 turns it
into something that runs real apps. **Most shipped; still open:
`router.Ws`.** (TOTP/2FA, OAuth PKCE + OIDC, typed input validation, DB
migrations, and WebAuthn/passkeys all landed 2026-06-05.)

| # | Item | Status | Evidence (2026-06-04) |
|---|---|---|---|
| 2.1 | **`amalgame-crypto`** — Argon2id, HMAC (HS256/384/512), RSA, ECDSA, Ed25519, CSPRNG | 🟡 partial | v0.4.0 ships SHA-256, HMAC-SHA-256, AES-256-GCM, ES256, RS256, **scrypt** (password hashing), CSPRNG. **Missing:** Argon2id, Ed25519, HS384/512, ECDSA P-384/P-521 |
| 2.2 | **Auth middlewares** — Basic + Bearer/JWT + API key + session login | ✅ shipped | `basic_auth.am` (RFC 7617, web v0.15.0) + `jwt_auth.am` (HS256, v0.16.0) + route `.Protected()` gate. (folded into `amalgame-web`, no separate `-auth` pkg) |
| 2.3 | **OAuth 2.0 / OIDC client** — Google / GitHub / etc. SSO | ✅ shipped | OAuth2 auth-code client + GitHub/Google presets (`oauth2.am`, web v0.17.0); **PKCE** (RFC 7636 S256) via `WithPkce()` (web v0.34.0); **OpenID Connect** via `WithOidc(issuer)` (web v0.35.0) — HandleCallback verifies the id_token: RS256 sig vs the IdP's JWKS (pinned `jwks_uri` or discovery), alg hard-pinned to RS256 (rejects `none`/`HS*` → no algorithm-confusion), `iss`/`aud`/`exp` + one-time `nonce` (replay), fails closed, verify-only JWKS key (crypto v0.6.0 `JwsKey.FromJwkRsa`). Validated by a self-signed round-trip + negative tests |
| 2.4 | **WebAuthn / passkeys** — passwordless auth | ✅ shipped | `amalgame-webauthn` v0.1.0 (`WebAuthn` RP + `CborReader`/`CborValue` + result types) — server-side registration (attestationObject CBOR + clientDataJSON → extract COSE EC2 key + credId) and authentication (assertion **signature verification** over authData‖SHA-256(clientData) + strictly-increasing counter). Built on crypto v0.7.0 `FromCoseEc2`/`VerifyDer`. Scope: ES256 + attestation `none` (cert-chain attestation = future). 8 tests incl. a real ES256-signed assertion end-to-end |
| 2.5 | **TOTP / 2FA** — RFC 6238 | ✅ shipped | `amalgame-crypto` v0.5.0: `Totp.At/Now/Verify` (HMAC-SHA-1 + Base32), constant-time compare + skew window, validated vs the RFC 6238 vectors |
| 2.6 | **`amalgame-database-sqlite`** — prepared stmts + transactions | ✅ shipped | v0.4.0 (ExecBind/QueryBindAll `?`, Begin/Commit/Rollback) |
| 2.7 | **`amalgame-database-postgresql`** (libpq) — + mssql/mysql/oracle/duckdb/mongodb/redis | ✅ shipped | postgresql v0.3.0 (`$1` PQexecParams, transactions); siblings shipped |
| 2.8 | **Migration framework** — schema versioning + apply/rollback | ✅ shipped | `amalgame-database-migrate` v0.1.0 (`Migrator`/`Migration`/`MigrateResult`) — ordered `Up`/`Down`/`DownTo` against `amalgame-database-sqlite`, tracked in `schema_migrations`. Safety: ascending order, each step in its own transaction with **fail-stop rollback** (no half-applied drift), idempotent Up, **irreversible-step refusal**, duplicate-version rejection, identifier-checked table name. SQLite-only for now (PG sibling = future) |
| 2.9 | **HTML template engine with auto-escape** | ✅ shipped | `template.am` (web v0.24.0; `{{x}}` escaped, `{{{x}}}` raw) + context-aware filters `\|js \|attr \|url` (v0.29.0) + `WebContext.Render`/`RenderString`. Pure-AM, no FFI |
| 2.10 | **Typed input validation** — request-field schemas | ✅ shipped | `Validator` / `FieldRule` / `ValidationResult` (`validation.am`, web v0.36.0) — Required/Optional, Int/Float/Bool/Email (strict), Range, Min/MaxLen, OneOf allow-lists, Alnum/Alpha/Numeric, AllowMultiline; `v.Check(map)` → typed accessors + `ErrorJson()` 422. Security: 8 KiB default length cap (anti-DoS), control-char rejection (anti header/log injection + NUL), strict typing, allow-lists. Folded into `amalgame-web` (no separate `-validation` pkg, like auth) |
| 2.11 | **Session stores** beyond memory | ✅ shipped (variant) | `SignedCookieSessionStore` (signed+encrypted cookie, web v0.8.5) + `RedisSessionStore` (v0.8.4). **NB:** shipped a signed-cookie store instead of the planned `JsonFileSessionStore` — see [[project-sessions-signed-cookie]] |
| 2.12 | **`router.Ws(path, handler)`** — WS routes through the Router | ⬜ not started | WS still side-bound via `Ws.Serve(port, handler)`; only `.Sse(path, handler)` is a first-class WebApp route |
| 2.13 | **Multipart parser** | ✅ shipped | net-http v0.14.1 `Multipart`/`UploadedFile` + web v0.25.0 `ctx.Multipart()` / `mp.File(...)` + `SafeFilename`/`SaveToDir` (v0.19.0) |

#### Phase 3 — UX + ops (~1-2 weeks) — 🟢 observability done; DX remains

Quality of life. Better dev experience, smoother prod deploys.
**Observability shipped in web v0.33.0 (`/metrics` private-by-default +
`/healthz`/`/readyz` + JSON access logs); OpenTelemetry (3.3) is the
only observability item left. The remaining cluster is the all-Amalgame
`mosaic` toolchain (3.7–3.10).**

| # | Item | Status | Evidence (2026-06-04) |
|---|---|---|---|
| 3.1 | **Structured access logs** — JSON or Common Log Format | ✅ shipped | `LogConfig.WithFormat("json")` → one JSON object per line via `Log.Raw` (logging v0.2.0), fields JSON-escaped (web v0.33.0). Text remains default |
| 3.2 | **`/metrics` endpoint** — Prometheus exposition format | ✅ shipped | `WithObservability` → `/metrics` (requests by status class, total, in-flight gauge, duration sum), counters mutex-guarded for ServeMt (web v0.33.0). **Private-by-default** (loopback/RFC1918 or bearer token; else 404) |
| 3.3 | **OpenTelemetry tracing** — request spans through middleware | ⬜ not started | no `opentelemetry`/`tracing` symbols — the remaining Phase-3 observability item |
| 3.4 | **`/healthz` + `/readyz`** middleware | ✅ shipped | built-in via `WithObservability` — public liveness/readiness 200s (web v0.33.0) |
| 3.5 | **systemd `Type=notify` + watchdog** integration | 🟡 partial | `mosaic-service.sh` emits `Type=simple`; no `notify`/`WatchdogSec` |
| 3.6 | **`mosaic new <template>`** — scaffolding | ✅ shipped | `mosaic new` → `mosaic-new.sh` (mosaic v0.7.0) |
| 3.7 | **dlopen hot-swap** in `mosaic dev` — in-flight WS survive reload | ⬜ not started | `docs/proposals/hot-reload.md` chose SO_REUSEPORT worker-swap (dlopen rejected: amc lacks `--shared` + GC complexity); **proposal only** |
| 3.8 | **Native `mosaic` binary** — bash scripts → `src/*.am` | 🟡 partial | `mosaic-serve` is pure AM (`src/mosaic-serve.am`); the `mosaic` dispatcher + build/dev/routes/new are still bash |
| 3.9 | **Source mapping** — amc errors point at `app/X.am`, not `_routes.am` | ⬜ not started | listed as "future" in README |
| 3.10 | **`mosaic test`** — integration test harness against a running app | 🟡 partial | internal `tests/run_tests.sh` smoke suite exists; no user-facing `mosaic test` command |
| 3.11 | **Docker / systemd templates** — drop-in deploy recipes | 🟡 partial | systemd unit gen in `mosaic-service.sh`; no Docker templates |

#### Phase 4 — Polish + breadth (~2-3 weeks) — 🟠 partially done

Past phase 3, Mosaic is feature-complete for serious deployments.
Phase 4 fills in the long tail. **The TLS/HTTP/WS/static long tail is
largely shipped; the missing pieces are mostly new sibling packages
(queue/cron/i18n/openapi/WAF) and ACME breadth (other CAs, dns-01).**

| # | Item | Status | Evidence (2026-06-04) |
|---|---|---|---|
| 4.1 | **Native pure-AM ACME** (RFC 8555) — replaces certbot wrapper | ✅ shipped | `acme.am` `AcmeNative` (tls v0.3.0): JWS signing, order/auth/finalize, CSR DER, cert chain. **http-01 only** |
| 4.2 | **`Acme.WithDirectory(url)`** — other CAs (Buypass, ZeroSSL, Google) | ⬜ not started | only `LeProd()`/`LeStaging()` hardcoded |
| 4.3 | **DNS-01 challenges + DNS provider bindings** — wildcards | ⬜ not started | acme.am: "http-01 only" |
| 4.4 | **mTLS client cert auth** in `TlsConfig` + `WebContext.ClientCert` | 🟡 partial | `TlsConfig.WithClientAuth` + `TlsStream.PeerCertSubject` exist; **no `WebContext.ClientCert` wrapper** |
| 4.5 | **OCSP stapling** | ⬜ not started | no symbols in tls/net-http |
| 4.6 | **HTTP/1.1 over TLS** — browser-facing HTTPS without forcing h2 | ✅ shipped | `HttpsH1Server` advertises ALPN `http/1.1` (net-http v0.10) + SNI `AddSni` (multi-domain, see [[reference-nethttp-sni]]) |
| 4.7 | **Static file middleware** — MIME + `Cache-Control` + `ETag`/`If-None-Match` + `Last-Modified`/`If-Modified-Since` + `.gz` | ✅ shipped | `static.am` (web v0.13.0→v0.27.0). **`sendfile(2)` still TODO** |
| 4.8 | **gzip compression** middleware | ✅ shipped | `compress.am` (web v0.26.0, via amalgame-compress). **Brotli still TODO** |
| 4.9 | **Range requests (206 Partial Content)** | ✅ shipped | net-http v0.15.0 `FileRange` + web v0.27.0 (single range; 416 on unsatisfiable). **Multi-range TODO** |
| 4.10 | **Server-Sent Events (SSE)** | ✅ shipped | net-http v0.16.0 `SseConn` + web v0.28.0 `WebApp.Sse` |
| 4.11 | **WebSocket binary frames** — `WsConn.SendBinary`/`ReceiveBinary` | ⬜ not started | only `SendText`/`ReceiveText` exposed (binary opcode exists in runtime, no public API) |
| 4.12 | **WebSocket fragmentation** — multi-frame messages | ⬜ not started | no FIN/continuation API |
| 4.13 | **WebSocket subprotocol negotiation** | ✅ shipped | net-http v0.17.0 `Ws.ServeWithProtocols` + `WsConn.Subprotocol` |
| 4.14 | **HTTP/3 / QUIC** | ⬜ future | zero references (intentionally deferred) |
| 4.15 | **SMTP client (transactional mail)** | ✅ shipped | `amalgame-net-smtp` v0.2.4 (`Smtp` + `Mail` builder, RFC 2047 subjects) — see [[project-amalgame-net-smtp]] |
| 4.16 | **`amalgame-queue`** — background jobs over DB / Redis | ⬜ not started | no repo |
| 4.17 | **`amalgame-cron`** — scheduled tasks | ⬜ not started | no repo |
| 4.18 | **`amalgame-i18n`** — locale-aware responses, Accept-Language | ⬜ not started | no repo |
| 4.19 | **`amalgame-openapi`** — generate `swagger.json` from routes | ⬜ not started | no repo |
| 4.20 | **`Closure<A, R>` typed closures** in amc — removes `-Wno-int-conversion` | ✅ shipped | amc v0.8.35+; used throughout `facade.am` router/auth (`Closure<WebContext, HttpResponse>`) |
| 4.21 | **WAF rule engine** — anti-bot, SQL/XSS pattern detection | ⬜ not started | no repo |

> **Reverse proxy / load balancing** (§14) — ✅ shipped as
> `amalgame-net-proxy` v0.2.1: path routing + round-robin / IP-hash /
> least-connections + XFF + hop-by-hop stripping, wired into Mosaic via
> `mosaic_server.am`. TODO: active health checks, circuit breaker,
> caching proxy.

### 21.3 Critical preconditions — ✅ all shipped

The three packages that underpinned most of phases 1-4 are **all
shipped** (as of 2026-06-04):

1. **`amalgame-threading`** v0.1.0 (1.6) — ✅ + `amalgame-async` v0.3.0
   gives a fiber path too; servers handle N concurrent connections.
2. **`amalgame-crypto`** v0.4.0 (2.1) — ✅ enables auth, JWT, session
   signing, native ACME. (Argon2id specifically still TODO; scrypt
   covers password hashing in the meantime.)
3. **HTML template engine** (2.9) — ✅ shipped *inside* `amalgame-web`
   v0.24.0 (not a separate `amalgame-template` package), auto-escape
   by default.

These three are no longer blockers; the remaining work is breadth.

### 21.4 What's actually left (as of 2026-06-04)

The original "first ~25 days" order is complete: routing, auth (Basic +
JWT), sessions, DB, templates, security middleware, HTTPS auto-renew,
native ACME, concurrency — all shipped. The stack is **prod-ready for
single-node apps today**. Remaining work, in suggested priority order:

1. ~~**1.9** — IPv6 dual-stack listener binding~~ ✅ **shipped
   net-http v0.21.0** (H1 + HTTPS dual-stack; Phase 1 now complete).
2. ~~**3.2 + 3.4** — `/metrics` + `/healthz`/`/readyz` middleware~~ ✅
   **shipped web v0.33.0** (metrics private-by-default).
3. ~~**3.1** — JSON access logs~~ ✅ **shipped web v0.33.0**
   (`WithFormat("json")` + `Log.Raw`).
4. ~~**ACME Phase-3 production cutover** — migrate the live sites off the
   external Node/greenlock proxy onto Mosaic-native HTTPS.~~ ✅ **done**
   (verified live 2026-06-05): `~/Développement/Sites` `server.am`
   multi-site front holds 80/443 — amalgame.me/belfort/musicall/docs/demo
   all answer `Server: Mosaic (Amalgame)` over HTTPS with valid LE **prod**
   certs (SNI per domain, native ACME, 80→443 redirect). Node is no longer
   the front. ~~Minor residue: HEAD returns 404 (cosmetic; GET 200)~~ ✅
   fixed (web v0.36.1 — HEAD falls back to the GET route + strips the
   body, RFC 7231). Remaining: the dormant Node/pm2 fallback can be
   decommissioned. See [[roadmap-acme-autorenew-timer]].
5. ~~**2.3** — finish OAuth2 (PKCE + OIDC id_token verification).~~ ✅
   done (PKCE web v0.34.0, OIDC web v0.35.0).
6. ~~**2.8** — DB migrations.~~ ✅ done (`amalgame-database-migrate`
   v0.1.0). (~~2.10 input validation~~ ✅ web v0.36.0.)
7. **2.12** — `router.Ws()` first-class WebSocket routes.
8. **2.1 (Argon2id), 2.4, 2.5** — Argon2id, WebAuthn, TOTP (auth depth).
9. **3.7 / 3.8 / 3.9** — hot-reload, all-Amalgame `mosaic` toolchain,
   source mapping (DX).
10. **4.x breadth** — Brotli, `sendfile`, multi-range, WS binary/
    fragmentation, mTLS `WebContext.ClientCert`, OCSP, dns-01 + other
    CAs, and the new sibling packages (queue, cron, i18n, openapi, WAF).

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

This proposal is considered complete when (all gates target **v1.0**;
the packages are still on 0.x, so these stay unchecked even where the
underlying capability is demonstrable today — progress noted inline):

- [ ] `amalgame-tls`, `amalgame-net-http`, `amalgame-web` are all
      published with version ≥ 1.0. *(currently tls 0.3.3 / net-http
      0.20.0 / web 0.32.0 — API broad, not yet stabilized to 1.0.)*
- [ ] `mosaic new`, `mosaic dev`, `mosaic build`, `mosaic serve`
      all work end-to-end on Linux, macOS, Windows.
- [ ] A scaffolded `mosaic new` project ships with sample routes,
      static files, sessions, and CSRF wired in.
- [ ] HTTP/1.1 + HTTP/2 verified with `curl --http2`, `nghttp`,
      Chrome DevTools. *(H1/H2/HTTPS+ALPN verified on the fs-demo in
      the origin sprint; Qualys-grade cert chain still to confirm.)*
- [ ] TLS verified with `openssl s_client` and a public Qualys SSL
      Labs scan (target: A or A+ grade).
- [ ] ACME flow verified end-to-end against Let's Encrypt staging
      and production. *(native http-01 client shipped + auto-renew
      thread; production cutover of the live sites is the open
      Phase-3 item, deadline ~2026-07-27.)*
- [ ] Load test: 10k req/s sustained on a modern 8-core machine,
      static + dynamic mix.
- [ ] Documentation: README, getting-started tutorial, API reference
      generated from doc comments, security best-practices guide,
      deployment guide (Docker, systemd, behind nginx).
- [ ] Mosaic itself is dogfooded: `mosaic` CLI builds successfully
      from source on first install in under 5 min.
- [ ] At least one production user (could be Amalgame's own
      `amalgame-lang.org` website).
