# Installation

Mosaic is a set of packages on top of the `amc` compiler. Install `amc`
first (see the [Getting started](../01-getting-started.md) chapter or
[amalgame.me/install](https://amalgame.me/en/install)), then add the
web stack:

```bash
amc package add web net-http tls      # framework + HTTP/1.1 + TLS/ACME
```

`amalgame-web v0.20.0+` pulls in, among others:

- `amalgame-net-http >= 0.11.1` — HTTP/1.1 request/response (**0.11.1 is
  a hard floor**: before it, custom response headers — `Set-Cookie`,
  CSP, CORS — were silently dropped on the wire).
- `amalgame-tls >= 0.3.1` — TLS termination + ACME, for HTTPS.
- `amalgame-crypto`, `amalgame-random`, `amalgame-datetime`,
  `amalgame-logging` — used by signed-cookie sessions, CSRF entropy,
  the rate-limit clock, and access logs respectively.

Requires the `amc` compiler **`>= 0.8.72`** (the v0.20.0 `WithSession`
takes a `SessionStore` interface value, which needs interface dispatch;
older `amc` segfaults on it).

## Front-door extras (optional)

```bash
amc package add net-proxy             # HTTP/1.1 reverse proxy + load balancing
```

[`amalgame-net-proxy`](https://github.com/amalgame-lang/amalgame-net-proxy)
fronts N upstreams (longest-prefix routing, X-Forwarded-For, weighted
round-robin / ip-hash / least-connections) — the nginx-style front door
for a Mosaic deployment.

## Verify

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
