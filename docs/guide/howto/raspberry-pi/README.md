# Raspberry Pi

This is a hands-on path from a blank SD card to a running Amalgame
program that blinks an LED, reads a button, and talks to an I²C
sensor on a **Raspberry Pi**.

Amalgame compiles to native ARM64 code, so a Pi runs the *same*
language you use on your laptop — no cross-compiler, no toolchain
gymnastics. You write `.am`, you `amc build`, you get a real binary.

> 🥧 **What you need**
> - A Raspberry Pi (any model, **1 → 5**) running a **64-bit OS**.
> - A few jumper wires, an LED + a 330 Ω resistor, a push-button.
>   The I²C example wants any I²C device (an MPU-6050, a small OLED, a
>   PCF8574 LCD backpack — anything that ACKs on the bus).

## The three steps

1. **[Set up your Pi](01-setup.md)** — flash a 64-bit OS, install `amc`,
   enable the I²C/SPI interfaces, and install the `libgpiod` system
   dependency. Do this once.
2. **[Your first program](02-first-program.md)** — create a project, add
   the `amalgame-hardware-gpio` package, and blink an LED. ~5 minutes.
3. **[Examples](examples/README.md)** — copy-paste, wire up, run:
   - [Blink an LED](examples/01-blink.md)
   - [Read a button (edge events)](examples/02-button.md)
   - [Scan an I²C sensor bus](examples/03-sensor-i2c.md)
   - [Drive an LED bar (chase)](examples/04-led-bar.md)

## Which package?

Everything here uses **`amalgame-hardware-gpio`**, the first member of
the `Amalgame.Hardware` family. One package, five peripherals:

| Peripheral | What it covers |
|---|---|
| **GPIO** | digital in/out, internal pull-ups, edge events (interrupts) |
| **I²C** | master over `/dev/i2c-N` — sensors, displays, expanders |
| **SPI** | master over `/dev/spidev` — fast displays, ADCs |
| **PWM** | hardware PWM via sysfs — servos, dimming, tones |
| **UART** | serial over `termios` — GPS, modems, other boards |

It is backed by **libgpiod v2** on the GPIO character device
(`/dev/gpiochip*`) — the modern, kernel-blessed interface that works
on *every* Pi, including the Pi 5 (whose GPIO sits behind the RP1
chip, where the old `/dev/gpiomem` trick no longer works).

> The same pin API mirrors the planned `Amalgame.Mcu` HAL, so the code
> you write here will later read almost identically on a bare-metal
> microcontroller. (A `How To → MCU` section will follow.)

`hardware-gpio` is the **Raspberry Pi backend**. On top of it sits a
whole family of portable component drivers (LEDs, motors, sensors,
displays, …) — see the full **[Hardware components catalogue](../hardware-components.md)**.

Start with **[Set up your Pi](01-setup.md)**.
