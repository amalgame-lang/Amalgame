# Blink an LED

The GPIO "hello world": flash an LED on and off.

## Wiring

```
GPIO17 ──▶│── 330Ω ──┐
         LED         │
                    GND
```

LED anode → **GPIO17**, cathode → **330 Ω** → **GND**.

## Code

```amalgame
namespace Demo

import Amalgame.Hardware
import Amalgame.DateTime

public class Program {
    public static void Main(string[] args) {
        let led: int = 17

        if (!Gpio.PinMode(led, PinMode.Output)) {
            Console.WriteLine("could not open GPIO17 — check permissions")
            return
        }

        var i: int = 0
        while (i < 20) {
            Gpio.Toggle(led)               // flip the output
            DateTime.SleepMillis(500)      // ...every half second
            i = i + 1
        }

        Gpio.Close()                       // release the line
    }
}
```

## Run

```sh
amc package add datetime     # once — for DateTime.SleepMillis
amc build main.am -o blink
./blink
```

## What's happening

- `Gpio.PinMode(17, PinMode.Output)` requests GPIO17 as an output. It
  returns `false` (rather than crashing) if the line is busy or you
  lack permission — always check it.
- `Gpio.Toggle(17)` flips the current level. You could be explicit
  with `Gpio.DigitalWrite(17, Level.High)` / `Level.Low` instead.
- `Gpio.Close()` releases every line the program requested. The kernel
  would reclaim them on exit anyway, but closing is tidy.
- `DateTime.SleepMillis(500)` pauses for half a second. It comes from
  the [`amalgame-datetime`](https://github.com/amalgame-lang/amalgame-datetime)
  package (`DateTime.SleepSeconds(s)` and `Duration.Sleep()` are there
  too) — pure Amalgame, no inline C in your program.

**Next:** [read a button →](02-button.md)
