# Cookbook

Recettes idiomatiques pour les patterns courants en Amalgame.
Chaque recette est un snippet auto-suffisant qu'on peut copier
dans un fichier `.am`, compiler et exécuter avec `amc build`.

Pour la référence des classes / méthodes stdlib, voir
[04-stdlib.md](04-stdlib.md). Pour le langage lui-même, voir
[02-language-tour.md](02-language-tour.md).

---

## Strings

### Découper une string sur un séparateur

```amalgame
let line: string = "alice,30,paris"
let parts: List<string> = String_Split(line, ",")
// parts.Get(0) == "alice"
// parts.Get(1) == "30"
// parts.Get(2) == "paris"
```

### Tester un prefix/suffix

```amalgame
if (String_StartsWith(name, "amalgame-")) { /* ... */ }
if (String_EndsWith(path, ".am"))         { /* ... */ }
```

### Interpolation

```amalgame
let user: string = "Alice"
let age:  int    = 30
Console.WriteLine("hello {user}, you are {age}")
// → hello Alice, you are 30
```

L'interpolation accepte les chaînes d'appels :
`"count: {users.Count()}"`. Les sucres `${...}` du shell ne sont
pas reconnus — utiliser `$NAME` en string littérale si le but est
de produire du bash.

### Trim + lower

```amalgame
let cleaned: string = String_Lower(String_Trim(raw))
```

### Joindre une liste avec un séparateur

```amalgame
let xs: List<string> = new List<string>()
xs.Add("a"); xs.Add("b"); xs.Add("c")
let joined: string = String_Join(xs, ", ")
// joined == "a, b, c"
```

---

## Collections

### List<T> : itérer avec index

```amalgame
let users: List<User> = LoadUsers()
for i in 0..users.Count() {
    let u: User = users.Get(i)
    Console.WriteLine(String_FromInt(i) + ": " + u.Name)
}
```

### List<T> : itération directe

```amalgame
for u in users {
    Console.WriteLine(u.Name)
}
```

### List<T> : compréhensions

```amalgame
let adults: List<User>   = [u for u in users if u.Age >= 18]
let names:  List<string> = [u.Name for u in users]
```

### Map<K, V> : compter les occurrences

```amalgame
let counts: Map<string, int> = new Map<string, int>()
for w in words {
    let prev: int = 0
    if (counts.Has(w)) { prev = counts.Get(w) }
    counts.Set(w, prev + 1)
}
```

### Filter / Map fluent (closures)

```amalgame
let big: List<User> = users.Filter(u => u.Age >= 18)
let names: List<string> = users.Map(u => u.Name)
let total: int = numbers.Reduce(0, (acc, n) => acc + n)
```

---

## Fichiers + JSON

### Lire / écrire un fichier texte

```amalgame
let content: string = File.ReadAll("/path/to/input.txt")
File.WriteAll("/path/to/output.txt", content + "\nappendu\n")
```

### Charger un JSON, lire un champ

```amalgame
import Amalgame.Json

let raw: string    = File.ReadAll("config.json")
let jv:  JsonValue = Json.Parse(raw)
if (jv.IsObject()) {
    let host: string = jv.Get("host").AsString()
    let port: int    = jv.Get("port").AsInt()
}
```

### Sérialiser un map en JSON

```amalgame
let obj: JsonValue = JsonValue.NewObject()
obj.Set("name", JsonValue.NewString(user.Name))
obj.Set("age",  JsonValue.NewInt(user.Age))
let serialized: string = Json.Stringify(obj)
File.WriteAll("user.json", serialized)
```

---

## Outils CLI

### Parser argv avec ArgParser

```amalgame
let ap = new ArgParser()
ap.Flag("-v")
ap.Flag("--verbose")
ap.Option("--output")
ap.Parse(Args_Count(), 1)

let verbose: bool = ap.HasFlag("-v") || ap.HasFlag("--verbose")
let out:     string = ap.GetOption("--output")
let files:   List<string> = ap.GetPositionals()
```

### Lire une variable d'environnement avec valeur par défaut

```amalgame
var host: string = Env_Get("MY_APP_HOST")
if (String_Length(host) == 0) { host = "localhost" }
```

### Code de sortie personnalisé

```amalgame
public static void Main(string[] args) {
    if (problem) {
        Console.WriteError("oops")
        Exit_Set(2)
        return
    }
    // exit 0 par défaut
}
```

---

## Processus

### Capturer stdout + stderr d'une commande

