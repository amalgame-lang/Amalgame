# Mosaic — Configuration Reference

Canonical reference for every knob in the Mosaic stack: TOML key,
default, env-var override, and the library builder it maps to.

This doc is the **contract**. If a feature lands without a matching
section here, it's not finished. Source of truth for the build tool
(`mosaic` CLI) when it reads `mosaic.toml`, sets up env-var overrides,
and instantiates library builders.

For the design rationale, see `docs/proposals/amalgame-web.md` §18.

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

---

### `[tls]` — TLS / HTTPS / ACME

**Lib:** `Amalgame.Tls.TlsConfig` + `Amalgame.Tls.Acme`.
**Status:** *partial* — `acme` mode works (certbot wrapper); `files` and `off` work; the extra knobs below are *planned*.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `mode` | `"off"\|"files"\|"acme"` | `"off"` | `MOSAIC_TLS_MODE` | Top-level switch. |
| `acme_email` | `string` | — | `MOSAIC_TLS_ACME_EMAIL` | Required when mode = `"acme"`. |
| `acme_cache` | `string` | `"./data/acme"` | `MOSAIC_TLS_ACME_CACHE` | Cert/account dir. |
| `domains` | `[string]` | `[]` | `MOSAIC_TLS_DOMAINS` | SANs to provision. |
| `acme_server` | `string` | LE production | `MOSAIC_TLS_ACME_SERVER` | ACME directory URL. *planned* — Buypass / ZeroSSL / LE-staging by passing the directory URL. |
| `certbot_path` | `string` | `"certbot"` (looked up via `PATH`) | `MOSAIC_TLS_CERTBOT_PATH` | Absolute path override. *planned*. |
| `cert_file` | `string` | — | `MOSAIC_TLS_CERT_FILE` | Required when mode = `"files"`. |
| `key_file` | `string` | — | `MOSAIC_TLS_KEY_FILE` | Required when mode = `"files"`. |
| `min_version` | `"1.2"\|"1.3"` | `"1.3"` | `MOSAIC_TLS_MIN_VERSION` | Maps to `TlsConfig.WithMinVersion`. |
| `alpn` | `string` | `"h2,http/1.1"` | `MOSAIC_TLS_ALPN` | ALPN list (comma-separated). |

---

### `[sessions]` — server-side session storage

**Lib:** `Amalgame.Web.MemorySessionStore` / `SignedCookieSessionStore` (signed + AEAD modes) / `RedisSessionStore`.
**Status:** *fully shipped* — `memory` (v0.8.1, made thread-safe in v0.9.0) + `signed_cookie` strategy (signed v0.8.3, encrypted v0.8.5) + `redis` backend (v0.8.4) + `shm` (v0.9.0; alias of `memory` since `MemorySessionStore` is now thread-safe under `Http1.ServeMt`).

The schema has **two orthogonal dimensions**:

- **`strategy`** — *where* the session lives:
  - `server_side` (default) — id in cookie, data on the server (backend below)
  - `encrypted_cookie` — data IS the cookie, signed (and eventually encrypted)
    with `amalgame-crypto`. No server storage; `backend` is ignored.
- **`backend`** — *which* server-side store (when `strategy = "server_side"`):
  - `memory` (default, v0.9.0 thread-safe) — in-process Map. Safe under `Http1.ServeMt` (multi-thread accept loop) thanks to a process-wide mutex around the Map. Single-process, lost on restart.
  - `shm` (v0.9.0; alias of `memory`) — historically reserved for "shared between workers of the same node". Since `memory` is now thread-safe, `shm` is recognised as a synonym; both wire to `MemorySessionStore`.
  - `redis` (v0.8.4) — multi-worker, multi-node; uses `amalgame-database-nosql-redis`

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `strategy` | `"server_side"\|"encrypted_cookie"` | `"server_side"` | `MOSAIC_SESSIONS_STRATEGY` | When `encrypted_cookie`, the cookie value IS the encoded session payload (Flask/Rails pattern). Pair with `encrypted = "true"` to switch from HMAC-signed plain text (v0.8.3) to AES-256-GCM AEAD (v0.8.5, confidential + authenticated). |
| `encrypted` | `bool` | `false` | `MOSAIC_SESSIONS_ENCRYPTED` | *shipped v0.8.5*. When `true`, payload is sealed with AES-256-GCM (key = SHA-256(secret), fresh 12-byte nonce per Encode). Wire format `<nonce_hex_24>.<ct_and_tag_hex>`. |
| `secret` | `string` | — | `MOSAIC_SESSIONS_SECRET` | **Required** when `strategy = "encrypted_cookie"`. HMAC-SHA-256 key. Keep stable across deploys; rotation invalidates all signed cookies. |
| `backend` | `"memory"\|"shm"\|"redis"` | `"memory"` | `MOSAIC_SESSIONS_BACKEND` | Ignored when `strategy = "encrypted_cookie"`. |
| `dir` | `string` | `"./data/sessions"` | `MOSAIC_SESSIONS_DIR` | For future `file` backend (not in the 3-tier triplet). |
| `host` | `string` | `"127.0.0.1"` | `MOSAIC_SESSIONS_HOST` | For `redis` backend. *shipped v0.8.4*. |
| `port` | `int` | `6379` | `MOSAIC_SESSIONS_PORT` | For `redis` backend. *shipped v0.8.4*. |
| `key_prefix` | `string` | `"mosaic:session"` | `MOSAIC_SESSIONS_KEY_PREFIX` | For `redis` backend — Redis key namespace. *shipped v0.8.4*. |
| `url` | `string` | — | `MOSAIC_SESSIONS_URL` | Reserved for future `redis://host:port/db` parsing; for now use `host` + `port`. |
| `max_age_sec` | `int` | `86400` | `MOSAIC_SESSIONS_MAX_AGE_SEC` | Per-session TTL (server-side stores). |
| `cookie_name` | `string` | `"mosaic_session"` | `MOSAIC_SESSIONS_COOKIE_NAME` | *shipped v0.8.1* (memory) / *v0.8.3* (signed_cookie). |
| `cookie_secure` | `bool` | `true` (when TLS on) | `MOSAIC_SESSIONS_COOKIE_SECURE` | *shipped v0.8.1/v0.8.3*. |
| `cookie_samesite` | `"Strict"\|"Lax"\|"None"` | `"Lax"` | `MOSAIC_SESSIONS_COOKIE_SAMESITE` | *shipped v0.8.1/v0.8.3*. |
| `cookie_path` | `string` | `"/"` | `MOSAIC_SESSIONS_COOKIE_PATH` | *shipped v0.8.1/v0.8.3*. |
| `cookie_max_age` | `int` | `0` | `MOSAIC_SESSIONS_COOKIE_MAX_AGE` | Set-Cookie `Max-Age=`. 0 = session cookie. *shipped v0.8.1/v0.8.3*. |

