# 2 · Tour du langage

Un parcours de toutes les fonctionnalités du langage avec des extraits
exécutables. Si vous êtes pressé, survolez ce chapitre une première fois
et revenez-y au besoin.

## Conventions

- Chaque fichier commence par `namespace Foo.Bar`. Les symboles C émis
  pour ce fichier sont préfixés par `Foo_Bar_`.
- Le point d'entrée du programme est une méthode `Program.Main(string[] args)`
  sur une `public class Program`. En son absence, le fichier est compilé
  comme une bibliothèque (sans `main()`).
- `import Amalgame.X` est actuellement informatif — le résolveur connaît
  la stdlib globalement, donc les imports ne sont pas strictement
  nécessaires pour utiliser `Console`, `File`, etc.
- Commentaires : `// ligne` et `/* bloc */`.

## Variables et types primitifs

```kotlin
let x = 42                  // liaison immuable (recommandée)
var y = 3.14                // liaison mutable
let n: int = 7              // annotation de type explicite
let s: string = "hello"
let b: bool = true
let d: double = 1.5
```

Types primitifs : `int` (i64), `float` / `double`, `bool`, `string`
(C `char*`), `void`. Les types tableau utilisent `T[]` (ex. `string[]`).
Les types nullables utilisent `T?` (ex. `User?`).

Les littéraux entiers acceptent trois bases (v0.8.46+) :

```kotlin
let dec = 255       // décimal
let hex = 0xFF      // hexadécimal — aussi 0X..., chiffres 0-9 a-f A-F
let bin = 0b1111    // binaire      — aussi 0B..., chiffres 0/1
```

Les trois produisent la même valeur `int` (`255`). Les littéraux hex sont
transmis verbatim au C ; les littéraux binaires sont abaissés en décimal
à la phase lexicale pour que la sortie C reste portable (`0b...` est une
extension GCC).

## Opérateurs

```kotlin
// Arithmétique
let a = 1 + 2 - 3 * 4 / 5 % 6
// Comparaison
let cmp = a == 0 || a != 0 && a < 10
// Bit à bit
let bits = (a & 0xff) | (a >> 4) ^ ~a << 1
// Assignations composées
var n = 0
n += 1
n -= 1
n *= 2
n /= 2
n %= 5
n &= 0xff
n |= 0x10
n ^= 0xa
n <<= 2
n >>= 1
// Plage (utilisée dans for-in et match)
for i in 0..10 { /* … */ }
// Pipeline (composition de gauche à droite ; sucre sur a.f().g())
// ex. x |> double |> Console.WriteLine    (quand chacun accepte le résultat précédent)
```

La précédence est celle standard de C/Java ; utilisez des parenthèses en
cas de doute. Les opérateurs unaires `!`, `-`, `~` existent ; `!` requiert
un opérande de type bool ; `~` est le complément bit à bit.

## Chaînes de caractères

```kotlin
let plain = "hello"
let escaped = "tab\there\nnewline"
let hex = "\x1b[31mred\x1b[0m"          // \xHH = un octet
let unicode = "héllo 中 €"      // \uHHHH = point de code UTF-8
let multi = """
line 1
line 2 with "quotes" and \n untouched
"""
let interp = "x={n} obj.f={obj.field} call={Math.Sqrt(16.0)}"
```

L'interpolation prend en charge les variables simples, `obj.field` (où
`obj` est un local connu), `this.field`, et les appels de méthodes/statiques
(ex. `{Math.Sqrt(x)}`, `{String.Length(s)}`). Pour des expressions plus
profondes comme `{Math.Sqrt(obj.field)}`, utilisez d'abord une liaison
let :

```kotlin
let v = obj.field
"sqrt = {Math.Sqrt(v)}"
```

## Classes

```kotlin
public class User {
    // Champs avec types explicites
    public Name: string
    public Age:  int
    private Score: int

    // Constructeur (méthode portant le nom de la classe)
    public User(string name, int age) {
        this.Name  = name
        this.Age   = age
        this.Score = 0
    }

    // Méthodes
    public string Greet() {
        return "Hello, " + this.Name + "!"
    }

    public void AddScore(int points) {
        this.Score = this.Score + points
    }
}
```

Utilisation :

