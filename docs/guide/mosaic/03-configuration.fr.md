# Configuration

Référence canonique de chaque réglage de la stack Mosaic : clé TOML,
défaut, override par variable d'environnement, et le builder de
bibliothèque correspondant. Voir aussi
[Installation](02-installation.md) et le [tour du framework](01-framework.md).

Ce document est le **contrat**. Si une feature arrive sans section
correspondante ici, elle n'est pas finie. C'est la source de vérité de
l'outil de build (CLI `mosaic`) quand il lit `mosaic.toml`, met en place
les overrides d'environnement et instancie les builders de bibliothèque.

Pour le rationnel de conception, voir [`docs/proposals/amalgame-web.md` §18](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-web.md).

---

## 1. La cascade d'override

Il n'y a pas trois mais **six** couches, de la moins prioritaire à la plus :

```
1. Défaut bibliothèque (le constructeur de la classe)
2. mosaic.toml         (le CLI l'aplatit et appelle FromMap sur chaque feature)
3. variables d'env     (MOSAIC_<SECTION>_<KEY> — appliquées par le CLI, ou
                        lues directement par la lib pour les réglages runtime
                        comme MOSAIC_TLS_ACME_SERVER)
4. flags CLI           (mosaic dev --listen :8080)
5. Code applicatif     (builder chaîné dans main.am APRÈS que le CLI a
                        rendu l'instance pré-configurée)
6. Handler par route   (`resp.Header(...)` dans la closure de route — Apply
                        n'écrase jamais un en-tête déjà posé par le handler)
```

**« Le code » n'est pas une couche mais deux** : le défaut bibliothèque
est tout en bas (c'est ce que produit le constructeur), votre chaînage
dans `main.am` est au milieu, et votre handler de route est tout en
haut. Ils se ressemblent pour un lecteur mais occupent des positions
très différentes dans la cascade.

**Le handler gagne, c'est intentionnel** : une seule route exceptionnelle
(ex. `/embed` qui doit autoriser un `<iframe>`) peut surcharger une
politique globale sans la désactiver ailleurs. Le défaut prudent reste
prudent partout ailleurs.

### Deux scénarios

**Avec le CLI Mosaic :** TOML / env / flags sont empilés par le CLI, qui
appelle le `FromMap(...)` de chaque feature et livre l'instance
pré-configurée à votre `main.am`. Votre code peut chaîner d'autres
appels `With*(...)` — ceux-ci écrasent les valeurs appliquées par le CLI,
ce qui est exactement ce que l'on veut (le code est plus proche des
routes que le fichier de config).

**Sans le CLI Mosaic :** vous construisez les instances directement dans
`main.am` avec les builders. TOML / env / flags ne s'appliquent pas. La
bibliothèque n'a aucune dépendance à TOML ni à un quelconque CLI.

### Une feature = une instance

Par WebApp, vous devez avoir exactement un `SecurityHeaders` (et un
`Cors`, un `Csrf`, un `RateLimit`, …). Plusieurs instances sont une
source de contradictions silencieuses. Les sous-apps montées sur des
chemins différents sont le seul cas légitime d'en avoir plus d'une.

## 2. Emplacement du fichier

```
my-app/
├── amalgame.toml          # manifeste du package
├── mosaic.toml            # ← ce fichier
├── app/                   # routes
└── main.am
```

Un `mosaic.toml` absent équivaut à un fichier vide — chaque feature
retombe sur son défaut bibliothèque.

## 2b. Le schéma de fichier `mosaic serve`

