# Votre premier programme

Vous avez [préparé le Pi](01-setup.md). Créons un projet, tirons le
package matériel, et faisons clignoter une LED.

## Câbler une LED

```
GPIO17 ──▶│── 330Ω ──┐
         LED         │
                    GND
```

La patte longue de la LED (anode) va sur **GPIO17** (numérotation
BCM), la patte courte (cathode) passe par une résistance **330 Ω**
jusqu'à n'importe quelle broche **GND**.

> **Numérotation BCM.** Amalgame utilise les numéros GPIO Broadcom (les
> mêmes que `gpiozero` et la plupart des tutoriels), *pas* les
> positions physiques des broches. GPIO17 = broche physique 11.

## Créer un projet

```sh
mkdir blink && cd blink
amc new .              # génère amalgame.toml + main.am
```

## Ajouter le package matériel

```sh
amc package add hardware-gpio
```

Cela écrit la dépendance dans `amalgame.toml`, la résout, et
enregistre la révision exacte dans `amalgame.lock` :

```toml
[dependencies.hardware-gpio]
git = "https://github.com/amalgame-lang/amalgame-hardware-gpio"
tag = "v0.6.0"
```

> Nécessite **amc ≥ 0.8.72**. Lancez `amc --version` ; s'il est plus
> ancien, relancez la
> [commande d'installation](01-setup.md#2--installer-amc) pour le mettre
> à jour.

## L'écrire

Mettez ceci dans `main.am` :

```amalgame
namespace Demo

import Amalgame.Hardware
import Amalgame.DateTime    // pour la pause entre deux bascules

public class Program {
    public static void Main(string[] args) {
        Console.WriteLine("backend libgpiod : " + Gpio.Backend())

        let led: int = 17
        if (!Gpio.PinMode(led, PinMode.Output)) {
            Console.WriteLine("impossible d'ouvrir GPIO17 — vérifiez les permissions")
            return
        }

        var i: int = 0
        while (i < 20) {
            Gpio.Toggle(led)
            DateTime.SleepMillis(500)
            i = i + 1
        }

        Gpio.Close()
    }
}
```

> `DateTime.SleepMillis` vient du package
> [`amalgame-datetime`](https://github.com/amalgame-lang/amalgame-datetime)
> — ajoutez-le aussi : `amc package add datetime`. (Aucun C inline : le
> sleep vit dans le package, écrit en Amalgame par-dessus un unique
> appel FFI `nanosleep`.)

## Compiler et lancer

```sh
amc build main.am -o blink
./blink            # ou : sudo ./blink  (si vous n'êtes pas dans le groupe 'gpio')
```

La LED clignote dix fois sur dix secondes, puis le programme se
termine et le noyau récupère la ligne.

Si vous voyez `impossible d'ouvrir GPIO17`, c'est presque toujours une
question de permissions — relisez
[l'étape 5 de la préparation](01-setup.md#5--permissions-les-groupes-gpioi2cspi)
ou lancez simplement avec `sudo`.

---

Et voilà — vous pilotez du vrai matériel depuis Amalgame. Parcourez
maintenant les **[exemples](examples/README.md)** : un bouton anti-rebond
côté noyau, un scan de capteurs I²C et un chenillard multi-LEDs.
