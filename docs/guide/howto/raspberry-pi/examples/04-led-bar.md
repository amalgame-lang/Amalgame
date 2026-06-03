# LED bar chase

Drive several LEDs at once and run a "chase" — a lit dot that sweeps
back and forth, like a Knight Rider scanner. It shows how to manage
many GPIO lines from one program.

## Wiring

Wire **four** LEDs, each through its own 330 Ω resistor to GND, on
GPIO **17, 27, 22, 23**:

```
GPIO17 ──▶│── 330Ω ── GND
GPIO27 ──▶│── 330Ω ── GND
GPIO22 ──▶│── 330Ω ── GND
GPIO23 ──▶│── 330Ω ── GND
```

(Use as many as you like — just extend the list in the code.)

## Code

```amalgame
namespace Demo

import Amalgame.Hardware
import Amalgame.DateTime

public class Program {
    public static void Main(string[] args) {
        let pins: List<int> = [17, 27, 22, 23]

        // Request every pin as an output.
        for p in pins {
            if (!Gpio.PinMode(p, PinMode.Output)) {
                Console.WriteLine("could not open GPIO" + String_FromInt(p))
                return
            }
            Gpio.DigitalWrite(p, Level.Low)
        }

        // Light exactly one LED at a time, sweeping forward then back.
        var sweeps: int = 0
        while (sweeps < 6) {
            var i: int = 0
            while (i < pins.Count()) {
                Program.LightOnly(pins, i)
                DateTime.SleepMillis(120)
                i = i + 1
            }
            // back down (skip the ends so they don't double-blink)
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

    // Drive pin index `on` High, all the others Low.
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

## Run

```sh
amc package add datetime     # once — for DateTime.SleepMillis
amc build main.am -o ledbar
./ledbar
```

A single lit LED sweeps across the bar and back, six times.

## What's happening

- The pins live in a `List<int>`, so adding LEDs is just editing the
  list — the loops don't change.
- `LightOnly` walks the list and drives one pin `High`, the rest
  `Low`. Replace it with your own pattern (fill, blink-all, binary
  counter…) for different effects.
- Everything is plain digital `DigitalWrite`. For smooth *brightness*
  (a fade rather than on/off), you'd reach for **PWM** — `new Pwm(0, 0)`
  in the same package — but that needs a PWM-capable pin and the
  `dtoverlay=pwm` overlay.

← Back to the [examples index](README.md) ·
[Raspberry Pi overview](../README.md)