**SignedCookieSessionStore caveats:**
- Signed-only mode (`encrypted = false`, default): data is visible to anyone who has the cookie (tamper-proof but NOT confidential). Don't store secrets / credentials.
- Encrypted mode (`encrypted = true`, v0.8.5): confidential AND authenticated via AES-256-GCM. Cookie is ~2× larger (hex encoding + 16-byte tag + 12-byte nonce overhead). Safe for moderate-sensitivity payloads (user_id, email, role).
- Keys + values must not contain `&`, `=`, or `.` (no escaping in v0.1 — JSON payload planned).
- 4 KB cookie limit ≈ ~30 typical key=value pairs.
- Switch to `strategy = "server_side"` for very large session payloads or when revocation must be synchronous (cookie sessions can only be revoked by rotating the secret, which invalidates everyone).

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

**Lib:** `Amalgame.Web.Cors.FromMap(...)` (*planned*).
**Status:** *planned* — design locked in via this section.

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

### `[security.rate_limit]` — per-IP / per-key throttling

**Lib:** `Amalgame.Web.RateLimit.FromMap(...)` (v0.6.0).
**Status:** *shipped* — fixed-window algorithm, `memory` (v0.6.0, thread-safe in v0.9.0) + `redis` (v0.8.4) backends, per-IP keying. Sliding-window planned for v2.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `enabled` | `bool` | *omitted = on* | `MOSAIC_SECURITY_RATE_LIMIT_ENABLED` | Set to `false` to return `Disabled()` regardless of other keys. |
| `preset` | `"per_ip"\|"disabled"` | — | `MOSAIC_SECURITY_RATE_LIMIT_PRESET` | Starting point; explicit keys then override. |
| `rps` | `int` | — | `MOSAIC_SECURITY_RATE_LIMIT_RPS` | Shortcut: `max_requests=N, window_sec=1`. |
| `max_requests` | `int` | `0` | `MOSAIC_SECURITY_RATE_LIMIT_MAX_REQUESTS` | Requests per window. `0` = no throttle. |
| `window_sec` | `int` | `1` | `MOSAIC_SECURITY_RATE_LIMIT_WINDOW_SEC` | Window length in seconds. |
| `key_strategy` | `"ip"` | `"ip"` | `MOSAIC_SECURITY_RATE_LIMIT_KEY_STRATEGY` | Only `"ip"` supported today (strips `:port` from `RemoteAddr`). `"user"`/`"custom"` *planned v2*. |
| `trusted_proxies` | `[string]` | `[]` | `MOSAIC_SECURITY_RATE_LIMIT_TRUSTED_PROXIES` | CIDRs whose `X-Forwarded-For` is trusted. *planned v2*. |
| `backend` | `"memory"\|"redis"` | `"memory"` | `MOSAIC_SECURITY_RATE_LIMIT_BACKEND` | *shipped v0.8.4* — `redis` uses INCR + EXPIRE per window (atomic on the counter, EXPIRE set-once via INCR==1). |
| `redis_host` | `string` | `"127.0.0.1"` | `MOSAIC_SECURITY_RATE_LIMIT_REDIS_HOST` | *shipped v0.8.4*. |
| `redis_port` | `int` | `6379` | `MOSAIC_SECURITY_RATE_LIMIT_REDIS_PORT` | *shipped v0.8.4*. |
| `redis_key_prefix` | `string` | `"mosaic:rl"` | `MOSAIC_SECURITY_RATE_LIMIT_REDIS_KEY_PREFIX` | *shipped v0.8.4*. |
| `redis_url` | `string` | — | `MOSAIC_SECURITY_RATE_LIMIT_REDIS_URL` | Reserved for future `redis://` URL parsing; use the three keys above for now. |

**Algorithm caveat:** the v1 fixed-window counter can briefly allow
up to `2 × max_requests` across a window boundary (a burst that
straddles two windows). Acceptable for most apps; switch to
sliding-window / token-bucket in v2 if you need strict guarantees.

**Depends on:** [`amalgame-datetime`](https://github.com/amalgame-lang/amalgame-datetime) for the monotonic clock (`DateTime.NowMonotonicNanos()`).

---

### `[logging]` — access log + structured runtime log

**Lib:** `Amalgame.Web.Logger.FromMap(...)` (*planned*).
**Status:** *planned*.

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

- Design rationale + section skeleton: [`proposals/amalgame-web.md` §18](./proposals/amalgame-web.md)
- Security roadmap: [`proposals/amalgame-web.md` §21.2 (Phase 1)](./proposals/amalgame-web.md)
- Configuration strategy notes: agent memory `project_mosaic_config_strategy`
