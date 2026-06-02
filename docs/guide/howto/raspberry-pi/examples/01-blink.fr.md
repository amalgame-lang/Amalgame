# Clignotement d'une LED

Le « hello world » du GPIO : allumer et éteindre une LED.

## Câblage

```
GPIO17 ──▶│── 330Ω ──┐
         LED         │
                    GND
```

Anode de la LED → **GPIO17**, cathode → **330 Ω** → **GND**.

## Code

```amalgame
namespace Demo

import Amalgame.Hardware
import Amalgame.DateTime

public class Program {
    public static void Main(string[] args) {
        let led: int = 17

        if (!Gpio.PinMode(led, PinMode.Output)) {
            Console.WriteLine("impossible d'ouvrir GPIO17 — vérifiez les permissions")
            return
        }

        var i: int = 0
        while (i < 20) {
            Gpio.Toggle(led)               // bascule la sortie
            DateTime.SleepMillis(500)      // ...toutes les demi-secondes
            i = i + 1
        }

        Gpio.Close()                       // libère la ligne
    }
}
```

## Lancer

```sh
amc package add datetime     # une fois — pour DateTime.SleepMillis
amc build main.am -o blink
./blink
```

## Ce qui se passe

- `Gpio.PinMode(17, PinMode.Output)` demande GPIO17 en sortie. Il
  renvoie `false` (plutôt que de planter) si la ligne est occupée ou
  si vous manquez de permissions — vérifiez toujours.
- `Gpio.Toggle(17)` inverse le niveau courant. Vous pourriez être
  explicite avec `Gpio.DigitalWrite(17, Level.High)` / `Level.Low`.
- `Gpio.Close()` libère toutes les lignes demandées par le programme.
  Le noyau les récupérerait de toute façon à la sortie, mais fermer
  proprement est plus net.
- `DateTime.SleepMillis(500)` met en pause une demi-seconde. Vient du
  package [`amalgame-datetime`](https://github.com/amalgame-lang/amalgame-datetime)
  (`DateTime.SleepSeconds(s)` et `Duration.Sleep()` y sont aussi) —
  du pur Amalgame, aucun C inline dans votre programme.

**Suivant :** [lire un bouton →](02-button.md)
