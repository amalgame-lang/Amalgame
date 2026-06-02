# 4 · Bibliothèque standard

La stdlib est une fine façade sur les en-têtes C du runtime dans
`runtime/` et les modules pure-AM dans `src/stdlib/`. Toutes les
méthodes statiques sont accessibles sous la forme `Class.Method(args)`
et sont mappées en `Class_Method(args)` dans le C émis — ce qui est
listé ici est donc exactement ce qui est lié.

Ce chapitre documente l'API publique de la stdlib **embarquée** —
les modules livrés avec chaque binaire amc. Pour les détails
d'implémentation, consultez les en-têtes eux-mêmes : `runtime/Amalgame_*.h`.

## Packages de l'écosystème (depuis v0.7.7)

La séparation du framework livrée en v0.7.7 a déplacé dix modules
orientés utilisateur hors de la stdlib embarquée vers des packages
indépendants sur `amalgame-lang/`. Ils sont versionnés séparément et
s'installent une fois via `amc package add <nom-court>` :

| Module                       | Nom court du package | Dépôt                                                                                       |
| ---------------------------- | -------------------- | ------------------------------------------------------------------------------------------ |
| `Amalgame.Math`              | `math`               | [amalgame-math](https://github.com/amalgame-lang/amalgame-math)                            |
| `Amalgame.Math.Vec`          | `math-vec`           | [amalgame-math-vec](https://github.com/amalgame-lang/amalgame-math-vec)                    |
| `Amalgame.Random`            | `random`             | [amalgame-random](https://github.com/amalgame-lang/amalgame-random)                        |
| `Amalgame.Encoding`          | `encoding`           | [amalgame-encoding](https://github.com/amalgame-lang/amalgame-encoding)                    |
| `Amalgame.Crypto`            | `crypto`             | [amalgame-crypto](https://github.com/amalgame-lang/amalgame-crypto)                        |
| `Amalgame.DateTime`          | `datetime`           | [amalgame-datetime](https://github.com/amalgame-lang/amalgame-datetime)                    |
| `Amalgame.Logging`           | `logging`            | [amalgame-logging](https://github.com/amalgame-lang/amalgame-logging)                      |
| `Amalgame.Service`           | `service`            | [amalgame-service](https://github.com/amalgame-lang/amalgame-service)                      |
| `Amalgame.IO.FileWatcher`    | `io-filewatcher`     | [amalgame-io-filewatcher](https://github.com/amalgame-lang/amalgame-io-filewatcher)        |
| `Amalgame.Formats.Yaml`      | `yaml`               | [amalgame-yaml](https://github.com/amalgame-lang/amalgame-yaml)                            |
| `Amalgame.Threading`         | `threading`          | [amalgame-threading](https://github.com/amalgame-lang/amalgame-threading) — Mutex / Channel / Thread.Spawn via pthread |
| `Amalgame.Async`             | `async`              | [amalgame-async](https://github.com/amalgame-lang/amalgame-async) — coroutines stackful (Fiber / Channel / Scheduler) + I/O epoll + annulation + WithTimeout (Linux) |
| `Amalgame.Net.Http`          | `net-http`           | [amalgame-net-http](https://github.com/amalgame-lang/amalgame-net-http) — HTTP/1.1 + HTTP/2 (h2c + HTTPS+ALPN) + WebSocket, dont `Http1.ServeAsync(With)` (Linux fiber-driven) |
| `Amalgame.Web`               | `web`                | [amalgame-web](https://github.com/amalgame-lang/amalgame-web) — framework web Mosaic : Router + Sessions + WebContext + 6 points d'entrée `Serve*` |

Chaque package dispose de son propre README documentant sa surface ;
ils suivent le même style `Class.Method(args)` que la stdlib embarquée.

```bash
amc package add datetime logging        # prend le dernier tag
amc package add yaml@v0.1.0             # version explicite
```

`amc package add` lit les `[dependencies]` de `amalgame.toml` (ou les
crée) et écrit le tag épinglé + le commit-rev dans `amalgame.lock`.

## Console — entrées/sorties terminal

`runtime/Amalgame_Console.h`

| Méthode                                    | Effet                                   |
| ------------------------------------------ | --------------------------------------- |
| `Console.WriteLine(s: string)`             | affiche s + `\n` sur stdout             |
| `Console.Write(s: string)`                 | affiche s sur stdout (sans saut de ligne)|
| `Console.WriteError(s: string)`            | affiche s + `\n` sur stderr             |
| `Console.ReadLine() : string`              | lit une ligne depuis stdin (sans `\n` final) |
| `Console.Clear()`                          | efface le terminal (séquence ANSI)      |

```kotlin
Console.WriteLine("Hi")
Console.WriteError("oops")
let name = Console.ReadLine()
Console.WriteLine("Hello, " + name + "!")
```

## String — manipulation de texte

`runtime/Amalgame_String.h` · déclarations canoniques dans [stdlib/strings.am](../../stdlib/strings.am)

### Inspection

| Méthode                                   | Retour  | Notes                           |
| ----------------------------------------- | ------- | ------------------------------- |
| `String.Length(s)`                        | int     | longueur en octets (pas en points de code)|
| `String.IsEmpty(s)`                       | bool    | vrai pour `""` et `null`        |
| `String.IsWhitespace(s)`                  | bool    | vrai pour vide / tout espace    |

### Recherche

| `String.Contains(s, sub)`                 | bool    |                                 |
| `String.StartsWith(s, prefix)`            | bool    |                                 |
| `String.EndsWith(s, suffix)`              | bool    |                                 |
| `String.IndexOf(s, sub)`                  | int     | -1 si non trouvé                |
| `String.LastIndexOf(s, sub)`              | int     | -1 si non trouvé                |

### Découpage & accès

| `String.Substring(s, start, len)`         | string  | borné aux limites               |
| `String.From(s, start)`                   | string  | suffixe à partir de l'index `start`|
| `String.Until(s, end)`                    | string  | préfixe jusqu'à l'index `end`   |
| `String.CharAt(s, i)`                     | string  | chaîne d'un octet               |

### Transformation

| `String.ToUpper(s)`                       | string  | ASCII (pas de tables Unicode)   |
| `String.ToLower(s)`                       | string  | ASCII                           |
| `String.Trim(s)`                          | string  | supprime les espaces en début et fin |
| `String.TrimStart(s)`                     | string  |                                 |
| `String.TrimEnd(s)`                       | string  |                                 |
| `String.Replace(s, old, new)`             | string  | remplace toutes les occurrences |
| `String.Repeat(s, n)`                     | string  | concatène s n fois              |
| `String.PadLeft(s, len, ch)`              | string  | (quand implémenté)              |
| `String.PadRight(s, len, ch)`             | string  | (quand implémenté)              |

### Découpage / jointure

| `String.Split(s, sep) : List<string>`     |         | séparateur vide → liste d'un seul élément|
| `String.Join(parts, sep) : string`        |         | inverse de Split                |

### Conversion

| `String.ToInt(s) : int`                   |         | 0 en cas d'échec de parsing     |
| `String.ToFloat(s) : float`               |         | 0.0 en cas d'échec              |
| `String.ToBool(s) : bool`                 |         | vrai pour `"true"` et `"1"`     |
| `String.FromInt(n) : string`              |         | décimal                         |
| `String.FromFloat(n) : string`            |         | format `%g`                     |
| `String.FromBool(b) : string`             |         | `"true"` / `"false"`            |
| `String.FromByte(b) : string`             |         | chaîne d'un octet de 0..255     |
| `String.FromCodepoint(cp) : string`       |         | point de code encodé en UTF-8   |

```kotlin
let s = "  Hello, World!  "
let trimmed = String.Trim(s)
Console.WriteLine(String.ToUpper(trimmed))           // → HELLO, WORLD!
let parts = String.Split(trimmed, ", ")              // → ["Hello", "World!"]
Console.WriteLine(String.FromInt(String.Length(s))) // → 19
```

## File / Path — système de fichiers

`runtime/Amalgame_IO.h`

### Lecture

| `File.ReadAll(path) : string`             | contenu intégral du fichier             |
| `File.ReadLine(path, n) : string`         | n-ième ligne (quand implémenté)         |
| `File.Exists(path) : bool`                |                                         |
| `File.Size(path) : int`                   | octets                                  |

### Écriture

| `File.WriteAll(path, contents)`           | écrase                                  |
| `File.AppendAll(path, contents)`          | ajoute à la suite                       |
| `File.WriteLines(path, lines: List<string>)` | écrase, une ligne par élément        |
| `File.OpenWrite(path)`                    | ouvre un flux global (utilisé par gen_test pour une sortie multi-Mo rapide) |
| `File.StreamLine(line: string)`           | ajoute une ligne au flux ouvert         |
| `File.CloseWrite()`                       | ferme le flux                           |
| `File.Delete(path) : bool`                |                                         |

### Utilitaires de chemin — API plate historique

Les fonctions runtime brutes `Path_*` restent appelables pour la
rétrocompatibilité. Le nouveau code devrait utiliser la façade
`namespace Amalgame.Path` à la place — voir la section
[Path](#path--manipulation-de-chemins-multiplateforme) ci-dessous.

```kotlin
let cfg = File.ReadAll("config.txt")
File.AppendAll("log.txt", "[startup]\n")
let lines = String.Split(cfg, "\n")
File.WriteLines("clean.txt", lines)
```

## Path — manipulation de chemins multiplateforme

`namespace Amalgame.Path` expose une façade `public class Path`
qui reflète la sémantique de `pathlib` de Python / `Path` de Rust.
Opérations purement sur les chaînes — aucune méthode ne touche le
système de fichiers. Pour la résolution de chemins nécessitant de
suivre les liens symboliques ou de vérifier l'existence, utilisez
`File.*` de la section précédente.

Gestion des séparateurs : chaque méthode accepte `/` et `\` sur
toutes les plateformes, reflétant le comportement des API noyau
Windows elles-mêmes. Les chemins en sortie utilisent `/` comme
séparateur canonique (Windows l'accepte partout) ; appelez
`Path.Sep()` si vous avez besoin de l'octet natif à la plateforme
pour des commandes shell ou des chaînes de registre.

```kotlin
import Amalgame.Path

let cfg: string = Path.Combine("/etc/app", "config.toml")  // "/etc/app/config.toml"
let dir: string = Path.Directory(cfg)                       // "/etc/app"
let ext: string = Path.Extension(cfg)                       // ".toml"
let stem: string = Path.Stem(cfg)                            // "config"
let isAbs: bool = Path.IsAbsolute(cfg)                       // true
let norm: string = Path.Normalize("a/b/../c/./d")            // "a/c/d"
```

| Méthode                             | Retour   | Notes                                                                       |
|-------------------------------------|----------|-----------------------------------------------------------------------------|
| `Path.Combine(a: string, b: string)`| `string` | Joint avec `/`, idempotent sur les entrées avec slash final                 |
| `Path.Extension(p: string)`         | `string` | `.gz` pour `report.tar.gz` (dernière extension seulement), `""` si aucune  |
| `Path.Filename(p: string)`          | `string` | Dernier composant du chemin (`/a/b/c.txt` → `c.txt`)                       |
| `Path.Directory(p: string)`         | `string` | Répertoire parent (`/a/b/c` → `/a/b`, bare `c` → `.`)                      |
| `Path.Stem(p: string)`              | `string` | Nom de fichier sans la dernière extension (`report.tar.gz` → `report.tar`) |
| `Path.IsAbsolute(p: string)`        | `bool`   | Vrai pour POSIX `/...` ou Windows `<lecteur>:` / racines UNC               |
| `Path.Normalize(p: string)`         | `string` | Forme canonique lexicale (normalise `//`, résout `.`/`..`) ; vide → `"."`  |
| `Path.Sep()`                        | `string` | `"/"` sur POSIX, `"\\"` sur Windows                                        |

**Sémantique de Normalize** : miroir de `filepath.Clean` de Go :

```kotlin
Path.Normalize("a/b/../c")    // "a/c"
Path.Normalize("./a//b")      // "a/b"
Path.Normalize("/usr/../etc") // "/etc"
Path.Normalize("../../foo")   // "../../foo"  (préservé quand relatif)
Path.Normalize("")            // "."
Path.Normalize("/")           // "/"
```

Cette fonction **ne touche pas le système de fichiers** — donc `..`
au-delà d'un lien symbolique peut se résoudre incorrectement par
rapport au chemin réel. Utilisez un utilitaire de résolution
filesystem si vous en avez besoin.

## Collections — List, Map, Set

`runtime/Amalgame_Collections.h`

### List<T>

| `let xs = new List<int>()`           | constructeur                     |
| `xs.Add(x)`                          | ajouter à la fin                 |
| `xs.Get(i) : T`                      | accès par indice                 |
| `xs.Count() : int`                   | longueur                         |
| `xs.IsEmpty() : bool`                |                                  |
| `xs.Remove(item) : bool`             | par valeur (égalité de pointeur) |
| `xs.RemoveAt(i)`                     | par indice                       |
| `xs.Clear()`                         | vider la liste                   |
| `xs.Reserve(n)`                      | augmenter la capacité            |

Méthodes d'ordre supérieur (depuis v0.3.6, prennent une lambda) :

| `xs.Filter(pred) : List<T>`          | garde les éléments où `pred(x)` est vrai |
| `xs.Map(fn) : List<T>`               | applique `fn` à chaque élément   |
| `xs.Reduce(init, fn) : U`            | repli gauche `fn(acc, x) → acc`  |
| `xs.ForEach(action)`                 | exécute `action(x)` pour chaque élément |
| `xs.Any(pred) : bool`                | vrai si un élément satisfait pred|
| `xs.All(pred) : bool`                | vrai si tous les éléments satisfont pred |
| `xs.CountIf(pred) : int`             | nombre d'éléments satisfaisant pred |

```kotlin
let xs = new List<string>()
xs.Add("a") ; xs.Add("b") ; xs.Add("c")
for i in 0..xs.Count() {
    Console.WriteLine(xs.Get(i))
}

// Ordre supérieur
let nums = new List<int>()
nums.Add(1) ; nums.Add(2) ; nums.Add(3) ; nums.Add(4)
let big   = nums.Filter(x => x > 2)              // [3, 4]
let times = nums.Map(x => x * 10)                // [10, 20, 30, 40]
let total = nums.Reduce(0, (acc, x) => acc + x)  // 10
```

Les valeurs lambda sont encore `(i64) → i64` à la frontière C en
v0.3.6 — la fermeture passe par `intptr_t`. Les prédicats retournant
`bool` fonctionnent car `false`/`true` font l'aller-retour via int
(0 / 1). Les signatures non-int (ex. `xs.Map(x => x.Name)` où le
résultat est `List<string>`) nécessitent la couche de typage lambda
prévue pour une version ultérieure.

### Map<K,V>

| `let m = new Map<string,int>()`      |                                  |
| `m.Set(k, v)`                        | insérer ou écraser               |
| `m.Get(k) : V`                       | NULL si absent                   |
| `m.Has(k) : bool`                    |                                  |
| `m.Remove(k) : bool`                 |                                  |
| `m.Size() : int`                     |                                  |

### Set<T>

| `let s = new Set<int>()`             |                                  |
| `s.Add(x)`                           | idempotent                       |
| `s.Contains(x) : bool`               |                                  |
| `s.Remove(x) : bool`                 |                                  |
| `s.Size() : int`                     |                                  |

> Les types d'éléments génériques sont effacés en `void*` au niveau C
> (voir chapitre 5). Le boxing pour les types primitifs utilise
> `(void*)(intptr_t)v`.

## Net — TCP et UDP

`runtime/Amalgame_Net.h` · sockets POSIX / Winsock2

> HTTP a été retiré du runtime embarqué en v0.8.31 — installez le
> package externe `amalgame-net-http` (`amc package add net-http`)
> pour le client + serveur HTTP/1.1 pure-AM. amc ne lie plus libcurl.

### TCP

| `TcpServer.Listen(port) : TcpServer`     | bind + listen          |
| `TcpServer.Accept() : TcpConn`           | bloque + accepte       |
| `TcpServer.IsListening() : bool`         |                        |
| `TcpServer.Stop()`                       |                        |
| `TcpConn.Send(data: string) : bool`      |                        |
| `TcpConn.Receive() : string`             |                        |
| `TcpConn.Close() : bool`                 |                        |

> Net est le sous-ensemble le plus expérimental — les API peuvent évoluer.

## Net.WebSocket — client RFC 6455 (trames texte, TCP simple)

`runtime/Amalgame_WebSocket.h`

Client minimal implémentant RFC 6455 sur TCP simple — suffisant pour
communiquer avec tout endpoint `ws://` qui échange des messages texte.
Le handshake (SHA-1 + base64 de la valeur `Sec-WebSocket-Accept`) est
calculé dans le runtime afin que l'en-tête soit auto-contenu ; pas
d'OpenSSL ni de bibliothèque crypto externe à ce stade.

**Périmètre v0.7.3 :**
- `ws://host:port/path` — TCP simple
- Trame Client → Serveur avec le masque requis par RFC
- Trames texte (opcode `0x1`)
- Réponse Pong automatique aux Ping serveur (`0x9`)
- Handshake de fermeture (`0x8`)

**Reporté :** TLS `wss://`, trames binaires (`0x2`),
trames de continuation (messages multi-fragments), négociation
per-message-deflate, sous-protocoles HTTP (`Sec-WebSocket-Protocol`).

```kotlin
let ws = WebSocket.Connect("echo.websocket.org", 80, "/")
if (ws == null) {
    Console.WriteError("connect failed")
    return
}

WebSocket.SendText(ws, "hello")
let reply: string = WebSocket.ReceiveText(ws)
Console.WriteLine(reply)

WebSocket.Close(ws)
```

| Méthode                                                      | Retour          | Notes                                                        |
|--------------------------------------------------------------|-----------------|--------------------------------------------------------------|
| `WebSocket.Connect(host: string, port: int, path: string)`   | `WebSocket*`    | `null` sur DNS / refus / 101 / mauvaise clé accept           |
| `WebSocket.SendText(ws, text: string)`                       | `bool`          | Faux en cas d'erreur d'écriture                             |
| `WebSocket.ReceiveText(ws)`                                  | `string`        | `null` à la fermeture / erreur de lecture ; `""` sur trame non-texte |
| `WebSocket.Close(ws)`                                        | `void`          | Envoie une trame Close + ferme le socket                    |
| `WebSocket.IsConnected(ws)`                                  | `bool`          | Faux après Close / erreur                                   |
| `WebSocket.GetHost(ws) / GetPort(ws)`                        | `string` / `int`| Repris depuis les arguments de connect                      |
| `WebSocket.AcceptKey(clientKey: string)`                     | `string`        | Exposé pour les tests / vérification manuelle               |

**Limite de taille de trame :** le récepteur refuse les charges utiles
supérieures à 16 Mio pour éviter qu'un serveur malveillant ou bogué
ne provoque un OOM. Ajustez la limite dans l'en-tête runtime si votre
protocole la dépasse légitimement.

**Trames binaires** (opcode `0x2`) sont rapportées comme `""` plutôt
que NULL — les appelants qui doivent distinguer « fermeture » de
« non-texte » doivent associer `ReceiveText` à une vérification
`IsConnected`.

## Args / Exit — processus

Initialisés dans `int main()` et accessibles depuis Amalgame :

| `Args.Count() : int`            | argc                       |
| `Args.Get(i) : string`          | argv[i] (i=0 est le programme) |
| `Exit.Set(n: int)`              | définit le code de retour du processus |
| `Exit.Get() : int`              | lit le code de retour actuel |

```kotlin
public static void Main(string[] args) {
    let n = Args.Count()
    Console.WriteLine("argc = {String.FromInt(n)}")
    var i = 1
    while (i < n) {
        Console.WriteLine(Args.Get(i))
        i = i + 1
    }
    Exit.Set(0)
}
```

Le paramètre `args: string[]` est une signature vestigiale — utilisez
`Args.Count()` / `Args.Get(i)` à la place.

## Json — parsing, encodage, accesseurs

`src/stdlib/json.am` · implémentation pure-Amalgame, descente récursive

Parseur strict RFC 8259 + encodeur + une surface d'accès `JsonValue`.
Utilisé en interne par `amc lsp`, `amc migrate`, `amc generate`,
`amc explain` pour lire les réponses API ; disponible pour le code
utilisateur sous le namespace `Amalgame.Json`.

```kotlin
import Amalgame.Json

let body = "{\"users\":[{\"name\":\"Alice\",\"age\":30}]}"
let r = Json.Parse(body)
if (r.Ok) {
    let root: JsonValue  = r.Value
    let users: JsonValue = root.Get("users")
    let u0: JsonValue    = users.At(0)
    let name: JsonValue  = u0.Get("name")
    Console.WriteLine(name.AsString())     // → Alice
}
```

| `Json.Parse(s) : JsonResult`                    | parse, retourne ok/err       |
| `Json.Encode(v: JsonValue) : string`            | sérialisation compacte       |
| `Json.EscapeString(s) : string`                 | échappe pour l'intégration   |
| `Json.NullValue() / OfBool / OfInt / OfFloat`   | constructeurs factory        |
| `Json.OfString / OfArray`                       | constructeurs factory        |

`JsonValue` porte l'un des sept types (`Null`, `Bool`, `Int`,
`Float`, `String`, `Array`, `Object`) :

| `v.IsNull() / IsBool() / IsInt() / IsFloat() / IsString() / IsArray() / IsObject()` |
| `v.AsBool() / AsInt() / AsFloat() / AsString() / AsArray()`                          |
| `v.Get(key: string) : JsonValue`        | accès objet (Null si absent)    |
| `v.Has(key: string) : bool`             | existence d'une clé objet       |
| `v.Keys() : List<string>`               | ordre d'itération = insertion   |
| `v.At(i: int) : JsonValue`              | accès tableau (Null si absent)  |
| `v.Length() : int`                      | longueur tableau / nombre de clés objet |

> Note codegen : une chaîne comme `r.Value.Get("k").AsString()` passe
> actuellement à travers cgen car `obj.Field.Method()` est abaissé en
> concaténation de noms (`Value_Get`). Extrayez des locaux typés
> intermédiaires (`let v: JsonValue = r.Value; let kn: JsonValue = v.Get("k");
> kn.AsString()`) jusqu'à ce que le correctif codegen arrive. Même
> contournement que dans l'échantillon de test JSON
> `tests/samples/stdlib_json.am`.

## Formats.MsgPack — codec MessagePack 1.0 (sous-ensemble)

`src/stdlib/msgpack.am` · pure-Amalgame, pas d'en-tête runtime

Codec binaire au-dessus de l'arbre `JsonValue` existant. Même forme
en entrée, moins d'octets en sortie — les appelants peuvent changer
de format fil avec un simple renommage d'une ligne :

```kotlin
let bytes: List<int> = MsgPack.EncodeJson(jv)
let jv2:   JsonValue = MsgPack.DecodeJson(bytes)
```

**Couverture :** nil, bool, positive / negative fixint, int 8 / 16 / 32,
fixstr + str 8 / 16 (≤ 65 535 octets), fixarray + array 16 (≤ 65 535
entrées), fixmap + map 16. Couvre >95% des charges utiles typiques de
config / RPC.

**Hors périmètre** (l'encodeur se rabat silencieusement, le décodeur
retourne null) : int 64 / uint 64, float 32 / 64 (les floats JsonValue
sont tronqués en int — l'aller-retour ne fonctionne que pour les
entiers), str 32 / array 32 / map 32, bin / ext / timestamps.

```kotlin
import Amalgame.Formats.MsgPack
import Amalgame.Json
import Amalgame.Collections

// Construire un JsonValue
let m = new JsonValue()
let keys = new List<string>()
let vals = new List<JsonValue>()
let v1 = new JsonValue() v1.SetInt(10)
let v2 = new JsonValue() v2.SetString("world")
keys.Add("a") vals.Add(v1)
keys.Add("b") vals.Add(v2)
m.SetObject(keys, vals)

// Aller-retour encode → decode
let bytes: List<int> = MsgPack.EncodeJson(m)
let back:  JsonValue = MsgPack.DecodeJson(bytes)
Console.WriteLine(String.FromInt(back.Get("a").AsInt()))  // 10
Console.WriteLine(back.Get("b").AsString())               // world
```

| Méthode                                      | Retour        | Notes                                             |
|----------------------------------------------|---------------|---------------------------------------------------|
| `MsgPack.EncodeJson(value: JsonValue)`       | `List<int>`   | Tampon d'octets prêt pour socket / fichier        |
| `MsgPack.DecodeJson(bytes: List<int>)`       | `JsonValue`   | Kind null si tronqué / non pris en charge         |

> **Chaînes ASCII uniquement.** Le décodeur reconstruit les octets de
> charge utile via une table de correspondance 7-bit imprimable — les
> octets non-ASCII font l'aller-retour en `?`. Convient pour les
> charges utiles de style JSON ; utilisez le chemin liste-d'octets bruts
> pour du binaire arbitraire. (Support UTF-8 suivi parallèlement au
> travail String byte-iter en amont.)

> **Repli int de l'encodeur.** Les valeurs hors de la plage int 32
> sont actuellement tronquées à leurs 32 bits de poids faible plutôt
> que d'émettre int 64 — correction facile dès qu'un site d'appel
> en a besoin.

## Regex — expressions régulières étendues POSIX

`runtime/Amalgame_Regex.h`

Binding fin sur `regex.h` POSIX (`regcomp` + `regexec`) — disponible
sur toute plateforme POSIX et MinGW, sans dépendance PCRE tierce.

**Syntaxe : POSIX étendu (ERE).** Au quotidien : `.` `*` `+` `?`
`^` `$` `[...]` `(...)` `|` `{n,m}`.

**Hors périmètre** (territoire PCRE) : raccourcis `\d` / `\w` / `\s`,
look-arounds, modificateurs non-greedy (`*?` / `+?`), captures nommées,
classes de propriétés Unicode. Utilisez `Process.Run("grep -P …")`
ou un futur package PCRE2 si vous en avez besoin.

```kotlin
if (Regex.Test("[0-9]+", input)) {
    let m = Regex.Match("([a-z]+) ([a-z]+)", "alpha beta")
    Console.WriteLine(m.GetText())            // "alpha beta"
    Console.WriteLine(m.GroupText(0))         // "alpha"
    Console.WriteLine(m.GroupText(1))         // "beta"
}

let masked: string = Regex.ReplaceAll("[0-9]+", "abc123def456", "X")
// → "abcXdefX"
```

### Niveau supérieur

| Méthode                                                  | Retour   | Notes                                             |
|----------------------------------------------------------|----------|---------------------------------------------------|
| `Regex.Test(pattern: string, subject: string)`           | `bool`   | Vrai si le motif correspond quelque part ; léger  |
| `Regex.Match(pattern, subject)`                          | `Match*` | `null` si aucune correspondance ou motif invalide |
| `Regex.Replace(pattern, subject, replacement)`           | `string` | Première occurrence seulement ; `\1` reste littéral|
| `Regex.ReplaceAll(pattern, subject, replacement)`        | `string` | Chaque occurrence non-chevauchante                |

### Match

| Méthode                           | Retour   | Notes                                             |
|-----------------------------------|----------|---------------------------------------------------|
| `GetText()`                       | `string` | La correspondance complète                        |
| `GetStart() / GetEnd()`           | `int`    | Décalages en octets dans le sujet                 |
| `GroupCount()`                    | `int`    | Nombre de groupes de capture (0..16)              |
| `GroupText(idx: int)`             | `string` | Indexé à 0 ; `""` hors limites                   |
| `GroupStart(idx) / GroupEnd(idx)` | `int`    | `-1` hors limites                                 |

`GroupText(0)` est le **premier groupe entre parenthèses**, pas la
correspondance complète — pour la correspondance complète, utilisez
`GetText()`. POSIX limite les captures à 16 ici
(`AMALGAME_REGEX_MAX_GROUPS`), suffisant pour les cas d'extraction de
config et de tokenisation de gabarits qui motivent l'API.

> Les captures ne sont pas développées dans `Replace` / `ReplaceAll`.
> Un `\1` dans la chaîne de remplacement reste littéral. Si vous avez
> besoin de backreferences dans la substitution, faites la boucle
> vous-même avec `Match` + concaténation de chaînes pour l'instant.

## Compress — gzip + deflate brut via zlib

`runtime/Amalgame_Compress.h` · lie `-lz`

Deux paires de codecs appuyées sur zlib :

- **`Gzip` / `Gunzip`** — enveloppe gzip RFC 1952. Mêmes octets
  que `gzip -c` produirait, magic `1f 8b` correct, adapté aux
  fichiers `.gz` et à HTTP `Content-Encoding: gzip`.
- **`Deflate` / `Inflate`** — deflate brut RFC 1951. Sans en-tête,
  plus compact, adapté aux protocoles embarqués (WebSocket
  per-message-deflate, RPC binaires personnalisés).

Les tampons d'octets circulent sous forme de `List<int>` avec chaque
entrée dans `[0, 255]` — même convention que `Crypto.Sha256` et
`Random.SystemBytes`, donc un `File.ReadBytes(...)` passe directement
sans format intermédiaire. Des utilitaires de chaîne encapsulent le
chemin d'octets UTF-8 pour le cas courant « compresser ce texte ».

Pas d'erreurs structurées en v1 : une entrée malformée ou un échec
interne zlib retourne une **liste vide**. Les appelants qui doivent
distinguer « tronqué » de « mauvaise somme de contrôle » peuvent se
rabattre sur `Process.Run("gunzip")` jusqu'à ce que la proposition
`Result<T, E>` arrive.

```kotlin
import Amalgame.Collections

// Aller-retour sur chaîne
let payload: string = "Hello, Amalgame compression!"
let gz: List<int> = Compress.GzipString(payload)
let back: string  = Compress.GunzipString(gz)             // == payload

// Tampons d'octets bruts
let bytes: List<int> = File.ReadBytes("config.json")      // quand disponible
let z: List<int>     = Compress.Gzip(bytes)
File.WriteBytes("config.json.gz", z)

// Deflate brut pour protocole embarqué
let frame: List<int> = Compress.Deflate(message_bytes)
let msg:   List<int> = Compress.Inflate(frame)
```

| Méthode                               | Retour       | Notes                                                |
|---------------------------------------|--------------|------------------------------------------------------|
| `Compress.Gzip(bytes: List<int>)`     | `List<int>`  | Enveloppe gzip RFC 1952                              |
| `Compress.Gunzip(bytes: List<int>)`   | `List<int>`  | Contrepartie de `Gzip`                               |
| `Compress.Deflate(bytes: List<int>)`  | `List<int>`  | Deflate brut RFC 1951 (sans en-tête)                 |
| `Compress.Inflate(bytes: List<int>)`  | `List<int>`  | Contrepartie de `Deflate`                            |
| `Compress.GzipString(s: string)`      | `List<int>`  | Octets UTF-8 de `s` → gzip                           |
| `Compress.GunzipString(bytes)`        | `string`     | Décompresse + interprète comme chaîne UTF-8          |

Le niveau de compression est fixé à `Z_DEFAULT_COMPRESSION` de zlib
(~6). Les utilisateurs qui doivent ajuster pour la vitesse ou le taux
peuvent pour l'instant déléguer à un shell ; exposer un argument de
niveau est un ajout d'une ligne.

## Database.SQLite — SQL embarqué via package opt-in

`namespace Amalgame.Database.SQLite` est un binding SQLite 3 livré
comme package externe opt-in : **`amalgame-lang/amalgame-database-sqlite`**.
Depuis v0.5, il n'est plus embarqué avec le compilateur — installez-le via :

```bash
amc package add sqlite                  # résolution automatique du dernier (amc 0.6.0+)
amc package add sqlite@v0.2.2           # épingler un tag spécifique
```

Le package vendor l'amalgamation SQLite (domaine public en amont),
donc aucun package système `libsqlite3-dev` n'est requis sur
Linux / macOS / Windows — la source se compile directement dans le
binaire utilisateur au moment de la liaison. Depuis v0.2.1, le
manifeste déclare `[stdlib].precompile = true` (nécessite amc 0.5.4+) :
`amc package add` exécute la passe gcc une fois à l'installation et
met en cache le `.o` dans
`~/.amalgame/packages/.../build/<plateforme>/`, de sorte que les
`amc test` / builds suivants le réutilisent instantanément.

Le namespace s'imbrique sous `Amalgame.Database.<Moteur>` pour que
les backends frères (DuckDB, Postgres, MySQL) puissent arriver en
tant que leurs propres packages sans noms de classes en conflit.
SQLite v0.1.0 est le premier package inaugural ; les frères sont
suivis dans la roadmap.

```kotlin
import Amalgame.Database.SQLite
import Amalgame.Collections

let db = SQLite.Open(":memory:")   // ou un vrai chemin de fichier
if (!SQLite.IsOpen(db)) {
    Console.WriteError("open failed: " + SQLite.LastError(db))
    return
}

SQLite.Exec(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)")
SQLite.Exec(db, "INSERT INTO users (name, age) VALUES ('Alice', 30)")
SQLite.Exec(db, "INSERT INTO users (name, age) VALUES ('Bob', 25)")
let id: int = SQLite.LastInsertId(db)            // 2

// La liste externe n'a pas d'annotation car le parseur rejette les
// génériques imbriqués (`List<List<string>>`). La liste interne est
// annotée pour que le cgen puisse résoudre `.Get(int)` en résultat typé.
let rows = SQLite.QueryAll(db, "SELECT id, name FROM users ORDER BY id")
let firstRow: List<string> = rows.Get(0)
let firstName: string = firstRow.Get(1)          // "Alice"

SQLite.Close(db)
```

| Méthode                               | Retour                 | Notes                                                              |
|---------------------------------------|------------------------|--------------------------------------------------------------------|
| `SQLite.Open(path: string)`           | `AmalgameSQLite*`      | `:memory:` pour en mémoire temporaire ; tout chemin pour disque    |
| `SQLite.IsOpen(db)`                   | `bool`                 | Vérifie que le handle est actif                                    |
| `SQLite.Close(db)`                    | `void`                 | Idempotent ; no-op sur les handles déjà fermés                     |
| `SQLite.Exec(db, sql: string)`        | `bool`                 | SQL sans résultat (DDL / DML / PRAGMA). Retourne faux en cas d'erreur |
| `SQLite.QueryAll(db, sql: string)`    | `List<List<string>>`   | Lignes × colonnes en texte. Liste vide si aucune ligne OU erreur   |
| `SQLite.LastInsertId(db)`             | `int`                  | Rowid du dernier INSERT sur ce handle                              |
| `SQLite.Changes(db)`                  | `int`                  | Nombre de lignes du dernier INSERT/UPDATE/DELETE                   |
| `SQLite.LastError(db)`                | `string`               | Instantané du message d'erreur le plus récent ; vide si aucun      |

**Forme du résultat :** `QueryAll` retourne chaque valeur de colonne
en texte (via `sqlite3_column_text`). Les colonnes NULL deviennent des
chaînes vides. Convertissez les numériques dans Amalgame selon le
besoin (`String_ToInt(row.Get(0))`).

**Liaison :** le manifeste du package déclare `sqlite3.c` sous son
tableau `[stdlib].sources`, donc depuis v0.5.2 **`amc test` le lie
automatiquement** — précompile chaque source une fois vers un `.o`
en cache dans `/tmp`, puis l'ajoute (avec `-ldl -lpthread`) à la
commande de liaison de chaque binaire de test. Aucune étape `gcc`
manuelle n'est nécessaire.

Pour les builds `amc <file>.am` ad hoc (hors `amc test`), la source
vendorisée se trouve toujours dans
`~/.amalgame/packages/github.com/amalgame-lang/amalgame-database-sqlite/<tag>_<sha>/runtime/Amalgame_Database/sqlite/sqlite3.c`
et peut être liée manuellement :

```
PKG=~/.amalgame/packages/github.com/amalgame-lang/amalgame-database-sqlite/v0.2.0_*
gcc -O2 -Iruntime -w -c $PKG/runtime/Amalgame_Database/sqlite/sqlite3.c -o sqlite3.o
gcc -O2 -Iruntime your_program.c sqlite3.o -lgc -lm -ldl -lpthread -o your_program
```

Un futur `amc build` câblera cette étape pour les binaires non-test
également, en réutilisant la même mécanique `[stdlib].sources`.

**Limitations v1 :**

- **Pas de liaison de paramètres.** Tout le SQL est passé en chaînes
  brutes, donc les appelants DOIVENT échapper manuellement toute
  entrée utilisateur ou s'en tenir à du SQL entièrement de confiance.
  La liaison d'instructions préparées avec `?` est la demande v2.
- **Colonnes texte uniquement.** Les types numériques sont retournés
  sous leur représentation chaîne. Les accesseurs typés (`AsInt(0)` /
  `AsBytes(0)`) arrivent avec la liaison `?` en v2.
- **Pas de transactions explicites.** Exécutez `BEGIN` / `COMMIT` /
  `ROLLBACK` via `Exec` pour l'instant ; une API `db.Begin()` /
  `Commit()` arrive en v2.

## Database.DuckDB — analytics embarqué via package opt-in

`namespace Amalgame.Database.DuckDB` est un binding
[DuckDB](https://duckdb.org) — base de données analytique (OLAP)
embarquée, le « SQLite de l'analytics ». Livré comme package
externe opt-in : **`amalgame-lang/amalgame-database-duckdb`**.

```bash
amc package add duckdb                  # résolution automatique du dernier (amc 0.6.0+)
amc package add duckdb@v0.1.1           # épingler un tag spécifique
amc package add github.com/amalgame-lang/amalgame-database-duckdb@v0.1.1
```

Le package vendor l'amalgamation C++ officielle DuckDB (MIT) —
aucun `libduckdb-dev` requis sur aucune plateforme. Nécessite
**amc ≥ 0.5.4** (pour precompile-on-install) et un `g++` compatible
C++17. Depuis v0.1.1, le manifeste déclare `[stdlib].precompile = true`,
donc la passe g++ sur l'amalgamation de ~25 Mo tourne **une seule
fois à l'installation** (typiquement 3–15 minutes selon le CPU) et le
`.o` résultant est mis en cache dans
`~/.amalgame/packages/.../build/<plateforme>/`. Les `amc test` / builds
suivants le réutilisent instantanément. Ignorez la compilation à
l'installation avec `amc package add duckdb --no-precompile` si vous
préférez une installation rapide + compilation paresseuse au premier build.

```amalgame
namespace App

import Amalgame.Collections
import Amalgame.Database.DuckDB

public class Program {
    public static void Main(string[] args) {
        let db = DuckDB.Open("")                       // "" = en mémoire
        DuckDB.Exec(db, "CREATE TABLE t (n INTEGER, kind VARCHAR)")
        DuckDB.Exec(db, "INSERT INTO t VALUES (1, 'a'), (2, 'b'), (3, 'a')")

        let rows = DuckDB.QueryAll(db, "SELECT kind, SUM(n) FROM t GROUP BY kind")
        let firstRow: List<string> = rows.Get(0)
        Console.WriteLine(firstRow.Get(0) + " → " + firstRow.Get(1))

        DuckDB.Close(db)
    }
}
```

| Méthode                             | Retour                | Notes                                                                |
|-------------------------------------|-----------------------|----------------------------------------------------------------------|
| `DuckDB.Open(path: string)`         | `AmalgameDuckDB*`     | Chaîne vide = en mémoire temporaire ; tout chemin pour fichier persistant |
| `DuckDB.IsOpen(db)`                 | `bool`                | Vérifie que l'ouverture + la connexion ont toutes deux réussi        |
| `DuckDB.Close(db)`                  | `void`                | Idempotent ; no-op sur les handles déjà fermés                       |
| `DuckDB.Exec(db, sql: string)`      | `bool`                | SQL sans résultat (DDL / DML). Retourne faux en cas d'erreur         |
| `DuckDB.QueryAll(db, sql: string)`  | `List<List<string>>`  | Lignes × colonnes en texte. Liste vide si aucune ligne OU erreur     |
| `DuckDB.LastError(db)`              | `string`              | Instantané du message d'erreur le plus récent ; vide si aucun        |

**Quand choisir DuckDB vs SQLite :**

- **DuckDB** — charges de travail analytiques (GROUP BY, agrégats,
  jointures sur des millions de lignes), ingestion Parquet/CSV/JSON,
  stockage colonnaire, pipelines data science embarqués.
- **SQLite** — OLTP, écritures ligne par ligne, empreinte réduite,
  décennies de stabilité.

Les deux s'exécutent en processus, les deux se lient via
`[stdlib].sources` et le pipeline v0.5.3 C++ (DuckDB) / C (SQLite).
Ils coexistent dans le même projet — le mangling de namespace signifie
que `SQLite.Open` et `DuckDB.Open` se rabaissent en symboles C
distincts (`Amalgame_Database_SQLite_Open` vs
`Amalgame_Database_DuckDB_Open`) sans collision de liaison.

**Limitations v0.1 :**

- Pas de liaison de paramètres (passer une entrée utilisateur nécessite
  un échappement SQL manuel pour l'instant).
- Accesseurs de colonnes texte uniquement (les valeurs numériques
  reviennent sous leur représentation chaîne).
- Pas d'API de transactions explicites (exécutez `BEGIN`/`COMMIT` via `Exec`).
- Pas d'instructions préparées / pas d'utilitaires Parquet (prochains
  jalons).

## Ce qui manque

- Math plus riche (trigonométrie, logarithmes)
- Abstractions async/iter/streaming sur les collections
- Heure locale / fuseaux horaires (reporté depuis DateTime v1)
- Regex
- Backends Database frères (DuckDB / Postgres / MySQL) — même
  famille de namespaces que SQLite, amalgamation vendorisée pour
  ceux qui en livrent une (DuckDB), lien dynamique pour les
  backends réseau (Postgres / MySQL).
- Un gestionnaire de packages et un écosystème

Ces points sont suivis dans [ROADMAP_COMPLET.md](../../ROADMAP_COMPLET.md).