```kotlin
let u = new User("Alice", 30)
Console.WriteLine(u.Greet())
u.AddScore(10)
```

### Méthodes statiques

```kotlin
public class Calc {
    public static int Add(int a, int b) { return a + b }
    public static int Mul(int a, int b) { return a * b }
}

let n = Calc.Add(2, 3)
```

### Héritage

```kotlin
public class Animal {
    public Name: string
    public Animal(string n) { this.Name = n }
    public string Speak() { return "..." }
}

public class Cat extends Animal {
    public Cat(string n) {
        // Pas encore de `super()` — affectez les champs directement si besoin.
        this.Name = n
    }
    public string Speak() { return "meow" }
}
```

### Classes de données / enregistrements

```kotlin
data class Point(float X, float Y)
record Line(Point A, Point B)
```

Les deux se déroulent en une classe avec des champs publics et un
constructeur public dans l'ordre de déclaration des champs. Utile pour
les simples porteurs de valeurs.

## Énumérations

Énumération simple :

```kotlin
public enum Direction {
    North
    East
    South
    West
}

let d = Direction.North
match d {
    Direction.North => Console.WriteLine("⬆"),
    Direction.East  => Console.WriteLine("➡"),
    Direction.South => Console.WriteLine("⬇"),
    Direction.West  => Console.WriteLine("⬅")
}
```

Énumération algébrique (union taguée) :

```kotlin
public enum Shape {
    Circle(int)
    Rect(int, int)
    Triangle(int, int, int)
}

let s = Shape.Rect(4, 3)
match s {
    Circle(r)        => Console.WriteLine("circle r={String.FromInt(r)}"),
    Rect(w, h)       => Console.WriteLine("{String.FromInt(w * h)}"),
    Triangle(a,b,c)  => Console.WriteLine("triangle")
}
```

> Les charges utiles de variante dans la déclaration sont **uniquement
> des types**, pas des paramètres nommés. Les noms sont introduits dans
> le bras de match via le motif de liaison : `Circle(r) => …`.

## Interfaces

```kotlin
public interface Shape {
    int Area()
    string Name()
}

public class Square implements Shape {
    public Side: int
    public Square(int s) { this.Side = s }
    public int Area() { return this.Side * this.Side }
    public string Name() { return "square" }
}
```

Depuis la v0.3.5, les interfaces prennent en charge les paramètres
génériques et le vérificateur de types applique le contrat :

```kotlin
public interface IComparable<T> {
    Compare(T other) -> int
}

public class IntBox implements IComparable<int> {
    public Value: int
    public IntBox(int v) { this.Value = v }
    public int Compare(int other) { return this.Value - other }
}
```

La vérification substitue `T → int` au site `implements`, puis s'assure
qu'`IntBox` possède une méthode `Compare(int) -> int`. Une incompatibilité
produit un diagnostic précis nommant la méthode/le paramètre fautif et
son type attendu vs obtenu.

Les génériques sont toujours effacés au niveau C (`T → void*`) ; la
nouvelle vérification est un *contrat statique* superposé à la dispatch
duck-typed existante — il n'y a pas encore de vtable.

## Flux de contrôle

```kotlin
if (x > 0) {
    Console.WriteLine("positive")
} else if (x < 0) {
    Console.WriteLine("negative")
} else {
    Console.WriteLine("zero")
}

while (n > 0) {
    n = n - 1
}

for i in 0..10 { /* fin exclusive : 0..9 */ }
for c in characters { /* itère une List<T> */ }

break
continue
```

### Portée de bloc (`{ … }`, v0.8.35+)

Un `{ … }` nu en position d'instruction introduit sa propre portée.
Utile pour donner aux méthodes longues des sections visuellement
séparées où chaque section peut réutiliser des noms locaux sans
collision au niveau C :

```kotlin
public static void Run() {
    {
        let r0: int = 1
        let r1: int = 2
        // ... section A utilise r0, r1 ...
    }
    {
        let r0: int = 10   // OK — nouvelle portée, pas d'avertissement de masquage
        let r1: int = 20
        // ... section B réutilise les mêmes noms ...
    }
}
```

