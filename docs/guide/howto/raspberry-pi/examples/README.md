# Examples

Four small, complete programs. Each one is self-contained: wire it as
shown, drop the code in `main.am`, `amc build main.am -o demo`, run.
They all assume you've finished the [setup](../01-setup.md) and
[first program](../02-first-program.md) (so the `hardware-gpio` package
is already in your project).

| Example | Peripheral | What you'll learn |
|---|---|---|
| **[Blink an LED](01-blink.md)** | GPIO output | drive a pin, `Toggle`, the run loop |
| **[Read a button](02-button.md)** | GPIO edge events | kernel edge detection, no busy-wait |
| **[Scan an I²C bus](03-sensor-i2c.md)** | I²C master | open a bus, probe for devices, read a register |
| **[LED bar chase](04-led-bar.md)** | several GPIO outputs | many pins, a moving pattern |

> Every example uses **BCM** GPIO numbers. Wiring diagrams give the
> BCM number; if you prefer physical pins, GPIO17 = pin 11,
> GPIO27 = pin 13, GPIO22 = pin 15, and so on.
