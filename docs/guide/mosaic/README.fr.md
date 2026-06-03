# Mosaic

**Mosaic** est le framework web côté serveur d'Amalgame — le package
[`amalgame-web`](https://github.com/amalgame-lang/amalgame-web) plus la
couche HTTP / TLS sur laquelle il s'appuie. Un binaire, N sites, HTTPS /
ACME automatique, sessions, auth, et une « porte d'entrée » reverse
proxy.

- [**Framework web**](01-framework.md) — routing, middlewares (security
  headers, CORS, CSRF, rate limiting), sessions, auth (`Protected()`),
  fichiers statiques, et les points d'entrée serveur HTTP / HTTPS.
- [**Installation**](02-installation.md) — `amc package add` de la stack,
  plus la porte d'entrée reverse proxy optionnelle.
- [**Configuration**](03-configuration.md) — la référence complète
  `mosaic.toml` / `MOSAIC_*` : chaque section, clé, défaut et statut.

```amalgame
import Amalgame.Web

WebApp.New()
    .Get("/", ctx => HttpResponse.New().Html("<h1>Bonjour Mosaic</h1>"))
    .ServeHttps(443, "/etc/ssl/cert.pem", "/etc/ssl/key.pem")
```
