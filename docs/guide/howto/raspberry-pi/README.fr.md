# Raspberry Pi

Un parcours concret, de la carte SD vierge jusqu'à un programme
Amalgame qui fait clignoter une LED, lit un bouton et dialogue avec un
capteur I²C sur un **Raspberry Pi**.

Amalgame compile en code natif ARM64 : un Pi exécute donc le *même*
langage que sur votre portable — pas de cross-compilateur, pas de
gymnastique de toolchain. Vous écrivez du `.am`, vous faites
`amc build`, vous obtenez un vrai binaire.

> 🥧 **Ce qu'il vous faut**
> - Un Raspberry Pi (n'importe quel modèle, **1 → 5**) avec un OS
>   **64 bits**.
> - Quelques fils, une LED + une résistance 330 Ω, un bouton-poussoir.
>   L'exemple I²C demande n'importe quel périphérique I²C (un
>   MPU-6050, un petit OLED, un backpack LCD PCF8574 — tout ce qui
>   répond sur le bus).

## Les trois étapes

1. **[Préparer le Pi](01-setup.md)** — flasher un OS 64 bits, installer
   `amc`, activer les interfaces I²C/SPI et installer la dépendance
   système `libgpiod`. À faire une fois.
2. **[Votre premier programme](02-first-program.md)** — créer un projet,
   ajouter le package `amalgame-hardware-gpio`, faire clignoter une
   LED. ~5 minutes.
3. **[Exemples](examples/README.md)** — copier-coller, câbler, lancer :
   - [Clignotement d'une LED](examples/01-blink.md)
   - [Lire un bouton (événements de front)](examples/02-button.md)
   - [Scanner un bus de capteurs I²C](examples/03-sensor-i2c.md)
   - [Piloter une barre de LEDs (chenillard)](examples/04-led-bar.md)

## Quel package ?

Tout ici repose sur **`amalgame-hardware-gpio`**, le premier membre de
la famille `Amalgame.Hardware`. Un package, cinq périphériques :

| Périphérique | Couverture |
|---|---|
| **GPIO** | entrées/sorties numériques, pull-ups internes, fronts (interruptions) |
| **I²C** | maître sur `/dev/i2c-N` — capteurs, afficheurs, expandeurs |
| **SPI** | maître sur `/dev/spidev` — afficheurs rapides, ADC |
| **PWM** | PWM matériel via sysfs — servos, gradation, tonalités |
| **UART** | série via `termios` — GPS, modems, autres cartes |

Il s'appuie sur **libgpiod v2** sur le périphérique caractère GPIO
(`/dev/gpiochip*`) — l'interface moderne bénie par le noyau, qui
fonctionne sur *tous* les Pi, y compris le Pi 5 (dont le GPIO est
derrière la puce RP1, où la vieille astuce `/dev/gpiomem` ne marche
plus).

> Cette même API de broches reflète le HAL `Amalgame.Mcu` prévu : le
> code écrit ici se lira presque à l'identique plus tard sur un
> microcontrôleur nu. (Une section `How To → MCU` suivra.)

`hardware-gpio` est le **backend Raspberry Pi**. Par-dessus vit toute
une famille de drivers de composants portables (LEDs, moteurs,
capteurs, afficheurs, …) — voir le **[catalogue des composants matériels](../hardware-components.md)**.

Commencez par **[Préparer le Pi](01-setup.md)**.
