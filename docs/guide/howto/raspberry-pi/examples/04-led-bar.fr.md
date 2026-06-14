# Chenillard de LEDs

Piloter plusieurs LEDs à la fois et faire un « chenillard » — un point
lumineux qui balaie d'avant en arrière, comme le scanner de K2000. Ça
montre comment gérer plusieurs lignes GPIO depuis un seul programme.

## Câblage

Câblez **quatre** LEDs, chacune via sa propre résistance 330 Ω vers
GND, sur les GPIO **17, 27, 22, 23** :

```
GPIO17 ──▶│── 330Ω ── GND
GPIO27 ──▶│── 330Ω ── GND
GPIO22 ──▶│── 330Ω ── GND
GPIO23 ──▶│── 330Ω ── GND
```

(Mettez-en autant que vous voulez — il suffit d'étendre la liste dans
le code.)

## Code

```amalgame
namespace Demo

import Amalgame.Hardware
import Amalgame.DateTime

public class Program {
    public static void Main(string[] args) {
        let pins: List<int> = [17, 27, 22, 23]

        // Demande chaque broche en sortie.
        for p in pins {
            if (!Gpio.PinMode(p, PinMode.Output)) {
                Console.WriteLine("impossible d'ouvrir GPIO" + String_FromInt(p))
                return
            }
            Gpio.DigitalWrite(p, Level.Low)
        }

        // Une seule LED allumée à la fois, balayage aller puis retour.
        var sweeps: int = 0
        while (sweeps < 6) {
            var i: int = 0
            while (i < pins.Count()) {
                Program.LightOnly(pins, i)
                DateTime.SleepMillis(120)
                i = i + 1
            }
            // retour (on saute les extrémités pour éviter le double clignotement)
            i = pins.Count() - 2
            while (i > 0) {
                Program.LightOnly(pins, i)
                DateTime.SleepMillis(120)
                i = i - 1
            }
            sweeps = sweeps + 1
        }

        Gpio.Close()
    }

    // Allume la broche d'indice `on`, éteint toutes les autres.
    private static void LightOnly(pins: List<int>, on: int) {
        var i: int = 0
        while (i < pins.Count()) {
            if (i == on) {
                Gpio.DigitalWrite(pins.Get(i), Level.High)
            } else {
                Gpio.DigitalWrite(pins.Get(i), Level.Low)
            }
            i = i + 1
        }
    }
}
```

## Lancer

```sh
amc package add datetime     # une fois — pour DateTime.SleepMillis
amc build main.am -o ledbar
./ledbar
```

Une seule LED allumée balaie la barre dans un sens puis dans l'autre,
six fois.

## Ce qui se passe

- Les broches vivent dans un `List<int>` : ajouter des LEDs revient
  donc à éditer la liste — les boucles ne changent pas.
- `LightOnly` parcourt la liste et met une broche à `High`, les autres
  à `Low`. Remplacez-le par votre propre motif (remplissage,
  clignotement global, compteur binaire…) pour d'autres effets.
- Tout est en numérique pur `DigitalWrite`. Pour une *luminosité*
  douce (un fondu plutôt que tout ou rien), vous utiliseriez le
  **PWM** — `new Pwm(0, 0)` dans le même package — mais cela demande
  une broche compatible PWM et l'overlay `dtoverlay=pwm`.

← Retour à l'[index des exemples](README.md) ·
[Vue d'ensemble Raspberry Pi](../README.md)
