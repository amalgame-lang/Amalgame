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
> target the HAL and never a specific board, the exact same driver code
> that runs on a Raspberry Pi (handed `hardware-gpio` pins) will run on
> a microcontroller once an MCU backend ships (see the
> [amc-embedded proposal](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-embedded.md)).
> Nothing here is Raspberry-Pi-only except the GPIO backend itself.

All packages install via the curated index — `amc package add <name>`
— and require **amc ≥ 0.8.72** (interface dispatch). The **Supported
components** column lists the concrete devices each package drives.

## Foundation

| Package | Install | Provides |
|---|---|---|
| **hal** ([repo](https://github.com/amalgame-lang/amalgame-hal)) | `amc package add hal` | The portable interfaces every driver targets: `DigitalOut`, `DigitalIn`, `PwmOut`, `I2cBus`, `SpiBus`, `Clock`, `SerialPort`. |

## Board backends

These provide the HAL on real hardware. Pick the one for your board;
the drivers below don't care which.

| Package | Install | Supported peripherals |
|---|---|---|
| **hardware-gpio** ([repo](https://github.com/amalgame-lang/amalgame-hardware-gpio)) | `amc package add hardware-gpio` | Raspberry Pi 1→5 / Linux SBC via **libgpiod v2**: GPIO digital I/O + edge events, I²C, SPI, hardware PWM, UART. Hands HAL pins/buses to every driver below. See the [Raspberry Pi how-to](raspberry-pi/README.md). |
| *MCU backend* | *(roadmap)* | Will expose the same HAL on bare-metal microcontrollers — [amc-embedded](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-embedded.md). |

## Output & actuation

| Package | Install | Supported components |
|---|---|---|
| **hardware-led** ([repo](https://github.com/amalgame-lang/amalgame-hardware-led)) | `amc package add hardware-led` | Plain LED (on/off), RGB LED (PWM), **APA102 / DotStar** strip, **WS2812 / NeoPixel** strip. |
| **hardware-motor** ([repo](https://github.com/amalgame-lang/amalgame-hardware-motor)) | `amc package add hardware-motor` | Servo / ESC, DC motor (H-bridge), 28BYJ-48 stepper, **A4988** stepper driver, **AccelStepper** (trapezoidal accel/decel for any step/dir driver), relay, **PCA9685** 16-channel PWM driver, piezo buzzer. |

## Input

| Package | Install | Supported components |
|---|---|---|
| **hardware-input** ([repo](https://github.com/amalgame-lang/amalgame-hardware-input)) | `amc package add hardware-input` | Debounced push-button, rotary encoder, **matrix keypad** (rows × cols). |

## Sensing

| Package | Install | Supported components |
|---|---|---|
| **hardware-sensor** ([repo](https://github.com/amalgame-lang/amalgame-hardware-sensor)) | `amc package add hardware-sensor` | **HC-SR04** distance, **MCP3008** 8-ch SPI ADC, **BME280** temp/pressure/humidity, **MPU-6050** 6-axis IMU, **BH1750** ambient light, **INA219** current/power, **ADS1115** 16-bit I²C ADC, **HX711** load-cell ADC, **MAX6675** K-type thermocouple, **SHT31** & **AHT20** temperature/humidity, **DS18B20** 1-Wire temperature (Pi/Linux). |
| **hardware-comms** ([repo](https://github.com/amalgame-lang/amalgame-hardware-comms)) | `amc package add hardware-comms` | NMEA **GPS** receiver over a HAL `SerialPort`. |

## Displays

| Package | Install | Supported components |
|---|---|---|
| **hardware-display** ([repo](https://github.com/amalgame-lang/amalgame-hardware-display)) | `amc package add hardware-display` | **SSD1306** 128×64 OLED (framebuffer + 5×7 text), **LCD1602** 16×2 character LCD, **MAX7219** 7-seg / LED-matrix driver, **TM1637** 4-digit 7-seg. |

## Expanders & I/O

| Package | Install | Supported components |
|---|---|---|
| **hardware-io** ([repo](https://github.com/amalgame-lang/amalgame-hardware-io)) | `amc package add hardware-io` | **PCF8574** 8-bit I²C expander, **SN74HC595** 8-bit shift register, **MCP23017** 16-bit I²C expander. Each exposes its pins as HAL `DigitalOut` + `DigitalIn`, so existing drivers can drive expander pins transparently. |

## Timekeeping (RTC)

| Package | Install | Supported components |
|---|---|---|
| **hardware-rtc** ([repo](https://github.com/amalgame-lang/amalgame-hardware-rtc)) | `amc package add hardware-rtc` | **DS3231** TCXO real-time clock, **DS1307** real-time clock (over I²C), with a `DateTime` read/set helper. |

## Control & math (pure Amalgame)

No hardware of its own — building blocks for control loops, board-agnostic.

| Package | Install | Provides |
|---|---|---|
| **hardware-control** ([repo](https://github.com/amalgame-lang/amalgame-hardware-control)) | `amc package add hardware-control` | PID controller, IMU complementary filter, moving average, `map` / `clamp` range helpers. |

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
