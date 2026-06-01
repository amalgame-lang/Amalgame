# Async et concurrence

Amalgame propose deux couches de concurrence coopérative, et elles
s'articulent ensemble :

1. **`async fn` / `await`** — sucre syntaxique intégré au compilateur
   (livré en v0.8.70). Il fait qu'une méthode renvoie un *future* et vous
   permet de l'`await` avec une syntaxe linéaire.
2. **`amalgame-async`** — le package runtime (`Amalgame.Async`) sur lequel
   le sucre est abaissé : fibers, channels et un scheduler coopératif. Vous
   pouvez aussi l'utiliser directement.

> **Point clé, à ne pas sauter.** `async fn` / `await` n'est *pas*
> autonome — il fait l'objet d'un lowering vers des appels runtime
> `Amalgame.Async` (le scheduler + les channels).
> Un programme qui utilise `async`/`await` **doit dépendre du package
> `amalgame-async`** pour que ces symboles soient liés. Ajoutez-le en
> premier :
>
> ```bash
> amc package add async
> ```
>
> Sans cela, vous verrez des avertissements `implicit declaration of
> 'Amalgame_Async_*'` à l'édition de liens et **aucun binaire n'est
> produit**.

Il s'agit de concurrence coopérative (fibers mono-thread) : les fibers se
cèdent la main mutuellement aux points `await` / channel / sleep. Ce n'est
pas du parallélisme par threads OS — pour cela, voyez le package
`amalgame-threading`.

---

## `async fn` / `await`

Marquez une méthode `async fn`. Elle renvoie immédiatement un *future* ;
`await` déballe le résultat éventuel, typé selon le type de retour déclaré
de la méthode :

```amalgame
import Amalgame.Collections
import Amalgame.String

public class Worker {
    public async fn Compute(int n): int {
        return n * 2
    }
}

public class Program {
    public static void Main(string[] args) {
        let w = new Worker()
        let f = w.Compute(21)      // renvoie immédiatement un future
        let r: int = await f       // attend le résultat → 42
        Console.WriteLine("result = " + String_FromInt(r))
    }
}
```

```bash
amc package add async              # requis — le sucre est lié contre lui
amc build main.am -o main
./main                             # result = 42
```

En coulisses (v0.8.70), un `async fn Foo` fait l'objet d'un lowering vers
trois fonctions C : le véritable corps, un trampoline de fiber qui envoie
le résultat dans un channel de future, et un wrapper public (qui conserve
le nom normal) qui lance la fiber et renvoie le future. Ainsi un site
d'appel reste un appel ordinaire, et `await` devient une réception sur
channel déballée vers le type de retour. Vous n'écrivez rien de tout cela —
mais le savoir explique pourquoi le package runtime est obligatoire.

