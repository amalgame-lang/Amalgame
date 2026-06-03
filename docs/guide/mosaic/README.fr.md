# Mosaic

**Mosaic** est la stack web d'Amalgame — et on peut la faire tourner de
deux façons. Comme **serveur** : on installe le binaire `mosaic`
pré-compilé, on écrit un `mosaic.toml`, et `mosaic serve` — N sites,
HTTPS / ACME automatique, middlewares, et un reverse proxy / load
balancer, sans aucun code Amalgame (façon nginx/Caddy). Ou comme
**bibliothèque** : le package
[`amalgame-web`](https://github.com/amalgame-lang/amalgame-web) plus la
couche HTTP / TLS, quand on veut écrire ses propres handlers.

- [**Framework web**](01-framework.md) — routing, middlewares (security
  headers, CORS, CSRF, rate limiting), sessions, auth (`Protected()`),
  fichiers statiques, et les points d'entrée serveur HTTP / HTTPS.
- [**Installation**](02-installation.md) — le binaire `mosaic serve` et
  les quatre façons de déployer, plus `amc package add` pour la
  bibliothèque.
- [**Configuration**](03-configuration.md) — le schéma de fichier
  `mosaic serve` (`[[site]]` / `[[proxy]]`) et la référence complète
  `mosaic.toml` / `MOSAIC_*` : chaque section, clé, défaut et statut.

```amalgame
import Amalgame.Web

WebApp.New()
    .Get("/", ctx => HttpResponse.New().Html("<h1>Bonjour Mosaic</h1>"))
    .ServeHttps(443, "/etc/ssl/cert.pem", "/etc/ssl/key.pem")
```
