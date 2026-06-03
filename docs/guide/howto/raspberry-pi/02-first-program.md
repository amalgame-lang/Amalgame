# Your first program

You've [set up your Pi](01-setup.md). Now let's create a project, pull in
the hardware package, and blink an LED.

## Wire one LED

```
GPIO17 ──▶│── 330Ω ──┐
         LED         │
                    GND
```

The LED's long leg (anode) goes to **GPIO17** (BCM numbering), the
short leg (cathode) through a **330 Ω** resistor to any **GND** pin.

> **BCM numbering.** Amalgame uses the Broadcom GPIO numbers (the same
> ones `gpiozero` and most tutorials use), *not* the physical
> board-pin positions. GPIO17 is physical pin 11.

## Create a project

```sh
mkdir blink && cd blink
amc new .              # scaffolds amalgame.toml + main.am
```

## Add the hardware package

```sh
amc package add hardware-gpio
```

This writes the dependency into `amalgame.toml`, resolves it, and
records the exact revision in `amalgame.lock`:

```toml
[dependencies.hardware-gpio]
git = "https://github.com/amalgame-lang/amalgame-hardware-gpio"
tag = "v0.6.0"
```

> Needs **amc ≥ 0.8.72**. Run `amc --version`; if it's older, re-run
> the [install one-liner](01-setup.md#2--install-amc) to update.

## Write it

Put this in `main.am`:

```amalgame
namespace Demo

import Amalgame.Hardware
import Amalgame.DateTime    // for the pause between toggles

public class Program {
    public static void Main(string[] args) {
        Console.WriteLine("libgpiod backend: " + Gpio.Backend())

        let led: int = 17
        if (!Gpio.PinMode(led, PinMode.Output)) {
            Console.WriteLine("could not open GPIO17 — check permissions")
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

> `DateTime.SleepMillis` comes from the
> [`amalgame-datetime`](https://github.com/amalgame-lang/amalgame-datetime)
> package — add it too: `amc package add datetime`. (No inline C: the
> sleep lives in the package, written in Amalgame over a single
> `nanosleep` FFI call.)

## Build and run

```sh
amc build main.am -o blink
./blink            # or: sudo ./blink  (if you're not in the 'gpio' group)
```

The LED blinks ten times over ten seconds, then the program exits and
the kernel reclaims the line.

If you see `could not open GPIO17`, it's almost always permissions —
re-read [step 5 of the setup](01-setup.md#5--permissions-the-gpioi2cspi-groups)
or just run with `sudo`.

---

That's it — you're driving real hardware from Amalgame. Now work
through the **[examples](examples/README.md)**: a debounced button,
an I²C sensor scan, and a multi-LED chase.