```amalgame
let r = Process.RunCapture("git log --oneline -5")
if (r.Exit == 0) {
    Console.WriteLine(r.Stdout)
} else {
    Console.WriteError("git failed: " + r.Stderr)
}
```

### Spawn streamable (v0.8.22+)

```amalgame
let p = Process.Spawn("tail -F /var/log/syslog", true)
while (p.IsAlive()) {
    let line: string = p.ReadLine()
    Console.WriteLine("> " + line)
}
let code: int = p.Wait()
```

---

## Réseau

### Client TCP : connect + send + receive

```amalgame
let cli = new TcpClient()
if (cli.Connect("example.com", 80)) {
    cli.SendString("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n")
    let resp: string = cli.ReceiveAll()
    cli.Close()
}
```

### Socket UDP : send + receive (avec adresse source, v0.8.52+)

```amalgame
let sock = new UdpSocket()
sock.Bind(0)   // port éphémère
sock.SetRecvBuf(8 * 1024 * 1024)   // 8 MiB

sock.SendTo("Hello", "127.0.0.1", 9000)

let pkt = sock.ReceiveFrom(1500)
if (pkt != null) {
    Console.WriteLine("from {pkt.FromIp}:{pkt.FromPort} → {pkt.Data}")
}
```

---

## Patterns

### Match avec arm guards

```amalgame
let classify: string = match n {
    0           => "zero",
    n if n > 0  => "positive",
    _           => "negative"
}
```

### Enum algébrique + destructuring

```amalgame
public enum Shape {
    Circle(radius: float)
    Square(side: float)
    Rectangle(w: float, h: float)
}

let area: float = match shape {
    Shape.Circle(r)      => 3.14 * r * r,
    Shape.Square(s)      => s * s,
    Shape.Rectangle(w, h) => w * h
}
```

### Try / catch / finally

```amalgame
try {
    DangerousOp()
} catch e {
    Console.WriteError("oops: " + e.Message)
} finally {
    Cleanup()
}
```

### Accès null-safe + coalescence

```amalgame
let name: string = user?.Name ?? "Anonymous"
let age:  int    = user?.Age  ?? 0
```

### Clause guard

```amalgame
public void Process(input: string?) {
    guard input != null else { return }
    // input non-null à partir d'ici
}
```

---

## Threading + async

### Lancer un thread (amalgame-threading)

```amalgame
import Amalgame.Threading

Threading.ThreadSpawn(_ => {
    Console.WriteLine("hello from another thread")
    return 0
}, null)
```

### Channel producteur/consommateur

```amalgame
let ch = Threading.ChannelNew(16)
Threading.ThreadSpawn(_ => {
    for i in 0..100 { Threading.ChannelSend(ch, i) }
    Threading.ChannelClose(ch)
    return 0
}, null)
while (!Threading.ChannelIsClosed(ch)) {
    let v: int = Threading.ChannelReceive(ch)
    Console.WriteLine("got: " + String_FromInt(v))
}
```

### Serveur HTTP piloté par fibers (amalgame-net-http + amalgame-async)

```amalgame
import Amalgame.Net.Http
import Amalgame.Async

Http1.ServeAsync(8080, (req, res) => {
    res.SetStatus(200)
    res.WriteText("hello from fiber!")
})
```

---

## Inline C

### Petite primitive bas niveau

```amalgame
public class Hash {
    public static int FastHash(string s) {
        @c {
            const char* p = s ? s : "";
            unsigned int h = 2166136261u;
            while (*p) h = (h ^ (unsigned char)*p++) * 16777619u;
            return (i64) h;
        }
    }
}
```

### Inclure un header système

```amalgame
@c { #include <sys/utsname.h> }

public class Sys {
    public static string Hostname() {
        @c {
            struct utsname u;
            uname(&u);
            return code_string_dup(u.nodename);
        }
    }
}
```

---

## Templates / scaffolders

```bash
amc new myapp                              # exe (default)
amc new myapp --template service           # daemon (systemd/launchd/sc.exe)
amc new myapp --template ui-web-form       # webview GUI (amalgame-ui-web)
amc new myapp --template lib               # library
amc new myapp --template test              # standalone test bundle
```

---

## Voir aussi

- [docs/guide/02-language-tour.md](02-language-tour.md) — syntaxe complète
- [docs/guide/04-stdlib.md](04-stdlib.md) — référence stdlib bundled
- [docs/language/grammar.ebnf](https://github.com/amalgame-lang/Amalgame/blob/main/docs/language/grammar.ebnf) — grammaire formelle
- `amc doc <file.am>` — extraire les doc-comments d'un fichier en Markdown
