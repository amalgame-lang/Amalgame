# Read a button (edge events)

Detect button presses the *right* way: ask the kernel to watch the
pin and wake your program only when the level actually changes — no
busy-waiting in a `while` loop, and microsecond-accurate timestamps
straight from the driver.

## Wiring

```
GPIO27 ──┬── button ── GND
         │
   (internal/board pull-up holds it High; pressing pulls it Low)
```

Wire a push-button between **GPIO27** and **GND**. Add an external
pull-up resistor, or rely on the board default — `WatchEdge` itself
sets no internal bias.

## Code

```amalgame
namespace Demo

import Amalgame.Hardware

public class Program {
    public static void Main(string[] args) {
        let button: int = 27

        if (!Gpio.WatchEdge(button, Edge.Both)) {
            Console.WriteLine("could not watch GPIO27 — check permissions")
            return
        }
        Console.WriteLine("watching GPIO27 (10 edges then exit)…")

        var seen: int = 0
        while (seen < 10) {
            let ev: GpioEvent = Gpio.WaitEdge(5000)   // block up to 5s
            if (ev.IsTimeout()) {
                Console.WriteLine("(idle 5s)")
            } else {
                var what: string = "falling"
                if (ev.KindOf() == Edge.Rising) { what = "rising" }
                Console.WriteLine(what + " edge on GPIO" +
                                  String_FromInt(ev.PinOf()) + " @ " +
                                  String_FromInt(ev.TimestampNsOf()) + "ns")
                seen = seen + 1
            }
        }

        Gpio.Close()
    }
}
```

## Run

```sh
amc build main.am -o button
./button
```

Press the button a few times; each press and release prints a line.

## What's happening

- `Gpio.WatchEdge(27, Edge.Both)` arms kernel edge detection. Use
  `Edge.Rising`, `Edge.Falling`, or `Edge.Both`.
- `Gpio.WaitEdge(5000)` blocks (sleeping in the kernel, costing zero
  CPU) until the next edge, or up to 5000 ms. On timeout it returns a
  sentinel whose `IsTimeout()` is `true`.
- A delivered `GpioEvent` tells you `KindOf()` (`Rising`/`Falling`),
  `PinOf()`, and `TimestampNsOf()` (kernel monotonic nanoseconds).
- Prefer **non-blocking** draining? `Gpio.PollEdges()` returns a
  `List<GpioEvent>` of everything queued, oldest first, without
  waiting.

This is the interrupt-style pattern — far better than reading
`Gpio.DigitalRead(27)` in a tight loop, which burns a core and can
miss fast presses.

**Next:** [scan an I²C bus →](03-sensor-i2c.md)
