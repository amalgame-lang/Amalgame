# Configuration

Canonical reference for every knob in the Mosaic stack: TOML key,
default, env-var override, and the library builder it maps to. See also
[Installation](02-installation.md) and the [framework tour](01-framework.md).

This doc is the **contract**. If a feature lands without a matching
section here, it's not finished. Source of truth for the build tool
(`mosaic` CLI) when it reads `mosaic.toml`, sets up env-var overrides,
and instantiates library builders.

For the design rationale, see [`docs/proposals/amalgame-web.md` §18](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-web.md).

---

## 1. The override cascade

There are not three but **six** layers, lowest-priority first:

```
1. Library default   (the class's constructor)
2. mosaic.toml       (CLI flattens it and calls FromMap on each feature)
3. env vars          (MOSAIC_<SECTION>_<KEY> — applied by CLI, or read
                      directly by the lib for runtime knobs like
                      MOSAIC_TLS_ACME_SERVER)
4. CLI flags         (mosaic dev --listen :8080)
5. App code          (builder chained in main.am AFTER the CLI hands
                      back the pre-configured instance)
6. Per-route handler (`resp.Header(...)` in the route closure — Apply
                      never overwrites a header the handler already set)
```

**"Code" is not one layer but two**: the library default lives at the
bottom (it's what the constructor produces), your `main.am` chaining
lives in the middle, and your route handler lives at the top. They
look the same to a reader but sit at very different positions in the
cascade.

**Handler-wins is intentional**: a single exceptional route
(e.g. `/embed` that must allow an `<iframe>`) can override a global
policy without disabling the policy elsewhere. The cautious default
stays cautious everywhere else.

### Two scenarios

**With the Mosaic CLI:** TOML / env / flags are layered by the CLI,
which calls each feature's `FromMap(...)` and yields the pre-configured
instance to your `main.am`. Your code can chain further `With*(...)`
calls — those overwrite the CLI-applied values, which is exactly what
you want (code is closer to the routes than the config file).

**Without the Mosaic CLI:** you build instances directly in `main.am`
with the builders. TOML / env / flags don't apply. The library has
zero dependency on TOML or any CLI.

### One feature = one instance

Per WebApp, you should have exactly one `SecurityHeaders` (and one
`Cors`, one `Csrf`, one `RateLimit`, …). Multiple instances are a
source of silent contradictions. Sub-apps mounted on different paths
are the only legitimate case for more than one.

## 2. File location

```
my-app/
├── amalgame.toml          # package manifest
├── mosaic.toml            # ← this file
├── app/                   # routes
└── main.am
```

A missing `mosaic.toml` is equivalent to an empty one — every
feature falls back to its library default.

## 3. Reference

Each section below maps a TOML table to the library/middleware that
consumes it. The legend:

- **Lib** — the AM class + static factory the Mosaic CLI calls.
- **Status** — *shipped* / *planned*.

---

### `[server]` — listener + worker pool

**Lib:** `Amalgame.Net.Http.Http1.Serve` / `Https.Serve` / `Http2.Serve` (selected via `[tls].mode`).
**Status:** *partial* — workers and queue_size not yet honored (single-conn loop).

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `listen` | `[string]` | `[":3000"]` | `MOSAIC_SERVER_LISTEN` | Bind addresses. Multiple = multi-port. |
| `workers` | `int` | `0` (auto = 2 × CPUs) | `MOSAIC_SERVER_WORKERS` | Worker pool size. *planned (needs amalgame-threading)* |
| `queue_size` | `int` | `0` (auto = workers × 4) | `MOSAIC_SERVER_QUEUE_SIZE` | Backlog. *planned* |
| `engine` | `string` | `"mt"` | `MOSAIC_SERVER_ENGINE` | `"mt"` = `WebApp.ServeMtWith` (default, cross-platform), `"async"` = `WebApp.ServeAsyncWith` (Linux only, fiber-per-conn via amalgame-async ≥ v0.2.2). *planned wiring — pre-v0.13.0 apps must call `ServeAsyncWith` directly until the config key lands.* See [docs/proposals/amalgame-async.md](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-async.md) for the trade-offs (1.5×–9× throughput on I/O-bound handlers, handles 2k concurrent connections where mt collapses at 1k–2k on 2-core boxes) |

---

### `[tls]` — TLS / HTTPS / ACME

**Lib:** `Amalgame.Tls.TlsConfig` + `Amalgame.Tls.Acme` (runtime), `Amalgame.Web.AcmeConfig` (Mosaic-side TOML wiring, v0.10.0).
**Status:** *partial* — `acme` mode shipped via amalgame-tls v0.2.2 (certbot wrapper) + amalgame-web v0.10.0 (`AcmeConfig.FromMap`); `files` and `off` work. Multi-domain SAN provisioning still uses one domain at a time (single-call EnsureCert per host).

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `mode` | `"off"\|"files"\|"acme"` | `"off"` | `MOSAIC_TLS_MODE` | Top-level switch. |
| `acme_email` | `string` | — | `MOSAIC_TLS_ACME_EMAIL` | Required when mode = `"acme"`. *shipped v0.10.0* (maps to `AcmeConfig.Email`). |
| `acme_cache` | `string` | `"./certs"` | `MOSAIC_TLS_ACME_CACHE` | Cert/account dir (maps to `AcmeConfig.CertDir`). *shipped v0.10.0*. |
| `domains` | `string` | `""` | `MOSAIC_TLS_DOMAINS` | Comma-separated SAN list — first entry becomes the cert-name. *shipped v0.11.1* via `AcmeConfig.Domains` + `Acme.EnsureCertMulti` (amalgame-tls v0.2.3). Up to 32 SANs per cert. |
| `acme_server` | `string` | LE production | `MOSAIC_TLS_ACME_SERVER` | ACME directory URL. *shipped v0.10.0* (maps to `AcmeConfig.AcmeServer`; env var honored by `Acme.EnsureCertEx`). Buypass / ZeroSSL / LE-staging by passing the directory URL. |
| `certbot_path` | `string` | `"certbot"` (looked up via `PATH`) | `MOSAIC_TLS_CERTBOT_PATH` | Absolute path override. *shipped v0.10.0* (maps to `AcmeConfig.CertbotPath`). |
| `cert_file` | `string` | — | `MOSAIC_TLS_CERT_FILE` | Required when mode = `"files"`. |
| `key_file` | `string` | — | `MOSAIC_TLS_KEY_FILE` | Required when mode = `"files"`. |
| `min_version` | `"1.2"\|"1.3"` | `"1.3"` | `MOSAIC_TLS_MIN_VERSION` | Maps to `HttpServerConfig.WithTlsMinVersion` (12/13). *shipped v0.11.2* via `Amalgame.Web.TlsBindingConfig.FromMap` + amalgame-net-http v0.7.1; honored end-to-end at `SSL_CTX_set_min_proto_version`. |
| `alpn` | `string` | `"h2,http/1.1"` | `MOSAIC_TLS_ALPN` | ALPN list (comma-separated). *partial v0.11.2* — parsed and forwarded to `HttpServerConfig.WithTlsAlpn`, but net-http's ALPN select callback still hardcodes `h2` only (one-line stderr warning on mismatch). Full http/1.1 fallback lands in amalgame-net-http v0.7.2. |

**Pattern d'usage (mode = "acme"):**

```amalgame
import Amalgame.Web         // AcmeConfig
import Amalgame.Tls         // Acme.EnsureCertEx
import Amalgame.Net.Http    // Https.Serve

let acme = AcmeConfig.FromMap(tomlAcmeSection)
let err: string = acme.Validate()
if (String_Length(err) > 0) { Console.WriteError(err); return }

if (acme.Enabled) {
    let rc: int = Acme.EnsureCertEx(
        acme.Domain, acme.Email, acme.CertDir,
        acme.AcmeServer, acme.CertbotPath)
    if (rc != 0) { Console.WriteError("ACME failed"); return }
}
Https.Serve(443, acme.CertPath(), acme.KeyPath(), handler)
```

L'appel `Acme.EnsureCertEx` reste côté user pour qu'il puisse séquencer le provisionnement avec son propre startup (DB / migrations / etc.) avant de bind port 443. v0.3 d'amalgame-tls swap l'implem en ACME natif (RFC 8555) sans changer l'API d'`AcmeConfig`.

---

### `[sessions]` — server-side session storage

**Lib:** `Amalgame.Web.MemorySessionStore` / `SignedCookieSessionStore` / `RedisSessionStore` (the last uses amalgame-database-nosql-redis) — `shm` backend planned (needs amalgame-threading).
**Status:** *shipped* — `memory` (v0.8.1) + `signed_cookie` strategy (v0.8.3) + `redis` backend all ship; `shm` is *planned*. Since **v0.20.0** the three stores implement a common `SessionStore` interface and the pipeline wires them automatically: `WebApp.WithSession(store)` loads `ctx.Session` from the cookie before the handler and persists it (backend write + Set-Cookie) afterwards — handlers just read/write `ctx.Session`.

The schema has **two orthogonal dimensions**:

- **`strategy`** — *where* the session lives:
  - `server_side` (default) — id in cookie, data on the server (backend below)
  - `encrypted_cookie` — data IS the cookie, signed (and eventually encrypted)
    with `amalgame-crypto`. No server storage; `backend` is ignored.
- **`backend`** — *which* server-side store (when `strategy = "server_side"`):
  - `memory` (default) — in-process Map; single-worker only
  - `shm` (planned) — shared memory between workers of the same node
  - `redis` (shipped) — multi-worker, multi-node (`RedisSessionStore`)

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `strategy` | `"server_side"\|"encrypted_cookie"` | `"server_side"` | `MOSAIC_SESSIONS_STRATEGY` | When `encrypted_cookie`, the cookie value IS the signed session payload (Flask/Rails pattern). v0.1 is signed-only (visible but tamper-proof); AEAD encryption comes with amalgame-crypto v0.2. |
| `secret` | `string` | — | `MOSAIC_SESSIONS_SECRET` | **Required** when `strategy = "encrypted_cookie"`. HMAC-SHA-256 key. Keep stable across deploys; rotation invalidates all signed cookies. |
| `backend` | `"memory"\|"shm"\|"redis"` | `"memory"` | `MOSAIC_SESSIONS_BACKEND` | Ignored when `strategy = "encrypted_cookie"`. `memory` + `redis` shipped; `shm` planned. |
| `dir` | `string` | `"./data/sessions"` | `MOSAIC_SESSIONS_DIR` | For future `file` backend (not in the 3-tier triplet). |
| `url` | `string` | — | `MOSAIC_SESSIONS_URL` | For `redis` (`redis://host:port/db`). |
| `max_age_sec` | `int` | `86400` | `MOSAIC_SESSIONS_MAX_AGE_SEC` | Per-session TTL (server-side stores). |
| `cookie_name` | `string` | `"mosaic_session"` | `MOSAIC_SESSIONS_COOKIE_NAME` | *shipped v0.8.1* (memory) / *v0.8.3* (signed_cookie). |
| `cookie_secure` | `bool` | `true` (when TLS on) | `MOSAIC_SESSIONS_COOKIE_SECURE` | *shipped v0.8.1/v0.8.3*. |
| `cookie_samesite` | `"Strict"\|"Lax"\|"None"` | `"Lax"` | `MOSAIC_SESSIONS_COOKIE_SAMESITE` | *shipped v0.8.1/v0.8.3*. |
| `cookie_path` | `string` | `"/"` | `MOSAIC_SESSIONS_COOKIE_PATH` | *shipped v0.8.1/v0.8.3*. |
| `cookie_max_age` | `int` | `0` | `MOSAIC_SESSIONS_COOKIE_MAX_AGE` | Set-Cookie `Max-Age=`. 0 = session cookie. *shipped v0.8.1/v0.8.3*. |

**SignedCookieSessionStore caveats (v0.8.3, signed-only):**
- Data is visible to anyone who has the cookie (tamper-proof but NOT confidential). Don't store secrets / credentials.
- Keys + values must not contain `&`, `=`, or `.` (no escaping in v0.1 — JSON payload comes in v0.2).
- 4 KB cookie limit ≈ ~30 typical key=value pairs.
- For sensitive data: switch to `strategy = "server_side"` with `backend = "redis"`, or wait for the AEAD-encrypted variant (depends on amalgame-crypto v0.2).

---

### `[security.headers]` — response-side hardening

**Lib:** `Amalgame.Web.SecurityHeaders.FromMap(...)` (since v0.4.1).
**Status:** *shipped* (v0.4.1).

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `preset` | `"strict_html"\|"strict_api"` | — | `MOSAIC_SECURITY_HEADERS_PRESET` | Starting point; subsequent keys override individual fields. |
| `csp` | `string` | — | `MOSAIC_SECURITY_HEADERS_CSP` | Full Content-Security-Policy value. |
| `frame_options` | `"DENY"\|"SAMEORIGIN"` | — | `MOSAIC_SECURITY_HEADERS_FRAME_OPTIONS` | X-Frame-Options. |
| `content_type_options` | `bool` | `false` | `MOSAIC_SECURITY_HEADERS_CONTENT_TYPE_OPTIONS` | True → `X-Content-Type-Options: nosniff`. |
| `referrer_policy` | `string` | — | `MOSAIC_SECURITY_HEADERS_REFERRER_POLICY` | |
| `permissions_policy` | `string` | — | `MOSAIC_SECURITY_HEADERS_PERMISSIONS_POLICY` | |
| `coop` | `string` | — | `MOSAIC_SECURITY_HEADERS_COOP` | Cross-Origin-Opener-Policy. |
| `coep` | `string` | — | `MOSAIC_SECURITY_HEADERS_COEP` | Cross-Origin-Embedder-Policy. |
| `hsts` | `string` | — | `MOSAIC_SECURITY_HEADERS_HSTS` | Pre-composed value; takes precedence over the components below. |
| `hsts_max_age` | `int` | — | `MOSAIC_SECURITY_HEADERS_HSTS_MAX_AGE` | Seconds. |
| `hsts_include_subdomains` | `bool` | `false` | `MOSAIC_SECURITY_HEADERS_HSTS_INCLUDE_SUBDOMAINS` | |
| `hsts_preload` | `bool` | `false` | `MOSAIC_SECURITY_HEADERS_HSTS_PRELOAD` | |

HSTS is intentionally unset by every preset — pinning HSTS on a
response served over HTTP can lock users out of the site. Set
`hsts_max_age` only when TLS is mandatory.

Unknown keys are ignored (forward-compat with future fields).

---

### `[security.cors]` — Cross-Origin Resource Sharing

**Lib:** `Amalgame.Web.Cors.FromMap(...)` + `WebApp.WithCors(...)`.
**Status:** *shipped* — `Cors.FromMap` composes the keys below (covered by `cors_test`); the pipeline handles OPTIONS preflight + stamps `Access-Control-*` on responses.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `preset` | `"disabled"\|"allow_all"\|"strict"` | `"disabled"` | `MOSAIC_SECURITY_CORS_PRESET` | |
| `allowed_origins` | `[string]` | `[]` | `MOSAIC_SECURITY_CORS_ALLOWED_ORIGINS` | Exact origins; `["*"]` = wildcard (incompatible with credentials). |
| `allowed_methods` | `[string]` | `["GET","POST","PUT","PATCH","DELETE","OPTIONS"]` | `MOSAIC_SECURITY_CORS_ALLOWED_METHODS` | |
| `allowed_headers` | `[string]` | `["Content-Type","Authorization"]` | `MOSAIC_SECURITY_CORS_ALLOWED_HEADERS` | Preflight Access-Control-Allow-Headers. |
| `exposed_headers` | `[string]` | `[]` | `MOSAIC_SECURITY_CORS_EXPOSED_HEADERS` | Access-Control-Expose-Headers. |
| `allow_credentials` | `bool` | `false` | `MOSAIC_SECURITY_CORS_ALLOW_CREDENTIALS` | Cookies / Authorization on cross-origin. |
| `max_age_sec` | `int` | `86400` | `MOSAIC_SECURITY_CORS_MAX_AGE_SEC` | Preflight cache TTL. |

---

### `[security.csrf]` — CSRF token validation

**Lib:** `Amalgame.Web.Csrf.FromMap(...)` (v0.7.0).
**Status:** *shipped* — double-submit cookie pattern, 256-bit token entropy via amalgame-random, prefix-matched exempt paths.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `enabled` | `bool` | *omitted = on* | `MOSAIC_SECURITY_CSRF_ENABLED` | Set to `false` to return `Csrf.Disabled()` regardless of other keys. |
| `cookie_name` | `string` | `"csrf_token"` | `MOSAIC_SECURITY_CSRF_COOKIE_NAME` | Double-submit cookie name. |
| `header_name` | `string` | `"X-CSRF-Token"` | `MOSAIC_SECURITY_CSRF_HEADER_NAME` | Where the SPA echoes the cookie value back. |
| `token_bytes` | `int` | `32` | `MOSAIC_SECURITY_CSRF_TOKEN_BYTES` | Entropy bytes (32 = 256-bit token, hex-encoded → 64 chars). |
| `safe_methods` | `[string]` | `["GET","HEAD","OPTIONS"]` | `MOSAIC_SECURITY_CSRF_SAFE_METHODS` | Comma-separated; these bypass Validate. |
| `exempt_paths` | `[string]` | `[]` | `MOSAIC_SECURITY_CSRF_EXEMPT_PATHS` | Comma-separated; prefix-matched against `req.Path`. |
| `cookie_path` | `string` | `"/"` | `MOSAIC_SECURITY_CSRF_COOKIE_PATH` | Set-Cookie `Path=`. |
| `cookie_secure` | `bool` | `true` | `MOSAIC_SECURITY_CSRF_COOKIE_SECURE` | Set-Cookie `Secure` flag (set `false` for HTTP-only dev). |
| `cookie_samesite` | `"Strict"\|"Lax"\|"None"` | `"Lax"` | `MOSAIC_SECURITY_CSRF_COOKIE_SAMESITE` | Set-Cookie `SameSite=`. |
| `cookie_max_age` | `int` | `0` | `MOSAIC_SECURITY_CSRF_COOKIE_MAX_AGE` | Seconds; `0` = session cookie (cleared on browser close). |

**Trade-off note:** the CSRF cookie is intentionally NOT `HttpOnly` —
the SPA needs to read it via JS to echo into the `X-CSRF-Token`
request header. Compensated by:
- `Secure` + `SameSite=Lax` defaults (cookie only sent on TLS + only on top-level navigations or same-site requests)
- Token has no semantic meaning beyond its randomness — it is NOT a session bearer
- 256-bit entropy from `/dev/urandom` / `BCryptGenRandom`

**Form-only flows** (HTML form post without JS): planned for v0.7.x.
Today the validation reads the configured header only; the
`body_key` config will accept a form field name as a fallback.

**Depends on:** [`amalgame-random`](https://github.com/amalgame-lang/amalgame-random) for crypto-grade entropy (`Random.SystemBytes` via `/dev/urandom` / `BCryptGenRandom`).

---

### `[security.auth]` — Basic / JWT authentication

**Lib:** `Amalgame.Web.BasicAuth` / `JwtAuth` + `WebApp.WithBasicAuth(...)` / `WithJwt(...)` + the `Protected()` route group.
**Status:** *shipped (v0.19.0)* — **code-wired, not a TOML table.** Auth has no `FromMap`: a `BasicAuth` verifier is a closure `(user, pwd) -> bool` (it maps to *your* credential store — env, DB, hash compare), which TOML can't express. Only the *secrets* belong in config (env), read in code.

```amalgame
// secrets from the environment; logic in code
let app = WebApp.New()
    .WithJwt(new JwtAuth(Env_Get("JWT_SECRET")))                // HS256 Bearer
    .WithBasicAuth(new BasicAuth("Admin")
        .WithVerifier((u, p) => u == "admin" && p == Env_Get("ADMIN_PASSWORD")))
    .Get("/", home)                                             // public
app.Protected()                                                 // gated group:
    .Get("/admin", dashboard)                                   //   401 unless a
    .Post("/api/x", create)                                     //   configured scheme verifies
```

- Routes registered via `Protected()` carry `Route.Protected = true`; auth runs **after match / before handler**, so public routes and 404s pay nothing.
- A protected route accepts the request if **any** configured scheme verifies, and **fails closed (401)** if none is configured.
- `OAuth2Client` (authorization-code login flow, Github/Google presets) is wired as ordinary route handlers (`StartLogin` / `HandleCallback`), not pipeline middleware.
- **MUST run over TLS** — Basic puts the password and Bearer the token on every request. Pair with `WebApp.ServeHttps` / `ServeHttpsMt`.

---

### `[security.rate_limit]` — per-IP / per-key throttling

**Lib:** `Amalgame.Web.RateLimit.FromMap(...)` (v0.6.0).
**Status:** *shipped* — fixed-window algorithm, in-process memory store, per-IP keying. Sliding-window + Redis backend planned for v2.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `enabled` | `bool` | *omitted = on* | `MOSAIC_SECURITY_RATE_LIMIT_ENABLED` | Set to `false` to return `Disabled()` regardless of other keys. |
| `preset` | `"per_ip"\|"disabled"` | — | `MOSAIC_SECURITY_RATE_LIMIT_PRESET` | Starting point; explicit keys then override. |
| `rps` | `int` | — | `MOSAIC_SECURITY_RATE_LIMIT_RPS` | Shortcut: `max_requests=N, window_sec=1`. |
| `max_requests` | `int` | `0` | `MOSAIC_SECURITY_RATE_LIMIT_MAX_REQUESTS` | Requests per window. `0` = no throttle. |
| `window_sec` | `int` | `1` | `MOSAIC_SECURITY_RATE_LIMIT_WINDOW_SEC` | Window length in seconds. |
| `key_strategy` | `"ip"` | `"ip"` | `MOSAIC_SECURITY_RATE_LIMIT_KEY_STRATEGY` | Only `"ip"` supported today (strips `:port` from `RemoteAddr`). `"user"`/`"custom"` *planned v2*. |
| `trusted_proxies` | `[string]` | `[]` | `MOSAIC_SECURITY_RATE_LIMIT_TRUSTED_PROXIES` | CIDRs whose `X-Forwarded-For` is trusted. *planned v2*. |
| `backend` | `"memory"\|"redis"` | `"memory"` | `MOSAIC_SECURITY_RATE_LIMIT_BACKEND` | Redis *planned v2*. |
| `redis_url` | `string` | — | `MOSAIC_SECURITY_RATE_LIMIT_REDIS_URL` | *planned v2*. |

**Algorithm caveat:** the v1 fixed-window counter can briefly allow
up to `2 × max_requests` across a window boundary (a burst that
straddles two windows). Acceptable for most apps; switch to
sliding-window / token-bucket in v2 if you need strict guarantees.

**Depends on:** [`amalgame-datetime`](https://github.com/amalgame-lang/amalgame-datetime) for the monotonic clock (`DateTime.NowMonotonicNanos()`).

---

### `[logging]` — access log + structured runtime log

**Lib:** `Amalgame.Web.LogConfig.FromMap(...)` + `WebApp.WithLogging(...)`.
**Status:** *shipped* — `WithLogging` applies level + file sink on the amalgame-logging facade and emits one access-log line per request on every path (static hit, middleware reject, route, 404), enriched in v0.18.0 with host + status + duration + client IP.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `level` | `"trace"\|"debug"\|"info"\|"warn"\|"error"` | `"info"` | `MOSAIC_LOGGING_LEVEL` | |
| `format` | `"json"\|"text"` | `"text"` | `MOSAIC_LOGGING_FORMAT` | JSON for prod (greppable). |
| `access_log` | `bool` | `true` | `MOSAIC_LOGGING_ACCESS_LOG` | One line per request. |
| `request_id_header` | `string` | `"X-Request-Id"` | `MOSAIC_LOGGING_REQUEST_ID_HEADER` | Propagated to logs + downstream. |

---

### `[limits]` — server-side resource ceilings

**Lib:** `Amalgame.Net.Http.HttpServerConfig` (C-struct + builders + getters) — fed by the Mosaic CLI to `Http1.ServeWith(port, config, handler)` / `Http2.ServeWith` / `Https.ServeWith` / `Ws.ServeWith` / `Wss.ServeWith`.
**Status:** *shipped end-to-end* — Slowloris timeouts in v0.4.3 (Http1) / v0.4.4 (all variants); H1 parser size-limit wiring in v0.4.5. `idle_timeout_sec` and `listen_backlog` remain planned (keep-alive + listen() refactor respectively).

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `header_timeout_sec` | `int` | `0` (off) | `MOSAIC_LIMITS_HEADER_TIMEOUT_SEC` | **shipped v0.4.3** (Http1) / **v0.4.4** (Http2/Https/Ws/Wss) — `SO_RCVTIMEO` on accepted connection. Slowloris guard. |
| `body_timeout_sec` | `int` | `0` (off) | `MOSAIC_LIMITS_BODY_TIMEOUT_SEC` | **shipped v0.4.3/4** — applied alongside `header_timeout_sec` (larger of the two used as the single phase deadline; v0.4.6 will split). |
| `max_body_bytes` | `int` | `8388608` (8 MiB) | `MOSAIC_LIMITS_MAX_BODY_BYTES` | **shipped v0.4.5** (H1) — parse fails (-1, conn closed) on Content-Length over the limit. H2 size enforcement pending. |
| `max_header_bytes` | `int` | `65536` (64 KiB) | `MOSAIC_LIMITS_MAX_HEADER_BYTES` | **shipped v0.4.5** (H1) — parse fails when the total header-block size exceeds this. |
| `max_url_bytes` | `int` | implicit (bounded by recv buffer) | `MOSAIC_LIMITS_MAX_URL_BYTES` | **shipped v0.4.5** (H1) — parse fails when the request-target length exceeds this. |
| `idle_timeout_sec` | `int` | `0` | `MOSAIC_LIMITS_IDLE_TIMEOUT_SEC` | *planned* — needs HTTP keep-alive (currently `Connection: close` only). |
| `listen_backlog` | `int` | `64` | `MOSAIC_LIMITS_LISTEN_BACKLOG` | *planned* — needs `H1Server_Listen` to thread the value through to `listen(2)`. |

**Caveat for Ws / Wss `header_timeout_sec`:** `SO_RCVTIMEO` persists for the connection lifetime, which breaks long-lived WebSocket frame loops. Handlers that intend long idle waits should clear/raise the timeout themselves (or wait for v0.4.6's post-upgrade auto-clear). For pure HTTP servers this is the right behavior.

**Always-on invariant (not configurable):** `HttpResponse.Header(name, value)`
silently drops any value containing CR (`\r`) or LF (`\n`) — HTTP-response-
splitting prevention. There is no opt-out config: no legitimate header
value contains CR/LF, and the cost of leaving it on is a CVE-class
injection vector. Power-users who need a value verbatim (test fixtures,
trusted internal builders) can call `HeaderUnsafe(name, value)` per
response. Shipped in `amalgame-net-http v0.4.2`.

---

### `[powered_by]` — server-identity advertisement

**Lib:** `Amalgame.Web.PoweredBy.FromMap(...)` + `WebApp.WithPoweredBy(...)` / `WithoutPoweredBy()` (since v0.21.0).
**Status:** *shipped (v0.21.0)*.

Stamps two server-identity headers onto **every** response:

```
X-Powered-By: Mosaic (Amalgame)    (framework convention — Express-style)
Server:       Mosaic (Amalgame)    (server convention — nginx/apache-style)
```

**This is the one feature that is ON by default** — a bare `WebApp.New()`
already advertises the stack (free, passive promotion). Every other
middleware here is opt-in; this one you opt *out* of. Because
`X-Powered-By` is mild fingerprinting (OWASP / helmet recommend dropping
it), set `enabled = false` if you'd rather not disclose the stack to
clients.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `enabled` | `bool` | `true` | `MOSAIC_POWERED_BY_ENABLED` | Master switch. `false` → emits nothing (equivalent to `WithoutPoweredBy()`). |
| `value` | `string` | `"Mosaic (Amalgame)"` | `MOSAIC_POWERED_BY_VALUE` | Advertised identity for both headers. |
| `x_powered_by` | `bool` | `true` | `MOSAIC_POWERED_BY_X_POWERED_BY` | Toggle the `X-Powered-By` header alone. |
| `server` | `bool` | `true` | `MOSAIC_POWERED_BY_SERVER` | Toggle the `Server` header alone. |

Like `SecurityHeaders`, `Apply` never overwrites a header the handler
already set — a handler that sets its own `Server:` wins. Unknown keys
are ignored (forward-compat).

---

## 4. Composing config in code (no `mosaic.toml`)

Apps that don't use the `mosaic` CLI can build the same config in
pure AM — bypassing TOML, env vars, and flags entirely. Every
feature exposes both a builder API and `FromMap(Map<string, string>)`:

```amalgame
// Direct builder — no TOML
let sec = SecurityHeaders.StrictHtml()
    .WithHsts(31536000, true, false)
    .WithCoep("require-corp")

// Via FromMap — what the Mosaic CLI calls after flattening TOML
let cfg = new Map<string, string>()
cfg.Set("preset", "strict_html")
cfg.Set("hsts_max_age", "31536000")
cfg.Set("hsts_include_subdomains", "true")
let sec2 = SecurityHeaders.FromMap(cfg)
```

The library knows nothing about TOML — the CLI flattens the TOML
table and calls `FromMap`. This keeps `amalgame-web`, `amalgame-tls`,
`amalgame-net-http` decoupled from any one config format.

## 5. Versioning the config schema

This file is the schema. When a key is added, removed, or renamed,
the entry here MUST be updated in the same PR. The `mosaic` CLI
treats unknown keys as a soft warning (forward-compat) but produces
no validation error — the consuming `FromMap` ignores them. Apps
get a `mosaic config --check` lint that grep-validates known keys.

## 6. Cross-references

- Design rationale + section skeleton: [`proposals/amalgame-web.md` §18](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-web.md)
- Security roadmap: [`proposals/amalgame-web.md` §21.2 (Phase 1)](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-web.md)
- Configuration strategy notes: agent memory `project_mosaic_config_strategy`
