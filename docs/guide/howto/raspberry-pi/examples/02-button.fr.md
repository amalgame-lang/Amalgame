# Lire un bouton (événements de front)

Détecter les appuis de la *bonne* manière : demander au noyau de
surveiller la broche et de réveiller votre programme uniquement quand
le niveau change vraiment — pas d'attente active dans une boucle
`while`, et des horodatages à la microseconde directement issus du
pilote.

## Câblage

```
GPIO27 ──┬── bouton ── GND
         │
   (un pull-up interne/carte maintient le niveau Haut ; l'appui tire vers le Bas)
```

Câblez un bouton-poussoir entre **GPIO27** et **GND**. Ajoutez une
résistance de pull-up externe, ou comptez sur le défaut de la carte —
`WatchEdge` lui-même ne pose aucun bias interne.

## Code

```amalgame
namespace Demo

import Amalgame.Hardware

public class Program {
    public static void Main(string[] args) {
        let button: int = 27

        if (!Gpio.WatchEdge(button, Edge.Both)) {
            Console.WriteLine("impossible de surveiller GPIO27 — vérifiez les permissions")
            return
        }
        Console.WriteLine("surveillance de GPIO27 (10 fronts puis sortie)…")

        var seen: int = 0
        while (seen < 10) {
            let ev: GpioEvent = Gpio.WaitEdge(5000)   // bloque jusqu'à 5s
            if (ev.IsTimeout()) {
                Console.WriteLine("(inactif 5s)")
            } else {
                var what: string = "descendant"
                if (ev.KindOf() == Edge.Rising) { what = "montant" }
                Console.WriteLine("front " + what + " sur GPIO" +
                                  String_FromInt(ev.PinOf()) + " @ " +
                                  String_FromInt(ev.TimestampNsOf()) + "ns")
                seen = seen + 1
            }
        }

        Gpio.Close()
    }
}
```

## Lancer

```sh
amc build main.am -o button
./button
```

Appuyez quelques fois sur le bouton ; chaque appui et relâchement
affiche une ligne.

## Ce qui se passe

- `Gpio.WatchEdge(27, Edge.Both)` arme la détection de fronts du
  noyau. Utilisez `Edge.Rising`, `Edge.Falling` ou `Edge.Both`.
- `Gpio.WaitEdge(5000)` bloque (en dormant dans le noyau, coût CPU
  nul) jusqu'au prochain front, ou jusqu'à 5000 ms. En cas de timeout
  il renvoie une sentinelle dont `IsTimeout()` vaut `true`.
- Un `GpioEvent` livré vous donne `KindOf()` (`Rising`/`Falling`),
  `PinOf()` et `TimestampNsOf()` (nanosecondes monotones du noyau).
- Vous préférez **drainer sans bloquer** ? `Gpio.PollEdges()` renvoie
  un `List<GpioEvent>` de tout ce qui est en file, du plus ancien au
  plus récent, sans attendre.

C'est le motif type interruption — bien meilleur que lire
`Gpio.DigitalRead(27)` dans une boucle serrée, qui brûle un cœur et
peut rater des appuis rapides.

**Suivant :** [scanner un bus I²C →](03-sensor-i2c.md)
