# Installation

Mosaic existe sous deux formes ; choisissez celle qui correspond à votre
manière de le faire tourner :

- **Le serveur** — un binaire `mosaic` pré-compilé que l'on installe une
  fois et que l'on pilote depuis un `mosaic.toml`. Aucun code Amalgame,
  aucun compilateur. C'est le chemin façon nginx/Caddy : on installe, on
  configure, `mosaic serve`.
- **La bibliothèque** — `amalgame-web` (+ la stack) ajouté à votre propre
  projet Amalgame via `amc package add`, quand vous voulez écrire vos
  propres routes et handlers en Amalgame.

## Quatre façons de déployer

| # | Vous voulez… | Utilisez |
|---|---|---|
| 1 | Écrire votre serveur en Amalgame (contrôle total) | `amc package add web net-http tls` — DIY, voir le [tour du framework](01-framework.md) |
| 2 | Servir des sites statiques + features depuis un fichier, sans code | le binaire **`mosaic serve`** + un `mosaic.toml` |
| 3 | Lancer un reverse proxy / load balancer par configuration | le binaire **`mosaic serve`** + `[[proxy]]` dans `mosaic.toml` |
| 4 | Composer votre propre binaire avec juste les packages voulus | `amc package add` le sous-ensemble (ex. `web net-proxy`) puis build |

Les modèles 2 et 3 sont le même binaire — voir [Configuration](03-configuration.md)
pour le schéma complet du `mosaic.toml`.

## Installer le serveur

```bash
curl -sSL https://raw.githubusercontent.com/amalgame-lang/mosaic/main/install.sh | bash
```

Cela dépose le dispatcher `mosaic` plus un binaire **`mosaic-serve`**
pré-compilé (Linux x86_64) dans `~/.local/bin`. Ensuite :

```bash
mosaic serve /etc/mosaic/mosaic.toml
```

Un binaire, une config, N sites en HTTPS — hébergement statique, TLS +
ACME, middleware par site (en-têtes de sécurité, CORS, CSRF, rate
limiting, logs) et hosts en reverse proxy / load balancing. Config
minimale :

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

Pour le faire tourner au fil des reboots et des crashs, enregistre-le
comme service en une commande — `mosaic service install` écrit une unit
systemd/launchd/SCM qui lance `mosaic serve` pour toi. Voir
[Configuration → Tourner en service](03-configuration.fr.md#4-tourner-en-service).

> Sur les plateformes sans `mosaic-serve` pré-compilé (macOS / Windows
> aujourd'hui), `mosaic serve` retombe sur un build depuis les sources —
> ce qui nécessite `amc` plus les packages de la stack ci-dessous.

## Installer la bibliothèque (DIY)

Installez d'abord `amc` (voir [Premiers pas](../01-getting-started.md) ou
[amalgame.me/install](https://amalgame.me/fr/install)), puis ajoutez la
stack web à votre projet :

```bash
amc package add web net-http tls      # framework + HTTP/1.1 + TLS/ACME
```

`amalgame-web v0.23.0+` tire, entre autres :

- `amalgame-net-http >= 0.13.6` — requête/réponse HTTP/1.1, SNI +
  keep-alive (**plancher dur** : avant 0.11.1, les en-têtes de réponse
  custom — `Set-Cookie`, CSP, CORS — étaient silencieusement perdus sur
  le fil).
- `amalgame-tls >= 0.3.1` — terminaison TLS + ACME, pour le HTTPS.
- `amalgame-crypto`, `amalgame-random`, `amalgame-datetime`,
  `amalgame-logging` — utilisés par les sessions signées, l'entropie
  CSRF, l'horloge du rate-limit et les access logs respectivement.

Nécessite le compilateur `amc` **`>= 0.8.72`** (le `WithSession` de
v0.20.0 prend une valeur d'interface `SessionStore`, qui demande le
dispatch d'interface ; un `amc` plus ancien segfault dessus).

### Extras de façade (optionnel)

```bash
amc package add net-proxy             # reverse proxy HTTP/1.1 + load balancing
```

[`amalgame-net-proxy`](https://github.com/amalgame-lang/amalgame-net-proxy)
place N upstreams en façade (routage par plus long préfixe,
X-Forwarded-For, round-robin pondéré / ip-hash / least-connections) — la
porte d'entrée façon nginx. Le binaire `mosaic serve` l'embarque déjà
pour la config `[[proxy]]`.

## Vérifier (bibliothèque)

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

Suite : le [tour du framework](01-framework.md) et la référence complète
de [Configuration](03-configuration.md).
