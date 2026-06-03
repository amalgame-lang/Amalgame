# Mosaic

**Mosaic** is Amalgame's server-side web framework — the
[`amalgame-web`](https://github.com/amalgame-lang/amalgame-web) package
plus the HTTP / TLS layer it builds on. One binary, N sites, automatic
HTTPS / ACME, sessions, auth, and a reverse-proxy front door.

- [**Web framework**](01-framework.md) — routing, middleware (security
  headers, CORS, CSRF, rate limiting), sessions, auth (`Protected()`),
  static files, and the HTTP / HTTPS serve entry points.
- [**Installation**](02-installation.md) — `amc package add` the stack,
  plus the optional reverse-proxy front door.
- [**Configuration**](03-configuration.md) — the full `mosaic.toml` /
  `MOSAIC_*` reference: every section, key, default, and status.

```amalgame
import Amalgame.Web

WebApp.New()
    .Get("/", ctx => HttpResponse.New().Html("<h1>Hello Mosaic</h1>"))
    .ServeHttps(443, "/etc/ssl/cert.pem", "/etc/ssl/key.pem")
```
