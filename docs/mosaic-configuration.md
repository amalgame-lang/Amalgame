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

**Lib:** `Amalgame.Web.MemorySessionStore` (today) / `JsonFileSessionStore` / `RedisSessionStore` (*planned*).
**Status:** *partial* — `memory` backend ships; `json_file` and `redis` are *planned*.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `backend` | `"memory"\|"json_file"\|"redis"` | `"memory"` | `MOSAIC_SESSIONS_BACKEND` | |
| `dir` | `string` | `"./data/sessions"` | `MOSAIC_SESSIONS_DIR` | For `json_file`. |
| `url` | `string` | — | `MOSAIC_SESSIONS_URL` | For `redis` (`redis://host:port/db`). |
| `max_age_sec` | `int` | `86400` | `MOSAIC_SESSIONS_MAX_AGE_SEC` | Per-session TTL. |
| `cookie_name` | `string` | `"mosaic_session"` | `MOSAIC_SESSIONS_COOKIE_NAME` | *planned*. |
| `cookie_secure` | `bool` | `true` (when TLS on) | `MOSAIC_SESSIONS_COOKIE_SECURE` | *planned*. |
| `cookie_samesite` | `"Strict"\|"Lax"\|"None"` | `"Lax"` | `MOSAIC_SESSIONS_COOKIE_SAMESITE` | *planned*. |
| `cookie_path` | `string` | `"/"` | `MOSAIC_SESSIONS_COOKIE_PATH` | *planned*. |

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

**Lib:** `Amalgame.Web.Csrf.FromMap(...)` (*planned*).
**Status:** *planned*.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `enabled` | `bool` | `false` | `MOSAIC_SECURITY_CSRF_ENABLED` | |
| `cookie_name` | `string` | `"csrf_token"` | `MOSAIC_SECURITY_CSRF_COOKIE_NAME` | Double-submit cookie. |
| `header_name` | `string` | `"X-CSRF-Token"` | `MOSAIC_SECURITY_CSRF_HEADER_NAME` | Where the SPA sends the token back. |
| `safe_methods` | `[string]` | `["GET","HEAD","OPTIONS"]` | `MOSAIC_SECURITY_CSRF_SAFE_METHODS` | Methods skipped from validation. |
| `exempt_paths` | `[string]` | `[]` | `MOSAIC_SECURITY_CSRF_EXEMPT_PATHS` | Glob-matched. |

---

### `[security.rate_limit]` — per-IP / per-key throttling

**Lib:** `Amalgame.Web.RateLimit.FromMap(...)` (*planned*).
**Status:** *planned*.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `enabled` | `bool` | `false` | `MOSAIC_SECURITY_RATE_LIMIT_ENABLED` | |
| `rps` | `int` | `100` | `MOSAIC_SECURITY_RATE_LIMIT_RPS` | Sustained req/sec. |
| `burst` | `int` | `200` | `MOSAIC_SECURITY_RATE_LIMIT_BURST` | Token bucket size. |
| `key_strategy` | `"ip"\|"user"\|"custom"` | `"ip"` | `MOSAIC_SECURITY_RATE_LIMIT_KEY_STRATEGY` | |
| `trusted_proxies` | `[string]` | `[]` | `MOSAIC_SECURITY_RATE_LIMIT_TRUSTED_PROXIES` | CIDRs whose `X-Forwarded-For` we trust. |
| `backend` | `"memory"\|"redis"` | `"memory"` | `MOSAIC_SECURITY_RATE_LIMIT_BACKEND` | Redis backend *planned v2*. |
| `redis_url` | `string` | — | `MOSAIC_SECURITY_RATE_LIMIT_REDIS_URL` | |

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

**Lib:** `Amalgame.Net.Http.HttpServerConfig.FromMap(...)` (*planned*).
**Status:** *planned* — currently hardcoded in the runtime.

| Key | Type | Default | Env | Notes |
|---|---|---|---|---|
| `max_body_size_mb` | `int` | `8` | `MOSAIC_LIMITS_MAX_BODY_SIZE_MB` | 413 returned on overflow. |
| `max_header_size_kb` | `int` | `16` | `MOSAIC_LIMITS_MAX_HEADER_SIZE_KB` | 431 on overflow. |
| `max_url_length` | `int` | `2048` | `MOSAIC_LIMITS_MAX_URL_LENGTH` | Bytes. |
| `header_timeout_sec` | `int` | `10` | `MOSAIC_LIMITS_HEADER_TIMEOUT_SEC` | Slowloris guard — `SO_RCVTIMEO` on the connection until headers parsed. |
| `body_timeout_sec` | `int` | `30` | `MOSAIC_LIMITS_BODY_TIMEOUT_SEC` | Idle-during-body guard. |
| `idle_keepalive_sec` | `int` | `60` | `MOSAIC_LIMITS_IDLE_KEEPALIVE_SEC` | *needs HTTP keep-alive support — currently `Connection: close` only.* |

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