Chaque bloc émet un wrapper C `{ … }` correspondant, de sorte qu'un `let`
à l'intérieur d'un bloc est invisible pour le suivant. Correspond à la
portée C / C++ / Java ; nécessaire uniquement quand on veut un nouvel
emplacement pour le même nom. Les corps de `if` / `while` / `for` / bras
de `match` obtiennent automatiquement leur propre portée.

### Clauses de garde

```kotlin
public static int Clamp(int x, int lo, int hi) {
    guard x > lo else { return lo }
    guard x < hi else { return hi }
    return x
}
```

Se lit de haut en bas : si la condition est fausse, exécute le bloc
`else` (typiquement un `return` / `throw` / `break` / `continue`).
Équivalent à `if (!(cond)) { sortie }`.

### Filtrage par motif

```kotlin
match n {
    0           => Console.WriteLine("zero"),
    x if x < 0  => Console.WriteLine("negative"),         // garde de bras
    1..9        => Console.WriteLine("small"),            // plage
    10..99      => Console.WriteLine("medium"),
    _           => Console.WriteLine("else")              // joker
}
```

Les corps de bras sont des **instructions**, pas des expressions. Pour
calculer une valeur, utilisez un retour anticipé dans les bras ou
affectez dans chaque branche :

```kotlin
public string Classify(int n) {
    if (n == 0) { return "zero" }
    if (n < 0)  { return "negative" }
    return "positive"
}
```

Pour les énumérations algébriques, les motifs destructurent la charge
utile :

```kotlin
match shape {
    Circle(r)  => useRadius(r),
    Rect(w, h) => useDims(w, h)
}
```

### Try / catch / throw

```kotlin
try {
    risky()
} catch (e) {
    Console.WriteError("caught")
} finally {
    cleanup()
}
```

L'implémentation est basée sur `setjmp`/`longjmp` au niveau C — aucun
coût de déroulement de pile quand aucun throw ne se déclenche.

## Tuples

```kotlin
public class Math2 {
    public static (int, int) DivMod(int a, int b) {
        return (a / b, a % b)
    }
}

let (q, r) = Math2.DivMod(17, 5)   // q = 3, r = 2
```

## Lambdas

```kotlin
// Un paramètre, corps expression
let double = x => x * 2

// Plusieurs paramètres (v0.3.5)
let add  = (x, y) => x + y
let pick = (a, b, c) => a + b + c

// Corps bloc (v0.3.5) — liaisons let + return explicite
let plus3 = x => {
    let doubled = x * 2
    return doubled + 3
}

// Les closures capturent les locaux englobants par valeur (v0.3.4)
let n = 100
let shift = x => x + n           // capture n
```

Les lambdas compilent vers une fonction C de niveau supérieur plus une
valeur `AmalgameClosure { fn, env }` ; les locaux capturés sont
instantanés dans un env alloué sur le tas au site de création, pas par
substitution textuelle. Le runtime expose `Closure_call1` / `_call2` /
`_call3` pour les arités 1 à 3.

