# Exemples

Quatre petits programmes complets. Chacun est autonome : câblez comme
indiqué, mettez le code dans `main.am`, `amc build main.am -o demo`,
lancez. Tous supposent que vous avez fini la
[préparation](../01-setup.md) et le
[premier programme](../02-first-program.md) (donc que le package
`hardware-gpio` est déjà dans votre projet).

| Exemple | Périphérique | Ce que vous apprenez |
|---|---|---|
| **[Clignotement d'une LED](01-blink.md)** | sortie GPIO | piloter une broche, `Toggle`, la boucle d'exécution |
| **[Lire un bouton](02-button.md)** | fronts GPIO | détection de fronts noyau, sans busy-wait |
| **[Scanner un bus I²C](03-sensor-i2c.md)** | maître I²C | ouvrir un bus, sonder les périphériques, lire un registre |
| **[Chenillard de LEDs](04-led-bar.md)** | plusieurs sorties GPIO | plusieurs broches, un motif animé |

> Chaque exemple utilise les numéros GPIO **BCM**. Les schémas donnent
> le numéro BCM ; si vous préférez les broches physiques, GPIO17 =
> broche 11, GPIO27 = broche 13, GPIO22 = broche 15, etc.
