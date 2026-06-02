# Hardware components

The **`Amalgame.Hardware`** family is a set of small packages for
talking to real-world electronics from Amalgame. They split cleanly
into three layers:

1. **The HAL** (`amalgame-hal`) — portable *interfaces*
   (`DigitalOut`/`DigitalIn`, `PwmOut`, `I2cBus`, `SpiBus`, `Clock`,
   `SerialPort`). A driver is written **once** against these.
2. **A board backend** — implements the HAL on a given board. Today:
   `amalgame-hardware-gpio` (Raspberry Pi / Linux SBCs). An MCU
   backend is on the roadmap.
3. **Drivers** — components (LEDs, motors, sensors, displays…) written
   against the HAL, so they run on *any* backend unchanged.

> 🧩 **Pi today, MCU tomorrow — same driver code.** Because drivers
> target the HAL and never a specific board, the exact same
> `amalgame-hardware-led` / `-motor` / `-sensor` code that runs on a
> Raspberry Pi (handed `hardware-gpio` pins) will run on a
> microcontroller once an MCU backend ships (see the
> [amc-embedded proposal](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-embedded.md)).
> Nothing here is Raspberry-Pi-only except the GPIO backend itself.

All packages install via the curated index — `amc package add <name>`
— and require **amc ≥ 0.8.72** (interface dispatch).

## Foundation

| Package | Install | What it is |
|---|---|---|
| **hal** ([repo](https://github.com/amalgame-lang/amalgame-hal)) | `amc package add hal` | Portable hardware-abstraction interfaces: `DigitalOut`/`DigitalIn`, `PwmOut`, `I2cBus`, `SpiBus`, `Clock`, `SerialPort`. The contract every driver targets. |

## Board backends

These provide the HAL on real hardware. Pick the one for your board;
the drivers below don't care which.

| Package | Install | What it is |
|---|---|---|
| **hardware-gpio** ([repo](https://github.com/amalgame-lang/amalgame-hardware-gpio)) | `amc package add hardware-gpio` | Raspberry Pi 1→5 / Linux SBC backend via **libgpiod v2**: GPIO digital I/O + edge events, I²C, SPI, hardware PWM, UART. Hands HAL pins/buses to drivers. See the [Raspberry Pi how-to](raspberry-pi/README.md). |
| *MCU backend* | — | On the roadmap ([amc-embedded](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-embedded.md)). Will expose the same HAL on bare-metal microcontrollers. |

## Output & actuation

| Package | Install | Components |
|---|---|---|
| **hardware-led** ([repo](https://github.com/amalgame-lang/amalgame-hardware-led)) | `amc package add hardware-led` | Simple on/off LED + RGB-PWM LED drivers over the HAL. |
| **hardware-motor** ([repo](https://github.com/amalgame-lang/amalgame-hardware-motor)) | `amc package add hardware-motor` | Servo / ESC, DC motor (H-bridge), 28BYJ-48 stepper, and relay drivers. |

## Input & sensing

| Package | Install | Components |
|---|---|---|
| **hardware-input** ([repo](https://github.com/amalgame-lang/amalgame-hardware-input)) | `amc package add hardware-input` | Debounced push-button, rotary encoder. |
| **hardware-sensor** ([repo](https://github.com/amalgame-lang/amalgame-hardware-sensor)) | `amc package add hardware-sensor` | HC-SR04 ultrasonic distance, MCP3008 ADC, BME280 temperature / pressure / humidity. |
| **hardware-comms** ([repo](https://github.com/amalgame-lang/amalgame-hardware-comms)) | `amc package add hardware-comms` | Serial-comms drivers — NMEA GPS over a HAL `SerialPort`. |

## Displays

| Package | Install | Components |
|---|---|---|
| **hardware-display** ([repo](https://github.com/amalgame-lang/amalgame-hardware-display)) | `amc package add hardware-display` | SSD1306 128×64 OLED (framebuffer + text rendering) over I²C. |

## Expanders & I/O

| Package | Install | Components |
|---|---|---|
| **hardware-io** ([repo](https://github.com/amalgame-lang/amalgame-hardware-io)) | `amc package add hardware-io` | PCF8574 8-bit I²C I/O expander — each expander pin is exposed as a HAL `DigitalOut` + `DigitalIn`, so existing drivers can drive expander pins transparently. |

## Control & math (pure Amalgame)

No hardware of its own — building blocks for control loops, board-agnostic.

| Package | Install | Components |
|---|---|---|
| **hardware-control** ([repo](https://github.com/amalgame-lang/amalgame-hardware-control)) | `amc package add hardware-control` | PID controller, IMU complementary filter, moving average, `map` / `clamp` helpers. |

---

## Putting it together

A typical project picks **one backend** + **the drivers it needs**.
On a Raspberry Pi, blinking an LED through the driver layer:

```sh
amc package add hardware-gpio    # the Pi backend (HAL provider)
amc package add hardware-led     # the portable LED driver
```

The driver is constructed from a HAL pin the backend hands out, so the
same program compiles unchanged on a future MCU backend.

New to all this? Start with the **[Raspberry Pi how-to](raspberry-pi/README.md)**
— it walks from a blank SD card to driving GPIO, buttons and I²C
sensors step by step.