Les méthodes d'ordre supérieur de `List<T>` acceptent un lambda
directement (depuis la v0.3.6 — voir [04-stdlib.md](04-stdlib.md#listt)
pour la liste complète) :

```kotlin
let nums = new List<int>()
nums.Add(1) ; nums.Add(2) ; nums.Add(3) ; nums.Add(4)
let big   = nums.Filter(x => x > 2)              // [3, 4]
let times = nums.Map(x => x * 10)                // [10, 20, 30, 40]
let total = nums.Reduce(0, (acc, x) => acc + x)  // 10
```

### Fonctions de première classe (type `Closure`, v0.8.30+)

Utilisez le type `Closure` pour déclarer des **champs de classe**,
**paramètres de méthode** et **éléments de collection** dont la valeur
est une fonction. Les lambdas liés littéralement ou par nom à un
emplacement `Closure` sont utilisables comme n'importe quelle autre
valeur, et une closure stockée est invoquée avec la même syntaxe
`field(args)` qu'un appel de méthode — le compilateur passe par
`AmalgameClosure_callN` en coulisses.

```kotlin
public class Route {
    public Path:    string
    public Handler: Closure                  // ← champ fonction de première classe

    public Route(string path, Closure handler) {
        this.Path    = path
        this.Handler = handler
    }

    public int Run(int x) {
        return this.Handler(x)                // ← invoque la closure stockée
    }
}

let r = new Route("/double", x => x * 2)
r.Run(21)                                    // → 42

// Closures dans une collection.
let routes = new List<Route>()
routes.Add(new Route("/inc", x => x + 1))
let r0: Route = routes.Get(0)
r0.Run(99)                                   // → 100
```

C'est le fondement des API de callbacks (registres de handlers,
émetteurs d'événements, chaînes de middleware, etc.).

Les arités 1, 2 et 3 sont prises en charge. Les closures d'arité 0 ou
4+ nécessitent une future extension du cgen.

### Closures typées (`Closure<A, R>`, v0.8.35+)

Annotez la forme de la closure pour que le compilateur émette des casts
typés sur chaque argument et la valeur de retour. Le dernier paramètre
séparé par une virgule est **R** (le type de retour) ; les autres sont
les types d'arguments. Même convention que le `(A) -> R` de Kotlin,
simplement écrit à l'intérieur de `<…>`.

```kotlin
// Arité 1 : scalaire en entrée, scalaire en sortie.
let f1: Closure<int, int> = x => x * 2
let r1: int = f1(21)                          // 42

// Arité 2 : deux scalaires en entrée, un en sortie.
let f2: Closure<int, int, int> = (a, b) => a + b
let r2: int = f2(10, 20)                      // 30

// Argument pointeur + retour pointeur.
let f3: Closure<User, string> = u => u.Name
let r3: string = f3(new User("alice", 30))    // "alice"

// Closure typée comme champ de classe — le résultat de l'appel est typé pointeur.
public class Server {
    public Handler: Closure<Conn, Conn>
    public Server(h: Closure<Conn, Conn>) { this.Handler = h }
    public int Dispatch(c: Conn) {
        let r: Conn = this.Handler(c)         // ← cast typé, pas d'avertissement
        return r.Id
    }
}
```

L'ABI runtime est inchangée — `Closure<…>` est toujours abaissé en
`AmalgameClosure*`. L'annotation est purement un indice à la compilation
qui informe :

  - **Émission du corps lambda** — `u => u.Name` sur `Closure<User, string>`
    type `u` comme `User*` (c'était `i64 u = (i64)(intptr_t)__arg0;` pour
    un `Closure` nu).
  - **Casts au site d'appel** — les deux formes IDENTIFIER (`f(x)`) et
    MEMBER (`this.Handler(c)`) castent le résultat vers R au lieu de
    passer par `i64 + intptr_t`.
  - **Vérificateur de types** — `c(x)` où c est `Closure<…, R>` expose R
    comme type résultat de l'appel, donc `let r: R = c(x)` est
    correctement typé sans recourir au joker `?`.

Le code utilisateur qui avait précédemment besoin de `-Wno-int-conversion`
pour faire taire le bruit « pointer from integer » autour du boxing de
closure peut retirer ces flags dès que les champs/paramètres/locaux
`Closure` concernés portent leur forme typée.

**Arguments lambda directs (v0.8.36+).** L'inférence du paramètre lambda
s'active aussi quand le lambda est passé *directement* à un constructeur
ou une méthode dont le paramètre est déclaré `Closure<A, R>` — le
résolveur parcourt la signature cible et pousse A dans le premier PARAM
du lambda. Les trois formes ci-dessous produisent le même dépaquetage
typé sans bruit `-Wint-conversion` :

```kotlin
// Argument de constructeur direct — c est inféré comme WebContext depuis la signature de Route.
new Route(c => Program.handle(c))

// Argument de méthode — même chemin d'inférence.
reg.Register("name", c => Program.handle(c))

// Liaison vers un local typé — c est inféré depuis l'annotation du local.
let h: Closure<WebContext, HttpResponse> = c => Program.handle(c)
new Route(h)

// Ou typer le paramètre lambda explicitement — remplace l'inférence.
new Route((c: WebContext) => Program.handle(c))
```

## Littéraux de liste et compréhensions

La syntaxe entre crochets construit une `List<T>` en ligne. Les
expressions séparées par des virgules sont des littéraux ; la forme
`for x in iter` est une compréhension qui mappe et filtre optionnellement :

```kotlin
// Liste vide — le type d'élément vient de l'emplacement cible.
let empty: List<int> = []

// Littéral avec éléments explicites.
let names: List<string> = ["alpha", "beta", "gamma"]

// Mélange avec des valeurs calculées et une virgule finale.
let n = 7
let nums: List<int> = [
    n,
    n + 1,
    n * 2,
]

// Argument de fonction.
TakeList(["x", "y", "z"])

// Compréhension de liste — mappe une plage.
let squares: List<int> = [i * i for i in 0..10]
// → [0, 1, 4, 9, 16, 25, 36, 49, 64, 81]

// Compréhension avec filtre.
let evens: List<int> = [i for i in 0..20 if i % 2 == 0]

// Itère une liste existante — `x` est lié élément par élément.
let upper: List<string> = [String_ToUpper(name) for name in names]
```

Les deux formes sont abaissées vers la même structure — une expression
compound-statement GCC qui alloue un `AmalgameList*` frais et pousse
chaque élément boxé. Les littéraux sont équivalents à la série
`new List<T>(); list.Add(...)` longue forme — utilisez celle qui se lit
le mieux au site d'appel.

La compréhension prend en charge deux formes d'itérable :
- une plage numérique (`lo..hi`) — émet une boucle `i64` comptée ;
- toute valeur `List<T>` — émet une boucle indexée sur la liste.

L'imbrication avec le même nom (`[ [j for j in 0..i] for i in 0..3 ]`)
n'est pas encore prise en charge — choisissez des variables de boucle
distinctes lors de l'imbrication.

**Opérateur de déploiement** `...src` (v0.8.36+). À l'intérieur d'un
littéral de liste, chaque élément `...src` insère les éléments de `src`
à sa position :

```kotlin
let a: List<int> = [1, 2, 3]
let b: List<int> = [4, 5]
let c: List<int> = [...a, ...b, 99]
// → [1, 2, 3, 4, 5, 99]  (Count() == 6)

// Mélange de déploiements avec des éléments littéraux en toute position.
let names = ["alice", "bob"]
let all   = ["zero", ...names, "tail"]
// → ["zero", "alice", "bob", "tail"]

// Déploie le résultat d'un appel de méthode — la source est évaluée une seule fois
// (liée à un local synthétique `__sp_<i>` dans le C généré).
let doubled = [...nums.Map(x => x * 2), 999]
```

**Paramètres variadiques** `...name: ElemType` (v0.8.61+). Le dernier
paramètre d'une méthode peut être variadique ; à l'intérieur du corps
c'est une `List<ElemType>`. Les appelants passent zéro ou plusieurs
arguments à cette position, ou déploient une liste existante avec
`...expr` :

```amalgame
public static int Sum(...nums: int) {
    var total: int = 0
    for n in nums { total = total + n }     // nums est une List<int>
    return total
}

public static int SumFrom(int base, ...nums: int) { ... }   // fixe + queue variadique

Sum()                  // 0   — zéro argument → liste vide
Sum(1, 2, 3, 4)        // 10  — arguments en ligne
let xs = new List<int>()
Sum(...xs)             // déploie une liste dans l'emplacement variadique
```

Les constructeurs peuvent aussi être variadiques (`Bag(...nums: int)`,
v0.8.63+).

**Async / await** `async fn` + `await` (v0.8.70). Un `async fn` renvoie
un future ; `await` le déballe. Le sucre est abaissé sur le runtime
`amalgame-async` (donc un programme qui l'utilise dépend de ce package).
La couverture complète — fibers, channels, le scheduler — est dans
[13-async.md](13-async.md) :

```amalgame
public async fn Compute(int n): int { return n * 2 }
// let f = w.Compute(21);  let r: int = await f   // r == 42
```

**Génériques imbriqués** `List<List<T>>` (v0.8.36+). Le chaînage
`.Get(i).Get(j)` déroule une couche générique par saut, de sorte que
chaque niveau obtient un cast typé plutôt que de tomber en `void*` :

```kotlin
let rows: List<List<string>> = …
let s: string = rows.Get(0).Get(0)   // cast code_string approprié

let cube: List<List<List<int>>> = …
let v: int = cube.Get(0).Get(0).Get(0)  // unbox via intptr_t
```

Fonctionne jusqu'à une profondeur arbitraire, aussi bien pour `List<…>`
que pour la moitié valeur de `Map<K, …>`. Le type d'élément brut est
suivi à chaque annotation `let` et chaque retour de méthode typé.

**Méthodes d'ordre supérieur sur `List<Classe>` (v0.8.35+).** Aussi bien
`xs.Filter(x => x > 0)` que `xs.Map(x => x.Name)` sur une `List<User>`
fonctionnent maintenant de bout en bout. Le résolveur corrige le premier
paramètre du lambda depuis le type d'élément du receveur, le cgen émet
un dépaquetage typé (`User* x = (User*)__arg0;`) et le côté consommateur
`for u in filterResult` est abaissé avec le bon cast d'élément (aucun
`as User*` manuel nécessaire). Les listes non typées (`new List<int>()`
sans annotation) retombent toujours sur `void*` et nécessitent des casts
explicites.

## Sûreté vis-à-vis de null

`T?` déclare un type nullable. `?.` court-circuite vers `null` quand le
receveur est null :

```kotlin
let user: User? = null
let name = user?.Name              // null
let len  = user?.GetNameLength()   // null
if (name == null) {
    Console.WriteLine("anonymous")
}
```

`??` est la coalescence de null (renvoie l'opérande droit quand le
gauche est null) :

```kotlin
let display = user?.Name ?? "anonymous"
```

## Décorateurs

```kotlin
public class Math2 {
    @inline
    public static int Square(int x) { return x * x }

    @deprecated
    public static int Old(int x) { return x }
}
```

| Décorateur    | Effet sur le C émis                                              |
| ------------- | ---------------------------------------------------------------- |
| `@inline`     | ajoute `inline` à la définition de fonction                      |
| `@deprecated` | ajoute `__attribute__((deprecated))` au prototype                |

Les autres décorateurs sont acceptés par le parser mais sont actuellement
sans effet.

## Arguments nommés

```kotlin
let p = new Person(name: "Bastien", age: 31)
Math2.Clamp(x: 50, lo: 0, hi: 10)
```

> Les noms sont **uniquement documentaires** au site d'appel pour
> l'instant : les valeurs sont passées dans l'ordre de la source
> indépendamment des noms. Suivi dans
> [ROADMAP_COMPLET.md](../../ROADMAP_COMPLET.md).

## Mode bibliothèque

Les fichiers sans `Program.Main` compilent automatiquement comme des
bibliothèques. Utilisez `--lib` pour forcer le mode bibliothèque même
quand un `Program.Main` existe.

```bash
./amc --lib mylib.am -o mylib
gcc -Iruntime -c mylib.c -o mylib.o
# Liez mylib.o dans un programme hôte depuis C, Amalgame, etc.
```

## Lire les diagnostics

Les erreurs viennent avec un extrait source et un caret, comme rustc :

```
error[typechecker]: Cannot assign 'string' to 'n' of type 'int'
  --> /tmp/test.am:19:13
   |
19 |         let n: int = p.Name
   |             ^

error[resolver]: Unknown symbol 'someThing'
  --> /tmp/test.am:4:9
  |
4 |         someThing.x()
  |         ^
```

Les deux passes sont :

- **Résolveur** — résolution de portée et de noms (identifiants inconnus,
  types non liés, réaffectation d'une liaison immuable).
- **Vérificateur de types** — assignabilité, types de retour, types
  d'accès aux membres via la MemberTable du résolveur.

Les deux erreurs sont individuellement non-fatales ; le compilateur émet
quand même un fichier `.c` quand c'est possible (pour que toutes les
erreurs soient visibles d'un coup), mais sort avec un code non nul dès
qu'une erreur a été signalée.

## Ce qui n'est pas encore dans le langage

- Inférence de type générique (`let xs = new List<int>()` compile bien
  mais le type d'élément `int` n'est pas propagé à travers chaque appel
  de méthode — voir [04-stdlib.md](04-stdlib.md#listt) pour les méthodes
  qui préservent le type d'élément).
- L'imbrication de compréhensions de liste avec le même nom (utilisez
  des variables de boucle distinctes).

Voir [ROADMAP_COMPLET.md](../../ROADMAP_COMPLET.md) pour le backlog
complet.
