# Framework web (Mosaic)

> **Portée.** Ce chapitre couvre **Mosaic**, le framework web côté
> serveur livré sous forme du package `amalgame-web` (routage HTTP,
> middleware, sessions, fichiers statiques, HTTPS). Il ne s'agit *pas*
> de [`amalgame-ui-web`](09-ui-web/README.fr.md), qui est un binding
> d'interface graphique de bureau au-dessus d'une webview de l'OS.
> Package différent, cas d'usage différent.
>
> Tout ce qui suit provient de `amalgame-web v0.17.1` et de ses
> dépendances telles que livrées. Les signatures de méthodes sont citées
> depuis les facades du package.

Mosaic est un framework web petit et explicite : vous créez un `WebApp`,
attachez des handlers de route sous forme de closures, les enveloppez
éventuellement dans du middleware (en-têtes de sécurité, CORS, CSRF,
limitation de débit, sessions, fichiers statiques), puis démarrez un
serveur. Pas d'état global, pas de magie par annotation, pas de
réflexion cachée — un handler n'est qu'une `Closure<WebContext, HttpResponse>`.

---

## Installation

Mosaic est un package externe. Ajoutez-le (ainsi que la couche HTTP sur
laquelle il s'appuie) à votre projet :

```bash
amc package add web
```

`amalgame-web v0.17.1` tire notamment :

- `amalgame-net-http >= 0.11.1` — requête/réponse HTTP/1.1 (**0.11.1 est
  un plancher obligatoire** : avant cette version, les en-têtes de
  réponse personnalisés — `Set-Cookie`, CSP, CORS — étaient
  silencieusement perdus sur le réseau).
- `amalgame-tls >= 0.3.1` — terminaison TLS + ACME, pour HTTPS.
- `amalgame-crypto`, `amalgame-random`, `amalgame-datetime`,
  `amalgame-logging` — utilisés respectivement par les sessions à cookie
  signé, l'entropie CSRF, l'horloge de limitation de débit et les logs
  d'accès.

Nécessite le compilateur `amc` `>= 0.8.58`.

---

## Bonjour, serveur

```amalgame
import Amalgame.Collections
import Amalgame.String
import Amalgame.Net.Http
import Amalgame.Web

public class Program {
    public static void Main(string[] args) {
        let app = new WebApp()

        app.Get("/", ctx => HttpResponse.New().Text("Hello from Mosaic"))

        app.Serve(8080)
    }
}
```

`WebApp.Serve(port)` exécute un serveur sériel, traitant une seule
connexion à la fois — idéal pour le développement. Il est bloquant. Les
quatre imports forment l'ensemble standard : `Collections` et `String`
pour les types de la stdlib que vos handlers manipulent, `Net.Http` pour
`HttpResponse`, et `Web` pour `WebApp`.

> `new WebApp()` et `WebApp.New()` sont équivalents — la méthode statique
> `New()` ne fait qu'appeler le constructeur. Utilisez celle qui se lit
> le mieux ; ce chapitre utilise `new WebApp()`.

---

## Routage

Enregistrez un handler par méthode HTTP. Chacun retourne le `WebApp`, ce
qui permet de chaîner les appels :

```amalgame
public WebApp Get(string path, Closure<WebContext, HttpResponse> handler)
public WebApp Post(string path, Closure<WebContext, HttpResponse> handler)
public WebApp Put(string path, Closure<WebContext, HttpResponse> handler)
public WebApp Patch(string path, Closure<WebContext, HttpResponse> handler)
public WebApp Delete(string path, Closure<WebContext, HttpResponse> handler)
```

Les chemins prennent en charge deux formes de capture :

- `:name` — un segment unique. `/users/:id` correspond à `/users/42`, et
  capture `id = "42"`.
- `*name` — une fin de chemin (wildcard). `/files/*path` correspond à
  `/files/css/app.css`, capturant `path = "css/app.css"` (slashes
  inclus).

> **L'ordre compte.** Les routes sont mises en correspondance dans leur
> ordre d'enregistrement, premier inscrit servi en premier. Enregistrez
> les chemins statiques avant les chemins paramétrés — placez
> `/users/me` avant `/users/:id`, sinon la route `:id` avalera `me`.

```amalgame
app.Get("/users/:id", ctx => {
    let id: string = ctx.Param("id")
    return HttpResponse.New().Json("{\"id\":\"" + id + "\"}")
})
```

---

## Lire la requête — `WebContext`

Le handler reçoit un `WebContext`. Les champs et méthodes que vous
utiliserez :

| Membre | Type | Ce qu'il fournit |
|--------|------|-------------------|
| `ctx.Param(name)` | méthode | un segment de chemin `:name` |
| `ctx.QueryParam(name)` | méthode | une valeur de requête `?name=…` |
| `ctx.Query` | champ `Map<string,string>` | tous les paramètres de requête |
| `ctx.Body` | champ `string` | corps brut de la requête (texte) |
| `ctx.Method` | champ `string` | `"GET"`, `"POST"`, … |
| `ctx.Path` | champ `string` | le chemin sans la chaîne de requête |
| `ctx.State` | champ `Map<string,string>` | état partagé au niveau de l'app (lecture) |
| `ctx.Session` | champ `Session` | la session, ou null s'il n'y en a aucune |

```amalgame
app.Post("/echo", ctx => {
    let body: string = ctx.Body
    return HttpResponse.New().Status(201).Text(body)
})
```

> Le corps de la requête est du **texte** en v0.17.1. Les uploads
> binaires (une API en streaming) ne sont pas encore livrés — ne les
> documentez pas et ne comptez pas dessus.

---

## Construire la réponse — `HttpResponse`

`HttpResponse` (issu de `amalgame-net-http`) est un builder chaînable :

```amalgame
public HttpResponse Status(int code)              // 200 par défaut
public HttpResponse Text(string s)                // Content-Type: text/plain
public HttpResponse Html(string s)                // Content-Type: text/html
public HttpResponse Json(string jsonText)         // Content-Type: application/json
public HttpResponse Header(string name, string value)   // ajoute/remplace (sûr vis-à-vis CR/LF)
public HttpResponse Redirect(string url, bool permanent) // 302, ou 308 si permanent
public HttpResponse SetCookie(Cookie cookie)      // en-tête Set-Cookie
public HttpResponse File(string path)             // corps de fichier sûr pour le binaire
```

```amalgame
app.Get("/teapot", ctx =>
    HttpResponse.New()
        .Status(418)
        .Header("X-Powered-By", "Mosaic")
        .Json("{\"short_and_stout\":true}"))
```

`Json` prend une chaîne — Mosaic n'impose pas d'encodeur JSON, donc
construisez le payload avec la stdlib `Amalgame.Formats.Json` (voir
[04-stdlib.fr.md](04-stdlib.fr.md)) ou assemblez de petits littéraux par
concaténation de chaînes comme ci-dessus.

---

## Middleware

Le middleware est attaché à l'app via des builders `With…` et s'exécute
autour de chaque handler. Toutes les pièces livrées :

```amalgame
let app = new WebApp()
    .WithSecurityHeaders(SecurityHeaders.StrictApi())
    .WithCors(Cors.AllowAll())          // dev uniquement — origine wildcard
    .WithCsrf(Csrf.Default())
    .WithRateLimit(RateLimit.PerIp(100, 60))   // 100 req / 60 s / IP
    .WithStatic(Static.New("/assets", "./public"))
    .WithLogging(new LogConfig().WithAccessLog(true))
    .WithState("appName", "demo")
```

### En-têtes de sécurité

`SecurityHeaders.StrictHtml()` fournit une Content-Security-Policy
stricte, `X-Frame-Options: DENY`, `nosniff`, etc. — pour les
applications HTML. `SecurityHeaders.StrictApi()` en est la variante pour
API JSON (`nosniff` + politique de referrer, sans règles de framing).
Affinez avec `.WithHsts(...)`, `.WithCsp(...)`, `.WithFrameOptions(...)`
et consorts. Les en-têtes propres au handler l'emportent sur les valeurs
par défaut.

### CORS

`Cors.Strict()` (aucune origine tant que vous n'en ajoutez pas),
`Cors.AllowAll()` (wildcard — **développement uniquement**), ou
`Cors.Disabled()`. Ajustez avec `.WithAllowedOrigins(list)`,
`.WithAllowedMethods(list)`, `.WithAllowCredentials(bool)`,
`.WithMaxAge(seconds)`.

### CSRF

`Csrf.Default()` est une protection par double-submit-cookie : un token
de 256 bits, `Secure` + `SameSite=Lax`, validé sur les méthodes non
sûres (POST/PUT/…) et ignoré sur GET/HEAD/OPTIONS. `Csrf.Disabled()` le
désactive.

### Limitation de débit

`RateLimit.PerIp(maxRequests, windowSec)` — une fenêtre fixe par IP
cliente, renvoyant `429` avec `Retry-After` en cas de dépassement.
`.WithBackend("redis")` + `.WithRedisHost(...)` partage la limite entre
instances ; le backend par défaut `"memory"` est par processus.

---

## Sessions

Trois stores sont livrés, exposant tous la même `Session`
(`Set`/`Get`/`Has`/`Delete`/`Clear`). Choisissez selon votre topologie
de déploiement :

### En mémoire (dev / processus unique)

```amalgame
let store = new MemorySessionStore()
let s: Session = store.Create("session-id-here")
s.Set("user", "alice")
let who: string = s.Get("user")     // "alice"
let cookie: Cookie = store.MakeCookie("session-id-here")  // pour resp.SetCookie(...)
```

Perdu au redémarrage, machine unique uniquement. Thread-safe sous
`ServeMt`.

### Cookie signé (sans état, scale horizontalement)

```amalgame
let store = new SignedCookieSessionStore("a-long-random-secret")
    .WithEncrypted(true)            // AES-256-GCM ; false par défaut = HMAC seul
let value: string = store.Encode(s) // la valeur du cookie
let back: Session  = store.Decode(value)
```

Le cookie *est* la session — aucun stockage serveur. La version
signée-seule est infalsifiable mais lisible par le client ;
`.WithEncrypted(true)` la rend également confidentielle. (Limitation
v0.1 : les clés/valeurs ne peuvent pas encore contenir `&`, `=` ou `.`
— pas d'échappement. Convient pour des ids et des flags.)

### Redis (distribué, persistant)

```amalgame
let store = new RedisSessionStore()
    .WithHost("127.0.0.1").WithPort(6379)
    .WithKeyPrefix("sess:").WithMaxAgeSec(3600)
let s: Session = store.Create("session-id-here")
store.Save(s)                        // persiste les mutations
```

Nécessite un Redis accessible (`>= 5.0`).

---

## Fichiers statiques

```amalgame
app.WithStatic(Static.New("/assets", "./public").WithCacheMaxAge(3600))
```

Sert `./public/app.css` à l'URL `/assets/app.css`. La traversée de
répertoire est bloquée, les répertoires renvoient `403` (pas d'auto-index),
les chemins absents retombent sur le routeur, et `If-None-Match` est
honoré (`304`). Le type MIME est déterminé par l'extension. Les requêtes
Range ne sont pas encore prises en charge (le fichier entier est
envoyé) ; restez donc sur des assets de taille raisonnable.

Enregistrez les préfixes spécifiques avant les préfixes généraux s'ils
se recouvrent.

---

## Servir : HTTP et HTTPS

Mosaic livre plusieurs points d'entrée serveur. Chacun prend un port et
bloque :

```amalgame
public int Serve(int port)        // sériel, une connexion à la fois — dev
public int ServeMt(int port)      // un thread OS par connexion
public int ServeAsync(int port)   // un thread, N fibers (Linux/epoll uniquement)
```

Chacun dispose d'une variante `…With(port, cfg)` prenant un
`AmalgameNetHttpServerConfig` — utilisez-la pour activer le keep-alive
HTTP/1.1 via `.WithIdleTimeoutSec(seconds)`. (Les formes simples
`Serve`/`ServeMt` traitent une requête par connexion.)

Pour TLS, terminez la connexion en interne (livré à partir de la
v0.14.0) :

```amalgame
public int ServeHttps(int port, string certPath, string keyPath)
public int ServeHttpsMt(int port, string certPath, string keyPath)
```

Ces points d'entrée nécessitent OpenSSL 3.x (ou LibreSSL) sur le
système. Pour des certificats Let's Encrypt automatiques, associez-les à
la prise en charge ACME de `amalgame-tls`.

---

## Un exemple complet

Paramètres de chemin, une route JSON, un corps POST et des en-têtes de
sécurité d'API — voici un programme autonome unique :

```amalgame
import Amalgame.Collections
import Amalgame.String
import Amalgame.Net.Http
import Amalgame.Web

public class Program {
    public static void Main(string[] args) {
        let app = new WebApp()

        app.Get("/", ctx => HttpResponse.New().Text("Hello from Mosaic"))

        app.Get("/users/:id", ctx => {
            let id: string = ctx.Param("id")
            return HttpResponse.New().Json("{\"id\":\"" + id + "\"}")
        })

        app.Post("/echo", ctx => {
            let body: string = ctx.Body
            return HttpResponse.New().Status(201).Text(body)
        })

        app.WithSecurityHeaders(SecurityHeaders.StrictApi())
        app.WithCors(Cors.AllowAll())

        app.Serve(8080)
    }
}
```

---

## Pièges

- **`amalgame-net-http` doit être `>= 0.11.1`.** Les versions plus
  anciennes perdent tous les en-têtes de réponse personnalisés sur le
  réseau — vos cookies `Set-Cookie`, CSP, CORS et CSRF disparaissent
  silencieusement. `amc package add web` résout un plancher correct ; ne
  fixez pas une version plus ancienne de net-http à la main.
- **L'ordre des routes est significatif** — statique avant paramétré,
  préfixes de fichiers statiques spécifiques avant les généraux.
- **Le corps de la requête est uniquement du texte** en v0.17.1 ; le
  streaming d'upload binaire n'est pas livré.
- **`Cors.AllowAll()` est destiné au développement.** En production,
  listez des origines explicites avec
  `Cors.Strict().WithAllowedOrigins([...])`.
- **HTTPS nécessite OpenSSL 3.x sur le système.** Les points d'entrée
  `ServeHttps*` s'y lient.

Pour les patterns de configuration (superposition toml/env/flag) voir
[mosaic-configuration.md](../mosaic-configuration.md). Pour les types
HTTP eux-mêmes, voir le README du package `amalgame-net-http`.