> `async fn` et `fn` ne diffèrent que par le mot-clé `async` ; le `fn`
> après `async` est lui-même optionnel (`async Compute(int n): int`
> s'analyse aussi).

---

## Le runtime directement — fibers, channels, scheduler

Le package `amalgame-async` (`import Amalgame.Async`, classe `Async`)
expose une API **plate** — `Async.FiberSpawn`, `Async.ChannelNew`, etc.
(pas `Fiber.Spawn`). Lancez des fibers, reliez-les avec un channel, puis
faites tourner le scheduler. Ce producteur/consommateur provient du README
du package lui-même :

```amalgame
import Amalgame.Async

public class Program {
    public static void Main(string[] args) {
        let ch = Async.ChannelNew(8)          // FIFO borné, capacité 8

        let producer = (_x: int) => {
            var i: int = 0
            while (i < 5) {
                Async.ChannelSend(ch, 100 + i)  // se met en pause si plein
                i = i + 1
            }
            Async.ChannelClose(ch)              // réveille les récepteurs en pause
            return 0
        }

        let consumer = (_x: int) => {
            while (true) {
                let v: int = Async.ChannelReceive(ch)   // se met en pause si vide
                if (v == 0 && Async.ChannelIsClosed(ch)) { break }
                Console.WriteLine("got " + String_FromInt(v))
            }
            return 0
        }

        Async.FiberSpawn(producer, 0)
        Async.FiberSpawn(consumer, 0)
        Async.SchedulerRun()                  // pompe jusqu'à ce que toutes les fibers finissent
    }
}
```

### L'API en un coup d'œil (v0.2)

**Fibers**

| Appel | Renvoie | Notes |
|------|---------|-------|
| `Async.FiberSpawn(closure, arg)` | `AmalgameFiber*` | met en file une closure à 1 argument ; s'exécute quand le scheduler la pompe |
| `Async.FiberYield()` | `void` | cession coopérative — l'appelant passe en queue de la file des prêts |
| `Async.FiberSleep(ms)` | `void` | met l'appelant en pause ; le scheduler exécute les autres entre-temps |
| `Async.FiberCurrentId()` | `int` | id de la fiber en cours ; 0 en dehors de toute fiber |

**Channels** — FIFO borné, sensible au scheduler

| Appel | Renvoie | Notes |
|------|---------|-------|
| `Async.ChannelNew(capacity)` | `AmalgameAsyncChannel*` | capacité ≥ 1 |
| `Async.ChannelSend(ch, value)` | `bool` | **se met en pause si plein** ; false si fermé |
| `Async.ChannelReceive(ch)` | `int` | **se met en pause si vide** ; **renvoie 0** si fermé + vide |
| `Async.ChannelTrySend(ch, value)` | `bool` | non-bloquant ; false si plein/fermé |
| `Async.ChannelTryReceive(ch)` | `int` | non-bloquant ; 0 si vide |
| `Async.ChannelClose(ch)` | `void` | réveille toutes les fibers en pause |
| `Async.ChannelIsClosed(ch)` | `bool` | |
| `Async.ChannelCount(ch)` / `Async.ChannelCapacity(ch)` | `int` | nombre d'éléments en tampon / capacité |

**Scheduler**

| Appel | Renvoie | Notes |
|------|---------|-------|
| `Async.SchedulerRun()` | `void` | pompe jusqu'à ce que les files prêts + endormis + en attente + fd-wait soient toutes vides |
| `Async.SchedulerRunUntil(ms)` | `void` | idem, mais s'arrête après `ms` |
| `Async.SchedulerPending()` | `int` | fibers encore vivantes |

**I/O non-bloquantes** (Linux, basées sur epoll)

| Appel | Renvoie | Notes |
|------|---------|-------|
| `Async.WaitFdReadable(fd, timeout_ms)` | `bool` | se met en pause jusqu'à ce que `fd` soit lisible ; timeout négatif = pour toujours. **Doit être à l'intérieur d'une fiber.** |
| `Async.WaitFdWritable(fd, timeout_ms)` | `bool` | idem pour la disponibilité en écriture |
| `Async.MakeNonBlocking(fd)` | `bool` | positionne `O_NONBLOCK` — le compagnon obligatoire de `WaitFd*` |

---

## Pièges

- **`async`/`await` requiert le package `amalgame-async`.** Le sucre est
  dans le compilateur, mais il *fait l'objet d'un lowering vers* des appels
  runtime — `amc package add async`, sinon vous obtenez des
  `Amalgame_Async_*` non résolus à l'édition de liens et aucun binaire.
- **`ChannelReceive` renvoie `0` comme sentinelle de fermé-et-vide.** Si
  `0` est une valeur légale dans votre flux, associez la réception à
  `Async.ChannelIsClosed(ch)` (comme le fait le consommateur ci-dessus)
  pour distinguer le « vrai zéro » du « channel terminé ».
- **Rien ne s'exécute avant `SchedulerRun()`.** `FiberSpawn` ne fait que
  mettre en file ; c'est la pompe du scheduler qui exécute les fibers. Un
  programme qui lance des fibers mais ne pompe jamais ne fait rien.
- **Coopératif, pas parallèle.** Une fiber monopolise le thread jusqu'à ce
  qu'elle atteigne un `await` / une opération de channel / `FiberYield` /
  `FiberSleep`. Les boucles gourmandes en CPU devraient appeler
  `FiberYield()` périodiquement. Pour du vrai parallélisme, utilisez
  `amalgame-threading`.
- **`WaitFd*` est réservé aux fibers et propre à Linux/epoll.** L'appeler
  en dehors d'une fiber, ou compter dessus sur une cible non-Linux, ne se
  comportera pas comme sur Linux.

Pour les threads OS et les mutex, voyez le package `amalgame-threading`.
Pour le workflow des packages (`add` / `update` / lockfile), voyez
[12-packages.fr.md](12-packages.fr.md).
