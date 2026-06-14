# Scan an I²C sensor bus

Almost every Pi sensor speaks **I²C**: temperature/humidity, IMUs,
small OLED displays, port expanders. This example is the Amalgame
equivalent of `i2cdetect -y 1` — it opens the bus, probes every
address, and prints what answers. Then we read one register from a
device to show the full round-trip.

## Wiring

```
sensor VCC ── 3V3        sensor SDA ── GPIO2 (SDA1, pin 3)
sensor GND ── GND        sensor SCL ── GPIO3 (SCL1, pin 5)
```

I²C is a shared two-wire bus, so several devices can hang off the same
SDA/SCL pins. Make sure I²C is enabled (see
[setup, step 3](../01-setup.md#3--enable-the-hardware-interfaces)).

## Code

```amalgame
namespace Demo

import Amalgame.Hardware

public class Program {
    // I²C addresses are read in hex; String_FromInt is decimal only, so
    // format a byte as two hex digits — pure Amalgame, no inline C.
    private static string Hex2(v: int) {
        let digits: string = "0123456789abcdef"
        let hi: int = (v / 16) % 16
        let lo: int = v % 16
        return String_CharAt1(digits, hi) + String_CharAt1(digits, lo)
    }

    public static void Main(string[] args) {
        let busNum: int = 1
        let bus: I2c = new I2c(busNum)        // /dev/i2c-1
        if (!bus.IsOpen()) {
            Console.WriteLine("could not open /dev/i2c-1 — I2C enabled? permissions?")
            return
        }

        let found: List<int> = bus.Scan()     // probe 0x03..0x77
        if (found.Count() == 0) {
            Console.WriteLine("no devices on bus " + String_FromInt(busNum))
        } else {
            Console.WriteLine(String_FromInt(found.Count()) + " device(s):")
            for addr in found {
                Console.WriteLine("  0x" + Program.Hex2(addr))
            }
        }

        bus.Close()
    }
}
```

## Run

```sh
amc build main.am -o i2c_scan
./i2c_scan
```

You'll see something like:

```
1 device(s):
  0x68
```

An MPU-6050 shows up at `0x68`, a PCF8574 LCD backpack at `0x27`, many
OLEDs at `0x3c`, and so on.

## Reading a register

Once you know an address, read or write its registers. For example, to
wake an MPU-6050 (clear its `PWR_MGMT_1` register at `0x6B`) and then
burst-read 6 bytes of accelerometer data:

```amalgame
bus.WriteReg(0x68, 0x6B, 0x00)             // wake the device
let raw: List<int> = bus.ReadBytes(0x68, 6) // ACCEL_X/Y/Z high+low bytes
```

The I²C API at a glance:

| Method | Use |
|---|---|
| `new I2c(bus)` / `.IsOpen()` / `.Close()` | open `/dev/i2c-<bus>` |
| `.Scan()` → `List<int>` | list responders, like `i2cdetect` |
| `.ReadByte(addr)` / `.WriteByte(addr, v)` | single raw byte |
| `.ReadReg(addr, reg)` / `.WriteReg(addr, reg, v)` | register access |
| `.ReadBytes(addr, n)` / `.WriteBytes(addr, list)` | multi-byte burst |

Bytes cross the boundary as `List<int>` (each `0..255`).

**Next:** [LED bar chase →](04-led-bar.md)
