# Installation

Mosaic est un jeu de packages au-dessus du compilateur `amc`. Installez
d'abord `amc` (voir le chapitre [Démarrer](../01-getting-started.md) ou
[amalgame.me/install](https://amalgame.me/fr/install)), puis ajoutez la
stack web :

```bash
amc package add web net-http tls      # framework + HTTP/1.1 + TLS/ACME
```

`amalgame-web v0.20.0+` tire notamment :

- `amalgame-net-http >= 0.11.1` — requête/réponse HTTP/1.1 (**0.11.1 est
  un plancher obligatoire** : avant, les en-têtes de réponse
  personnalisés — `Set-Cookie`, CSP, CORS — étaient silencieusement
  perdus sur le réseau).
- `amalgame-tls >= 0.3.1` — terminaison TLS + ACME, pour HTTPS.
- `amalgame-crypto`, `amalgame-random`, `amalgame-datetime`,
  `amalgame-logging` — utilisés par les sessions à cookie signé,
  l'entropie CSRF, l'horloge de rate-limit et les logs d'accès.

Nécessite le compilateur `amc` **`>= 0.8.72`** (le `WithSession` de la
v0.20.0 prend une valeur d'interface `SessionStore`, qui requiert le
dispatch d'interface ; un `amc` plus ancien segfault dessus).

## Extras « porte d'entrée » (optionnels)

```bash
amc package add net-proxy             # reverse proxy HTTP/1.1 + load balancing
```

[`amalgame-net-proxy`](https://github.com/amalgame-lang/amalgame-net-proxy)
place N upstreams derrière (routing longest-prefix, X-Forwarded-For,
round-robin pondéré / ip-hash / least-connections) — la porte d'entrée
façon nginx d'un déploiement Mosaic.

## Vérifier

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

Ensuite : le [tour du framework](01-framework.md) et la référence
[Configuration](03-configuration.md) complète.