Voici ce que le binaire pré-compilé **`mosaic serve`** lit réellement
(aucun code Amalgame). Il assemble un `MosaicServer` à partir du fichier :
N blocs statiques `[[site]]` + N blocs reverse-proxy `[[proxy]]`,
partageant tous un même front HTTPS (dispatch par Host + SNI + ACME +
redirect `:80`→`:443`). La [Référence](#3-référence) ci-dessous documente
chaque clé de middleware en détail ; cette section donne la disposition
de haut niveau du fichier.

```toml
[server]
port = 443            # défaut : 443 quand tls, sinon 8080
tls  = true

[tls]
acme     = true
email    = "admin@example.com"
cert_dir = "/var/mosaic/certs"   # <cert_dir>/<domain>/{fullchain,privkey}.pem

# ── Un site statique par Host ───────────────────────────
[[site]]
hosts = ["example.com", "www.example.com"]   # www se replie sur l'apex
root  = "/srv/example/public"
  [site.security.headers]                     # → SecurityHeaders.FromMap
  preset = "strict_html"
  [site.security.cors]                        # → Cors.FromMap
  preset = "strict"
  [site.logging]                              # → LogConfig.FromMap
  access_log = true

# ── Un host en reverse-proxy / load-balancé ─────────────
[[proxy]]
hosts  = ["api.example.com"]
routes = [
  { prefix = "/",      upstream  = "http://127.0.0.1:8080" },
  { prefix = "/heavy", upstreams = ["http://127.0.0.1:9001", "http://127.0.0.1:9002"], strategy = "round_robin" },
]
```

**`[[site]]`** — un site statique. `hosts` = les domaines qu'il sert
(chaque host non-`www.` rejoint l'ensemble SNI/ACME) ; `root` = le
répertoire de fichiers statiques (`/` → `index.html`, `/<slug>` →
`<slug>.html`, assets par type MIME). Les sous-tables
`[site.security.*]` / `[site.logging]` sont aplaties et passées à la
factory `FromMap` correspondante — mêmes clés que les sections de la
[Référence](#3-référence) ci-dessous.

**`[[proxy]]`** — un reverse proxy / load balancer (via
`amalgame-net-proxy`). Les `hosts` rejoignent le même front TLS/SNI/ACME
que les sites. `routes` est un tableau de **tables inline** (une par
ligne), matchées par plus long `prefix` :

| Clé de route | Type | Notes |
|---|---|---|
| `prefix` | `string` | Préfixe de chemin à matcher. Défaut `"/"`. Le plus long gagne. |
| `upstream` | `string` | Backend unique, ex. `"http://127.0.0.1:8080"`. |
| `upstreams` | `[string]` | Pool load-balancé (mutuellement exclusif avec `upstream`). |
| `strategy` | `string` | Politique du pool : `round_robin` (défaut) / `ip_hash` / `least_conn`. |

Un proxy à une seule route peut omettre `routes` et mettre `upstream` /
`upstreams` (+ `strategy` optionnel) directement sur le bloc `[[proxy]]`.
Une config avec uniquement des blocs `[[proxy]]` et aucun `[[site]]` est
valide.

> **Piège TOML.** Chaque route est une table inline *sur une seule
> ligne* ; le tableau `routes = [ … ]` peut s'étaler sur plusieurs
> lignes, mais pas un `{ … }`. N'utilisez pas un array-of-tables imbriqué
> `[[proxy.route]]` — le parser TOML embarqué gère mal un array-of-tables
> imbriqué dans un autre.

## 3. Référence

Chaque section ci-dessous mappe une table TOML vers la
bibliothèque/middleware qui la consomme. La légende :

- **Lib** — la classe AM + la factory statique que le CLI Mosaic appelle.
- **Statut** — *shipped* / *planned*.

---

### `[server]` — listener + pool de workers

**Lib :** `Amalgame.Net.Http.Http1.Serve` / `Https.Serve` / `Http2.Serve` (sélectionné via `[tls].mode`).
**Statut :** *partiel* — workers et queue_size pas encore honorés (boucle mono-connexion).

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `listen` | `[string]` | `[":3000"]` | `MOSAIC_SERVER_LISTEN` | Adresses de bind. Plusieurs = multi-port. |
| `workers` | `int` | `0` (auto = 2 × CPUs) | `MOSAIC_SERVER_WORKERS` | Taille du pool de workers. *planned (nécessite amalgame-threading)* |
| `queue_size` | `int` | `0` (auto = workers × 4) | `MOSAIC_SERVER_QUEUE_SIZE` | Backlog. *planned* |
| `engine` | `string` | `"mt"` | `MOSAIC_SERVER_ENGINE` | `"mt"` = `WebApp.ServeMtWith` (défaut, multi-plateforme), `"async"` = `WebApp.ServeAsyncWith` (Linux seulement, une fibre par connexion via amalgame-async ≥ v0.2.2). *câblage planned — les apps pré-v0.13.0 doivent appeler `ServeAsyncWith` directement en attendant la clé de config.* Voir [docs/proposals/amalgame-async.md](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-async.md) pour les arbitrages (débit 1,5×–9× sur handlers I/O-bound, tient 2k connexions concurrentes là où mt s'effondre à 1k–2k sur des machines 2 cœurs) |

---

### `[tls]` — TLS / HTTPS / ACME

**Lib :** `Amalgame.Tls.TlsConfig` + `Amalgame.Tls.Acme` (runtime), `Amalgame.Web.AcmeConfig` (câblage TOML côté Mosaic, v0.10.0).
**Statut :** *partiel* — mode `acme` shippé via amalgame-tls v0.2.2 (wrapper certbot) + amalgame-web v0.10.0 (`AcmeConfig.FromMap`) ; `files` et `off` marchent. Le provisionnement SAN multi-domaine se fait encore un domaine à la fois (un EnsureCert par host).

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `mode` | `"off"\|"files"\|"acme"` | `"off"` | `MOSAIC_TLS_MODE` | Interrupteur principal. |
| `acme_email` | `string` | — | `MOSAIC_TLS_ACME_EMAIL` | Requis quand mode = `"acme"`. *shipped v0.10.0* (mappe vers `AcmeConfig.Email`). |
| `acme_cache` | `string` | `"./certs"` | `MOSAIC_TLS_ACME_CACHE` | Dossier cert/compte (mappe vers `AcmeConfig.CertDir`). *shipped v0.10.0*. |
| `domains` | `string` | `""` | `MOSAIC_TLS_DOMAINS` | Liste SAN séparée par virgules — la première entrée devient le cert-name. *shipped v0.11.1* via `AcmeConfig.Domains` + `Acme.EnsureCertMulti` (amalgame-tls v0.2.3). Jusqu'à 32 SAN par cert. |
| `acme_server` | `string` | LE production | `MOSAIC_TLS_ACME_SERVER` | URL du directory ACME. *shipped v0.10.0* (mappe vers `AcmeConfig.AcmeServer` ; var d'env honorée par `Acme.EnsureCertEx`). Buypass / ZeroSSL / LE-staging en passant l'URL du directory. |
| `certbot_path` | `string` | `"certbot"` (cherché dans `PATH`) | `MOSAIC_TLS_CERTBOT_PATH` | Override par chemin absolu. *shipped v0.10.0* (mappe vers `AcmeConfig.CertbotPath`). |
| `cert_file` | `string` | — | `MOSAIC_TLS_CERT_FILE` | Requis quand mode = `"files"`. |
| `key_file` | `string` | — | `MOSAIC_TLS_KEY_FILE` | Requis quand mode = `"files"`. |
| `min_version` | `"1.2"\|"1.3"` | `"1.3"` | `MOSAIC_TLS_MIN_VERSION` | Mappe vers `HttpServerConfig.WithTlsMinVersion` (12/13). *shipped v0.11.2* via `Amalgame.Web.TlsBindingConfig.FromMap` + amalgame-net-http v0.7.1 ; honoré jusqu'à `SSL_CTX_set_min_proto_version`. |
| `alpn` | `string` | `"h2,http/1.1"` | `MOSAIC_TLS_ALPN` | Liste ALPN (séparée par virgules). *partiel v0.11.2* — parsée et transmise à `HttpServerConfig.WithTlsAlpn`, mais le callback de sélection ALPN de net-http hardcode encore `h2` seulement (avertissement stderr d'une ligne en cas de mismatch). Le fallback http/1.1 complet arrive en amalgame-net-http v0.7.2. |

**Pattern d'usage (mode = "acme") :**

```amalgame
import Amalgame.Web         // AcmeConfig
import Amalgame.Tls         // Acme.EnsureCertEx
import Amalgame.Net.Http    // Https.Serve

let acme = AcmeConfig.FromMap(tomlAcmeSection)
let err: string = acme.Validate()
if (String_Length(err) > 0) { Console.WriteError(err); return }

if (acme.Enabled) {
    let rc: int = Acme.EnsureCertEx(
        acme.Domain, acme.Email, acme.CertDir,
        acme.AcmeServer, acme.CertbotPath)
    if (rc != 0) { Console.WriteError("ACME failed"); return }
}
Https.Serve(443, acme.CertPath(), acme.KeyPath(), handler)
```

L'appel `Acme.EnsureCertEx` reste côté user pour qu'il puisse séquencer
le provisionnement avec son propre démarrage (DB / migrations / etc.)
avant de bind le port 443. La v0.3 d'amalgame-tls remplace l'implem par
de l'ACME natif (RFC 8555) sans changer l'API d'`AcmeConfig`.

> **Note `mosaic serve` :** le binaire utilise le schéma `[tls]`
> simplifié de la [section 2b](#2b-le-schéma-de-fichier-mosaic-serve)
> (`acme` / `email` / `cert_dir`) et provisionne via `AcmeNative` ; les
> clés ci-dessus correspondent au câblage côté bibliothèque.

---

### `[sessions]` — stockage de sessions côté serveur

**Lib :** `Amalgame.Web.MemorySessionStore` / `SignedCookieSessionStore` / `RedisSessionStore` (le dernier utilise amalgame-database-nosql-redis) — backend `shm` planned (nécessite amalgame-threading).
**Statut :** *shipped* — `memory` (v0.8.1) + stratégie `signed_cookie` (v0.8.3) + backend `redis` shippent tous ; `shm` est *planned*. Depuis **v0.20.0** les trois stores implémentent une interface commune `SessionStore` et le pipeline les câble automatiquement : `WebApp.WithSession(store)` charge `ctx.Session` depuis le cookie avant le handler et le persiste (write backend + Set-Cookie) ensuite — les handlers se contentent de lire/écrire `ctx.Session`.

Le schéma a **deux dimensions orthogonales** :

- **`strategy`** — *où* vit la session :
  - `server_side` (défaut) — id dans le cookie, données sur le serveur (backend ci-dessous)
  - `encrypted_cookie` — les données SONT le cookie, signées (et à terme chiffrées)
    avec `amalgame-crypto`. Pas de stockage serveur ; `backend` est ignoré.
- **`backend`** — *quel* store côté serveur (quand `strategy = "server_side"`) :
  - `memory` (défaut) — Map en process ; mono-worker seulement
  - `shm` (planned) — mémoire partagée entre workers d'un même nœud
  - `redis` (shipped) — multi-worker, multi-nœud (`RedisSessionStore`)

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `strategy` | `"server_side"\|"encrypted_cookie"` | `"server_side"` | `MOSAIC_SESSIONS_STRATEGY` | En `encrypted_cookie`, la valeur du cookie EST le payload de session signé (pattern Flask/Rails). v0.1 est signé-seulement (visible mais inviolable) ; le chiffrement AEAD arrive avec amalgame-crypto v0.2. |
| `secret` | `string` | — | `MOSAIC_SESSIONS_SECRET` | **Requis** quand `strategy = "encrypted_cookie"`. Clé HMAC-SHA-256. À garder stable entre déploiements ; une rotation invalide tous les cookies signés. |
| `backend` | `"memory"\|"shm"\|"redis"` | `"memory"` | `MOSAIC_SESSIONS_BACKEND` | Ignoré quand `strategy = "encrypted_cookie"`. `memory` + `redis` shippés ; `shm` planned. |
| `dir` | `string` | `"./data/sessions"` | `MOSAIC_SESSIONS_DIR` | Pour un futur backend `file` (hors du triplet 3-tiers). |
| `url` | `string` | — | `MOSAIC_SESSIONS_URL` | Pour `redis` (`redis://host:port/db`). |
| `max_age_sec` | `int` | `86400` | `MOSAIC_SESSIONS_MAX_AGE_SEC` | TTL par session (stores serveur). |
| `cookie_name` | `string` | `"mosaic_session"` | `MOSAIC_SESSIONS_COOKIE_NAME` | *shipped v0.8.1* (memory) / *v0.8.3* (signed_cookie). |
| `cookie_secure` | `bool` | `true` (quand TLS actif) | `MOSAIC_SESSIONS_COOKIE_SECURE` | *shipped v0.8.1/v0.8.3*. |
| `cookie_samesite` | `"Strict"\|"Lax"\|"None"` | `"Lax"` | `MOSAIC_SESSIONS_COOKIE_SAMESITE` | *shipped v0.8.1/v0.8.3*. |
| `cookie_path` | `string` | `"/"` | `MOSAIC_SESSIONS_COOKIE_PATH` | *shipped v0.8.1/v0.8.3*. |
| `cookie_max_age` | `int` | `0` | `MOSAIC_SESSIONS_COOKIE_MAX_AGE` | `Max-Age=` du Set-Cookie. 0 = cookie de session. *shipped v0.8.1/v0.8.3*. |

**Précautions SignedCookieSessionStore (v0.8.3, signé-seulement) :**
- Les données sont visibles par quiconque a le cookie (inviolable mais PAS confidentiel). N'y stockez pas de secrets / identifiants.
- Clés + valeurs ne doivent pas contenir `&`, `=`, ou `.` (pas d'échappement en v0.1 — payload JSON en v0.2).
- Limite de 4 Kio par cookie ≈ ~30 paires key=value typiques.
- Pour des données sensibles : passez en `strategy = "server_side"` avec `backend = "redis"`, ou attendez la variante chiffrée AEAD (dépend d'amalgame-crypto v0.2).

---

### `[security.headers]` — durcissement côté réponse

**Lib :** `Amalgame.Web.SecurityHeaders.FromMap(...)` (depuis v0.4.1).
**Statut :** *shipped* (v0.4.1).

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `preset` | `"strict_html"\|"strict_api"` | — | `MOSAIC_SECURITY_HEADERS_PRESET` | Point de départ ; les clés suivantes surchargent les champs individuels. |
| `csp` | `string` | — | `MOSAIC_SECURITY_HEADERS_CSP` | Valeur Content-Security-Policy complète. |
| `frame_options` | `"DENY"\|"SAMEORIGIN"` | — | `MOSAIC_SECURITY_HEADERS_FRAME_OPTIONS` | X-Frame-Options. |
| `content_type_options` | `bool` | `false` | `MOSAIC_SECURITY_HEADERS_CONTENT_TYPE_OPTIONS` | True → `X-Content-Type-Options: nosniff`. |
| `referrer_policy` | `string` | — | `MOSAIC_SECURITY_HEADERS_REFERRER_POLICY` | |
| `permissions_policy` | `string` | — | `MOSAIC_SECURITY_HEADERS_PERMISSIONS_POLICY` | |
| `coop` | `string` | — | `MOSAIC_SECURITY_HEADERS_COOP` | Cross-Origin-Opener-Policy. |
| `coep` | `string` | — | `MOSAIC_SECURITY_HEADERS_COEP` | Cross-Origin-Embedder-Policy. |
| `hsts` | `string` | — | `MOSAIC_SECURITY_HEADERS_HSTS` | Valeur pré-composée ; prend le pas sur les composants ci-dessous. |
| `hsts_max_age` | `int` | — | `MOSAIC_SECURITY_HEADERS_HSTS_MAX_AGE` | Secondes. |
| `hsts_include_subdomains` | `bool` | `false` | `MOSAIC_SECURITY_HEADERS_HSTS_INCLUDE_SUBDOMAINS` | |
| `hsts_preload` | `bool` | `false` | `MOSAIC_SECURITY_HEADERS_HSTS_PRELOAD` | |

HSTS est volontairement non posé par tous les presets — épingler HSTS
sur une réponse servie en HTTP peut verrouiller les utilisateurs hors du
site. Ne réglez `hsts_max_age` que lorsque le TLS est obligatoire.

Les clés inconnues sont ignorées (compat ascendante avec de futurs champs).

---

### `[security.cors]` — Cross-Origin Resource Sharing

**Lib :** `Amalgame.Web.Cors.FromMap(...)` + `WebApp.WithCors(...)`.
**Statut :** *shipped* — `Cors.FromMap` compose les clés ci-dessous (couvert par `cors_test`) ; le pipeline gère le preflight OPTIONS + estampille `Access-Control-*` sur les réponses.

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `preset` | `"disabled"\|"allow_all"\|"strict"` | `"disabled"` | `MOSAIC_SECURITY_CORS_PRESET` | |
| `allowed_origins` | `[string]` | `[]` | `MOSAIC_SECURITY_CORS_ALLOWED_ORIGINS` | Origines exactes ; `["*"]` = wildcard (incompatible avec credentials). |
| `allowed_methods` | `[string]` | `["GET","POST","PUT","PATCH","DELETE","OPTIONS"]` | `MOSAIC_SECURITY_CORS_ALLOWED_METHODS` | |
| `allowed_headers` | `[string]` | `["Content-Type","Authorization"]` | `MOSAIC_SECURITY_CORS_ALLOWED_HEADERS` | Access-Control-Allow-Headers du preflight. |
| `exposed_headers` | `[string]` | `[]` | `MOSAIC_SECURITY_CORS_EXPOSED_HEADERS` | Access-Control-Expose-Headers. |
| `allow_credentials` | `bool` | `false` | `MOSAIC_SECURITY_CORS_ALLOW_CREDENTIALS` | Cookies / Authorization en cross-origin. |
| `max_age_sec` | `int` | `86400` | `MOSAIC_SECURITY_CORS_MAX_AGE_SEC` | TTL du cache de preflight. |

---

### `[security.csrf]` — validation de token CSRF

**Lib :** `Amalgame.Web.Csrf.FromMap(...)` (v0.7.0).
**Statut :** *shipped* — pattern double-submit cookie, entropie de token 256 bits via amalgame-random, chemins exemptés matchés par préfixe.

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `enabled` | `bool` | *omis = activé* | `MOSAIC_SECURITY_CSRF_ENABLED` | Mettre à `false` pour renvoyer `Csrf.Disabled()` quelles que soient les autres clés. |
| `cookie_name` | `string` | `"csrf_token"` | `MOSAIC_SECURITY_CSRF_COOKIE_NAME` | Nom du cookie double-submit. |
| `header_name` | `string` | `"X-CSRF-Token"` | `MOSAIC_SECURITY_CSRF_HEADER_NAME` | Où la SPA réémet la valeur du cookie. |
| `token_bytes` | `int` | `32` | `MOSAIC_SECURITY_CSRF_TOKEN_BYTES` | Octets d'entropie (32 = token 256 bits, encodé hex → 64 chars). |
| `safe_methods` | `[string]` | `["GET","HEAD","OPTIONS"]` | `MOSAIC_SECURITY_CSRF_SAFE_METHODS` | Séparé par virgules ; ces méthodes contournent Validate. |
| `exempt_paths` | `[string]` | `[]` | `MOSAIC_SECURITY_CSRF_EXEMPT_PATHS` | Séparé par virgules ; matché par préfixe contre `req.Path`. |
| `cookie_path` | `string` | `"/"` | `MOSAIC_SECURITY_CSRF_COOKIE_PATH` | `Path=` du Set-Cookie. |
| `cookie_secure` | `bool` | `true` | `MOSAIC_SECURITY_CSRF_COOKIE_SECURE` | Flag `Secure` du Set-Cookie (mettre `false` pour du dev HTTP). |
| `cookie_samesite` | `"Strict"\|"Lax"\|"None"` | `"Lax"` | `MOSAIC_SECURITY_CSRF_COOKIE_SAMESITE` | `SameSite=` du Set-Cookie. |
| `cookie_max_age` | `int` | `0` | `MOSAIC_SECURITY_CSRF_COOKIE_MAX_AGE` | Secondes ; `0` = cookie de session (effacé à la fermeture du navigateur). |

**Note d'arbitrage :** le cookie CSRF n'est volontairement PAS `HttpOnly` —
la SPA doit le lire en JS pour le réémettre dans l'en-tête de requête
`X-CSRF-Token`. Compensé par :
- les défauts `Secure` + `SameSite=Lax` (cookie envoyé seulement sur TLS + seulement sur navigations top-level ou requêtes same-site)
- le token n'a aucune signification sémantique au-delà de son aléa — ce n'est PAS un bearer de session
- 256 bits d'entropie depuis `/dev/urandom` / `BCryptGenRandom`

**Flux form-only** (post de formulaire HTML sans JS) : prévu pour v0.7.x.
Aujourd'hui la validation lit seulement l'en-tête configuré ; la config
`body_key` acceptera un nom de champ de formulaire en fallback.

**Dépend de :** [`amalgame-random`](https://github.com/amalgame-lang/amalgame-random) pour de l'entropie cryptographique (`Random.SystemBytes` via `/dev/urandom` / `BCryptGenRandom`).

---

### `[security.auth]` — authentification Basic / JWT

**Lib :** `Amalgame.Web.BasicAuth` / `JwtAuth` + `WebApp.WithBasicAuth(...)` / `WithJwt(...)` + le groupe de routes `Protected()`.
**Statut :** *shipped (v0.19.0)* — **câblé en code, pas une table TOML.** L'auth n'a pas de `FromMap` : un vérificateur `BasicAuth` est une closure `(user, pwd) -> bool` (elle mappe vers *votre* store d'identifiants — env, DB, comparaison de hash), ce que TOML ne peut exprimer. Seuls les *secrets* relèvent de la config (env), lus en code.

```amalgame
// secrets depuis l'environnement ; logique en code
let app = WebApp.New()
    .WithJwt(new JwtAuth(Env_Get("JWT_SECRET")))                // Bearer HS256
    .WithBasicAuth(new BasicAuth("Admin")
        .WithVerifier((u, p) => u == "admin" && p == Env_Get("ADMIN_PASSWORD")))
    .Get("/", home)                                             // public
app.Protected()                                                 // groupe protégé :
    .Get("/admin", dashboard)                                   //   401 sauf si un
    .Post("/api/x", create)                                     //   schéma configuré vérifie
```

- Les routes enregistrées via `Protected()` portent `Route.Protected = true` ; l'auth tourne **après le match / avant le handler**, donc les routes publiques et les 404 ne paient rien.
- Une route protégée accepte la requête si **au moins un** schéma configuré vérifie, et **échoue fermé (401)** si aucun n'est configuré.
- `OAuth2Client` (flux login authorization-code, presets Github/Google) est câblé comme des handlers de route ordinaires (`StartLogin` / `HandleCallback`), pas comme un middleware de pipeline.
- **DOIT tourner en TLS** — Basic met le mot de passe et Bearer le token sur chaque requête. À coupler avec `WebApp.ServeHttps` / `ServeHttpsMt`.

---

### `[security.rate_limit]` — throttling par IP / par clé

**Lib :** `Amalgame.Web.RateLimit.FromMap(...)` (v0.6.0).
**Statut :** *shipped* — algorithme fenêtre fixe, store mémoire en process, clé par IP. Fenêtre glissante + backend Redis prévus pour v2.

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `enabled` | `bool` | *omis = activé* | `MOSAIC_SECURITY_RATE_LIMIT_ENABLED` | Mettre à `false` pour renvoyer `Disabled()` quelles que soient les autres clés. |
| `preset` | `"per_ip"\|"disabled"` | — | `MOSAIC_SECURITY_RATE_LIMIT_PRESET` | Point de départ ; les clés explicites surchargent ensuite. |
| `rps` | `int` | — | `MOSAIC_SECURITY_RATE_LIMIT_RPS` | Raccourci : `max_requests=N, window_sec=1`. |
| `max_requests` | `int` | `0` | `MOSAIC_SECURITY_RATE_LIMIT_MAX_REQUESTS` | Requêtes par fenêtre. `0` = pas de throttle. |
| `window_sec` | `int` | `1` | `MOSAIC_SECURITY_RATE_LIMIT_WINDOW_SEC` | Longueur de la fenêtre en secondes. |
| `key_strategy` | `"ip"` | `"ip"` | `MOSAIC_SECURITY_RATE_LIMIT_KEY_STRATEGY` | Seul `"ip"` supporté aujourd'hui (retire `:port` de `RemoteAddr`). `"user"`/`"custom"` *planned v2*. |
| `trusted_proxies` | `[string]` | `[]` | `MOSAIC_SECURITY_RATE_LIMIT_TRUSTED_PROXIES` | CIDR dont le `X-Forwarded-For` est de confiance. *planned v2*. |
| `backend` | `"memory"\|"redis"` | `"memory"` | `MOSAIC_SECURITY_RATE_LIMIT_BACKEND` | Redis *planned v2*. |
| `redis_url` | `string` | — | `MOSAIC_SECURITY_RATE_LIMIT_REDIS_URL` | *planned v2*. |

**Réserve sur l'algorithme :** le compteur fenêtre-fixe v1 peut
brièvement autoriser jusqu'à `2 × max_requests` à cheval sur une
frontière de fenêtre (un burst qui chevauche deux fenêtres). Acceptable
pour la plupart des apps ; passez en fenêtre-glissante / token-bucket en
v2 si vous avez besoin de garanties strictes.

**Dépend de :** [`amalgame-datetime`](https://github.com/amalgame-lang/amalgame-datetime) pour l'horloge monotone (`DateTime.NowMonotonicNanos()`).

---

### `[logging]` — access log + log runtime structuré

**Lib :** `Amalgame.Web.LogConfig.FromMap(...)` + `WebApp.WithLogging(...)`.
**Statut :** *shipped* — `WithLogging` applique le niveau + le sink fichier sur la façade amalgame-logging et émet une ligne d'access-log par requête sur chaque chemin (hit statique, rejet middleware, route, 404), enrichie en v0.18.0 avec host + statut + durée + IP client.

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `level` | `"trace"\|"debug"\|"info"\|"warn"\|"error"` | `"info"` | `MOSAIC_LOGGING_LEVEL` | |
| `format` | `"json"\|"text"` | `"text"` | `MOSAIC_LOGGING_FORMAT` | JSON pour la prod (greppable). |
| `access_log` | `bool` | `true` | `MOSAIC_LOGGING_ACCESS_LOG` | Une ligne par requête. |
| `request_id_header` | `string` | `"X-Request-Id"` | `MOSAIC_LOGGING_REQUEST_ID_HEADER` | Propagé aux logs + en aval. |

---

### `[limits]` — plafonds de ressources côté serveur

**Lib :** `Amalgame.Net.Http.HttpServerConfig` (struct C + builders + getters) — alimenté par le CLI Mosaic vers `Http1.ServeWith(port, config, handler)` / `Http2.ServeWith` / `Https.ServeWith` / `Ws.ServeWith` / `Wss.ServeWith`.
**Statut :** *shipped end-to-end* — timeouts Slowloris en v0.4.3 (Http1) / v0.4.4 (toutes variantes) ; câblage de la limite de taille du parser H1 en v0.4.5. `idle_timeout_sec` et `listen_backlog` restent planned (keep-alive + refactor listen() respectivement).

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `header_timeout_sec` | `int` | `0` (off) | `MOSAIC_LIMITS_HEADER_TIMEOUT_SEC` | **shipped v0.4.3** (Http1) / **v0.4.4** (Http2/Https/Ws/Wss) — `SO_RCVTIMEO` sur la connexion acceptée. Garde anti-Slowloris. |
| `body_timeout_sec` | `int` | `0` (off) | `MOSAIC_LIMITS_BODY_TIMEOUT_SEC` | **shipped v0.4.3/4** — appliqué avec `header_timeout_sec` (le plus grand des deux sert de deadline de phase unique ; v0.4.6 les séparera). |
| `max_body_bytes` | `int` | `8388608` (8 Mio) | `MOSAIC_LIMITS_MAX_BODY_BYTES` | **shipped v0.4.5** (H1) — le parse échoue (-1, conn fermée) si Content-Length dépasse la limite. Enforcement de taille H2 en attente. |
| `max_header_bytes` | `int` | `65536` (64 Kio) | `MOSAIC_LIMITS_MAX_HEADER_BYTES` | **shipped v0.4.5** (H1) — le parse échoue si la taille totale du bloc d'en-têtes dépasse ça. |
| `max_url_bytes` | `int` | implicite (borné par le buffer recv) | `MOSAIC_LIMITS_MAX_URL_BYTES` | **shipped v0.4.5** (H1) — le parse échoue si la longueur de la cible de requête dépasse ça. |
| `idle_timeout_sec` | `int` | `0` | `MOSAIC_LIMITS_IDLE_TIMEOUT_SEC` | *planned* — nécessite le keep-alive HTTP (actuellement `Connection: close` seulement). |
| `listen_backlog` | `int` | `64` | `MOSAIC_LIMITS_LISTEN_BACKLOG` | *planned* — nécessite que `H1Server_Listen` transmette la valeur à `listen(2)`. |

**Réserve pour Ws / Wss `header_timeout_sec` :** `SO_RCVTIMEO` persiste
pour toute la durée de vie de la connexion, ce qui casse les boucles de
frames WebSocket longue durée. Les handlers qui prévoient de longues
attentes idle devraient effacer/relever le timeout eux-mêmes (ou attendre
l'auto-effacement post-upgrade de v0.4.6). Pour des serveurs HTTP purs
c'est le bon comportement.

**Invariant toujours actif (non configurable) :** `HttpResponse.Header(name, value)`
supprime silencieusement toute valeur contenant CR (`\r`) ou LF (`\n`) —
prévention du response-splitting HTTP. Aucun opt-out de config : aucune
valeur d'en-tête légitime ne contient CR/LF, et le coût de le laisser
actif est un vecteur d'injection de classe CVE. Les power-users qui ont
besoin d'une valeur verbatim (fixtures de test, builders internes de
confiance) peuvent appeler `HeaderUnsafe(name, value)` par réponse.
Shippé dans `amalgame-net-http v0.4.2`.

---

### `[powered_by]` — affichage de l'identité du serveur

**Lib :** `Amalgame.Web.PoweredBy.FromMap(...)` + `WebApp.WithPoweredBy(...)` / `WithoutPoweredBy()` (depuis v0.21.0).
**Statut :** *shipped (v0.21.0)*.

Estampille deux en-têtes d'identité serveur sur **chaque** réponse :

```
X-Powered-By: Mosaic (Amalgame)    (convention framework — style Express)
Server:       Mosaic (Amalgame)    (convention serveur — style nginx/apache)
```

**C'est la seule feature activée par défaut** — un simple `WebApp.New()`
annonce déjà la stack (promotion passive et gratuite). Tous les autres
middlewares ici sont opt-in ; celui-ci on en opt *out*. Comme
`X-Powered-By` est du fingerprinting léger (OWASP / helmet recommandent
de le retirer), mettez `enabled = false` si vous préférez ne pas
divulguer la stack aux clients.

| Clé | Type | Défaut | Env | Notes |
|---|---|---|---|---|
| `enabled` | `bool` | `true` | `MOSAIC_POWERED_BY_ENABLED` | Interrupteur principal. `false` → n'émet rien (équivalent à `WithoutPoweredBy()`). |
| `value` | `string` | `"Mosaic (Amalgame)"` | `MOSAIC_POWERED_BY_VALUE` | Identité annoncée pour les deux en-têtes. |
| `x_powered_by` | `bool` | `true` | `MOSAIC_POWERED_BY_X_POWERED_BY` | Bascule l'en-tête `X-Powered-By` seul. |
| `server` | `bool` | `true` | `MOSAIC_POWERED_BY_SERVER` | Bascule l'en-tête `Server` seul. |

Comme `SecurityHeaders`, `Apply` n'écrase jamais un en-tête déjà posé par
le handler — un handler qui pose son propre `Server:` gagne. Les clés
inconnues sont ignorées (compat ascendante).

---

## 4. Tourner en service

Un `mosaic serve` au premier plan meurt avec ta session SSH et ne
revient pas après un reboot. `mosaic service` l'enregistre auprès du
gestionnaire de services natif de l'hôte — sans fichier unit écrit à la
main :

```bash
cd /srv/mon-site           # le dossier qui contient mosaic.toml
sudo mosaic service install        # génère l'unit + enable + start
mosaic service status
mosaic service logs -f
```

`install` enveloppe `mosaic serve <config>` (défaut `./mosaic.toml`) et
épingle le répertoire de travail de l'unit sur celui du fichier de
config : les chemins relatifs du `mosaic.toml` (`cert_dir`, le `root`
d'un site, …) résolvent donc exactement comme au premier plan.

### Actions

| Action | Effet |
|---|---|
| `install [CONFIG]` | Génère l'unit pour `mosaic serve CONFIG`, puis enable + start. |
| `uninstall` | Stoppe, désactive et supprime l'unit. |
| `start` / `stop` / `restart` | Contrôle du cycle de vie. |
| `status` | État du service (pas besoin de root). |
| `logs [-f]` | Suit les logs du service (`-f` = follow ; pas de root). |

### Drapeaux

| Drapeau | Sens |
|---|---|
| `--name NAME` | Nom du service. Défaut : `mosaic-<basename-du-dossier-config>`. |
| `--user USER` | Compte OS sous lequel le service tourne (`User=` systemd/launchd). Défaut : l'utilisateur appelant. |
| `--user-scope` | Installe un service *par utilisateur* (systemd `--user`, `~/.config/systemd/user/`) au lieu d'un service système — pas de root, mais pas de démarrage au boot sans lingering. |
| `--no-enable` | Enregistre sans activer au boot. |
| `--no-start` | Enregistre sans démarrer maintenant. |

`--user` (tourner *en tant que* tel compte) et `--user-scope` (systemd
par utilisateur) sont deux réglages différents — à ne pas confondre.

### Plateformes

Le scope système est le défaut ; l'enregistrement écrit dans des chemins
système, donc `install`/`uninstall`/`start`/`stop` exigent root (`sudo`,
ou Administrateur sous Windows). `status`/`logs` non.

| OS | Backend | Unit écrite |
|---|---|---|
| Linux | systemd | `/etc/systemd/system/<name>.service` |
| macOS | launchd | `/Library/LaunchDaemons/<name>.plist` |
| Windows | SCM | `sc create <name> binPath= "mosaic serve <config>"` (depuis Git Bash / MSYS2) |

L'unit systemd reçoit `AmbientCapabilities=CAP_NET_BIND_SERVICE` (bind
`:80`/`:443` sans tourner en root complet) et un arrêt
`SIGTERM`/`TimeoutStopSec` sur lequel le serveur s'éteint proprement.
Sous Windows, le binaire servi se connecte lui-même au dispatcher SCM
(via `amalgame-service`), donc `sc stop` est propre.

> Linux/systemd est le chemin principal, testé end-to-end en CI. launchd
> et le SCM Windows sont supportés mais moins éprouvés — teste-les en
> staging avant de t'y fier.

Le renouvellement des certificats n'est pas changé par le passage en
service : il suit toujours le motif restart-to-renew — couple-le au timer
de vérification des certs de [Hébergement → Renouvellement des
certificats](../16-hosting.fr.md). Pour un binaire `./server` custom (pas
`mosaic serve`), écris l'unit à la main comme montré là-bas ; `mosaic
service` ne pilote que le `mosaic serve` config-driven.

## 5. Composer la config en code (sans `mosaic.toml`)

Les apps qui n'utilisent pas le CLI `mosaic` peuvent construire la même
config en AM pur — en contournant entièrement TOML, variables d'env et
flags. Chaque feature expose à la fois une API builder et
`FromMap(Map<string, string>)` :

```amalgame
// Builder direct — pas de TOML
let sec = SecurityHeaders.StrictHtml()
    .WithHsts(31536000, true, false)
    .WithCoep("require-corp")

// Via FromMap — ce que le CLI Mosaic appelle après aplatissement du TOML
let cfg = new Map<string, string>()
cfg.Set("preset", "strict_html")
cfg.Set("hsts_max_age", "31536000")
cfg.Set("hsts_include_subdomains", "true")
let sec2 = SecurityHeaders.FromMap(cfg)
```

La bibliothèque ne connaît rien à TOML — le CLI aplatit la table TOML et
appelle `FromMap`. Cela garde `amalgame-web`, `amalgame-tls`,
`amalgame-net-http` découplés de tout format de config particulier.

## 6. Versionnage du schéma de config

Ce fichier EST le schéma. Quand une clé est ajoutée, retirée ou renommée,
l'entrée ici DOIT être mise à jour dans la même PR. Le CLI `mosaic`
traite les clés inconnues en avertissement souple (compat ascendante)
mais ne produit pas d'erreur de validation — le `FromMap` consommateur
les ignore. Les apps ont un lint `mosaic config --check` qui grep-valide
les clés connues.

## 7. Renvois

- Rationnel de conception + squelette de sections : [`proposals/amalgame-web.md` §18](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-web.md)
- Roadmap sécurité : [`proposals/amalgame-web.md` §21.2 (Phase 1)](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-web.md)
- Notes de stratégie de configuration : mémoire agent `project_mosaic_config_strategy`
