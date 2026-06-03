# Mosaic

**Mosaic** is Amalgame's web stack — and you can run it two ways. As a
**server**: install the prebuilt `mosaic` binary, write a `mosaic.toml`,
and `mosaic serve` — N sites, automatic HTTPS / ACME, middleware, and a
reverse-proxy / load balancer, with no Amalgame code (nginx/Caddy
style). Or as a **library**: the
[`amalgame-web`](https://github.com/amalgame-lang/amalgame-web) package
plus the HTTP / TLS layer, when you want to write your own handlers.

- [**Web framework**](01-framework.md) — routing, middleware (security
  headers, CORS, CSRF, rate limiting), sessions, auth (`Protected()`),
  static files, and the HTTP / HTTPS serve entry points.
- [**Installation**](02-installation.md) — the `mosaic serve` binary and
  the four ways to deploy, plus `amc package add` for the library path.
- [**Configuration**](03-configuration.md) — the `mosaic serve` file
  schema (`[[site]]` / `[[proxy]]`) and the full `mosaic.toml` /
  `MOSAIC_*` reference: every section, key, default, and status.

```amalgame
import Amalgame.Web

WebApp.New()
    .Get("/", ctx => HttpResponse.New().Html("<h1>Hello Mosaic</h1>"))
    .ServeHttps(443, "/etc/ssl/cert.pem", "/etc/ssl/key.pem")
```
