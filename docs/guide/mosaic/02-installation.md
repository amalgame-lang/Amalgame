# Installation

Mosaic ships in two forms, and you pick the one that matches how you
want to run it:

- **The server** — a prebuilt `mosaic` binary you install once and drive
  from a `mosaic.toml`. No Amalgame code, no compiler. This is the
  nginx/Caddy-style path: install, configure, `mosaic serve`.
- **The library** — `amalgame-web` (+ the stack) added to your own
  Amalgame project with `amc package add`, when you want to write your
  own routes and handlers in Amalgame.

## Four ways to deploy

| # | You want to… | Use |
|---|---|---|
| 1 | Write your own server in Amalgame (full control) | `amc package add web net-http tls` — DIY, see the [framework tour](01-framework.md) |
| 2 | Serve static sites + features from a config file, no code | the **`mosaic serve`** binary + a `mosaic.toml` |
| 3 | Run a reverse proxy / load balancer by configuration | the **`mosaic serve`** binary + `[[proxy]]` in `mosaic.toml` |
| 4 | Compose your own binary from just the packages you want | `amc package add` the subset (e.g. `web net-proxy`) and build |

Models 2 and 3 are the same binary — see [Configuration](03-configuration.md)
for the full `mosaic.toml` schema.

## Install the server

```bash
curl -sSL https://raw.githubusercontent.com/amalgame-lang/mosaic/main/install.sh | bash
```

This drops the `mosaic` dispatcher plus a prebuilt **`mosaic-serve`**
binary (Linux x86_64) into `~/.local/bin`. Then:

```bash
mosaic serve /etc/mosaic/mosaic.toml
```

One binary, one config, N sites over HTTPS — static hosting, TLS + ACME,
per-site middleware (security headers, CORS, CSRF, rate limiting,
logging), and reverse-proxy / load-balanced hosts. A minimal config:

```toml
[server]
port = 443
tls  = true

[tls]
acme  = true
email = "admin@example.com"

[[site]]
hosts = ["example.com", "www.example.com"]
root  = "/srv/example/public"
```

> On platforms without a prebuilt `mosaic-serve` (macOS / Windows today),
> `mosaic serve` falls back to building it from source — that needs `amc`
> plus the stack packages below.

## Install the library (DIY)

Install `amc` first (see [Getting started](../01-getting-started.md) or
[amalgame.me/install](https://amalgame.me/en/install)), then add the
web stack to your project:

```bash
amc package add web net-http tls      # framework + HTTP/1.1 + TLS/ACME
```

`amalgame-web v0.23.0+` pulls in, among others:

- `amalgame-net-http >= 0.13.6` — HTTP/1.1 request/response, SNI +
  keep-alive (**a hard floor**: before 0.11.1, custom response headers —
  `Set-Cookie`, CSP, CORS — were silently dropped on the wire).
- `amalgame-tls >= 0.3.1` — TLS termination + ACME, for HTTPS.
- `amalgame-crypto`, `amalgame-random`, `amalgame-datetime`,
  `amalgame-logging` — used by signed-cookie sessions, CSRF entropy,
  the rate-limit clock, and access logs respectively.

Requires the `amc` compiler **`>= 0.8.72`** (the v0.20.0 `WithSession`
takes a `SessionStore` interface value, which needs interface dispatch;
older `amc` segfaults on it).

### Front-door extras (optional)

```bash
amc package add net-proxy             # HTTP/1.1 reverse proxy + load balancing
```

[`amalgame-net-proxy`](https://github.com/amalgame-lang/amalgame-net-proxy)
fronts N upstreams (longest-prefix routing, X-Forwarded-For, weighted
round-robin / ip-hash / least-connections) — the nginx-style front door.
The `mosaic serve` binary already bundles it for `[[proxy]]` config.

## Verify (library)

```amalgame
import Amalgame.Web

WebApp.New()
    .Get("/", ctx => HttpResponse.New().Text("ok"))
    .Serve(8080)
```

```bash
amc run server.am          # → "listening on :8080"
curl -s localhost:8080/    # → ok
```

Next: the [framework tour](01-framework.md) and the full
[Configuration](03-configuration.md) reference.
