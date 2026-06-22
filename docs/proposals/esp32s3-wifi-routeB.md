# amc on ESP32-S3 — native WiFi, bare-metal, no FreeRTOS (Route B)

**Status:** B0 substrate in progress. Slice 1 + B0a + B0b-1 **done and verified
on real silicon** (ESP32-S3-DevKitM). Next: B0b-2 (Xtensa interrupts).

Sibling of [`amc-embedded.md`](amc-embedded.md) (which brought up Cortex-M /
STM32 first). This line targets the **ESP32-S3 (Xtensa LX7)** and aims at the
end goal of running a real networked device — replacing an ESPHome firmware
(a Zehnder ComfoAir VMC controller talking to Home Assistant over WiFi) with
pure amc-compiled Amalgame.

## Why "Route B" (no FreeRTOS)

The ESP32-S3 WiFi MAC/PHY are **closed binary blobs** (`libpp`, `libnet80211`,
`libcore`, `libmesh`, `libphy`, shipped in ESP-IDF) — they cannot be rebuilt or
reverse-replaced for production. They call into an **OS adapter**
(`wifi_osi_funcs_t`, ~90 function pointers: blocking tasks, semaphores,
recursive mutexes, queues w/ timeout + from-ISR, event groups, spinlocks,
`set_intr`/`set_isr`, malloc/free, timers, `phy_enable`…) and spin up internal
real-time tasks. **The only question is what scheduler sits under them.**

- **Route A** (rejected): build as an ESP-IDF app → FreeRTOS substrate, FFI
  `esp_wifi`. Fast, but FreeRTOS-bound; against amc's freestanding model.
- **Route B** (chosen): keep the freestanding runtime, **link the blobs**, and
  implement `wifi_osi_funcs_t` ourselves over a minimal cooperative+blocking
  scheduler + **lwIP in `NO_SYS=1`**. Proven possible (the Rust `esp-wifi`
  crate does exactly this — cited only as existence proof, not used).

Reference for everything = **the C of ESP-IDF itself** (which we link):
`components/esp_wifi/.../esp_adapter.c` (osi_funcs semantics), `esp_phy/src/
phy_init.c`, the SoC headers under `components/soc/esp32s3/register/soc/*.h`,
and the Espressif TRM. **No Rust.**

## Boot model

The bundled board **`runtime/embedded/boards/esp32s3-ramload/`** is a
**RAM-loaded** image: the first-stage ROM loader reads the image at flash `0x0`,
copies its segments into internal SRAM, and jumps to `_start`. No flash XIP, no
cache setup, no 2nd-stage bootloader. SRAM regions mirror the ESP-IDF esp32s3
bootloader layout: IRAM `.text` @ `0x403C8700` (0xA000), DRAM `.rodata/.data/
.bss` @ `0x3FCE2700` (0x5000). UART0 is left configured by the ROM, so the
runtime Console uses the ROM's `uart_tx_one_char`.

> ⚠️ **Size wall (B0c):** the WiFi blobs + lwIP do **not** fit in the ~40 KB
> RAM-load image. Before real WiFi integration we must boot from **flash with
> the cache (XIP)** — decide: write a minimal 2nd-stage bootloader vs. reuse the
> ESP-IDF bootloader binary (still not FreeRTOS).

## Done (verified on silicon)

- **Slice 1** — `amc build --target=esp32s3`: Xtensa toolchain dispatch
  (`xtensa-esp32s3-elf-gcc`, `-mlongcalls`), `esptool elf2image` packaging,
  RAM-load board, Console over ROM UART. → "Hello bare metal".
- **B0a** — disable the 3 watchdogs in `startup.c` (TG0 MWDT, RTC RWDT in
  flash-boot mode — clear the **whole** `RTC_CNTL_WDTCONFIG0`, not just EN — and
  the RTC super-watchdog via AUTO_FEED). → sustained loop, no reboot.
- **B0b-1** — real timebase from the **systimer** (16 MHz, XTAL-clocked, CPU-freq
  independent) → `Mcu_DelayMs`/`Mcu_Millis` (in `_runtime.h`, `#if __XTENSA__`).
  → 1 Hz heartbeat.

ARM/hosted codegen path stays byte-identical (the embedded §0 line holds; the
pre-existing golden-c drift is unrelated, confirmed by stash/rebuild).

## Roadmap

| Step | What | Key references (on disk, ESP-IDF) |
|---|---|---|
| **B0b-2** | Interrupts: VECBASE→IRAM vector table, interrupt matrix, systimer alarm IRQ | below ↓ |
| B0b-3 | DRAM heap (malloc/free) for the blobs | — |
| B0b-4 | 240 MHz clocks + WiFi clock-gating | `esp_hw_support`/`rtc_clk` |
| B0c | Flash XIP + cache (room for blobs+lwIP) | `bootloader`, MMU/cache regs |
| B1 | PHY/RF init + calibration | `esp_phy/src/phy_init.c`, NVS |
| B2 | `wifi_osi_funcs_t` + minimal blocking scheduler | `esp_wifi/.../esp_adapter.c` |
| B3 | `esp_wifi_init/start/connect` (STA) → got-IP | `esp_wifi.h`, `esp_private/wifi.h` |
| B4 | lwIP `NO_SYS=1` on WiFi RX/TX hooks | `esp_private/wifi.h` (tx/rx) |
| B5 | MQTT → Home Assistant; reimplement the VMC controller | — |

## NEXT SESSION starts here — B0b-2 (Xtensa interrupts)

**The difficulty cliff.** Easy part (register writes — addresses already known):

- **Interrupt matrix** base `DR_REG_INTERRUPT_CORE0_BASE`: write a chosen CPU
  interrupt number into `INTERRUPT_CORE0_SYSTIMER_TARGET0_INT_MAP_REG`. Source =
  `ETS_SYSTIMER_TARGET0_INTR_SOURCE` (`soc/esp32s3/include/soc/interrupts.h`).
- **Systimer periodic alarm** (base `0x60023000`): `TARGET0_HI +0x1c` /
  `TARGET0_LO +0x20`, `TARGET0_CONF +0x34` (period), `COMP0_LOAD +0x50`,
  `INT_ENA +0x64`, `INT_CLR +0x6c`.

**Hard part (the real work):** taking `VECBASE` to our own IRAM vector table
means **the whole table must be valid** — window over/underflow + exception
vectors too, not just the timer IRQ (else the first windowed call crashes).
There is **no shortcut**: port ESP-IDF's
`components/xtensa/xtensa_vectors.S` (+ `xtensa_context.S`); core config at
`components/xtensa/esp32s3/include/xtensa/config/core-isa.h`. Budget a focused
session with several on-hardware iterations. Acceptance test: heartbeat driven
by the timer **interrupt** (not a busy-wait).

## Build + flash

ASCII-path build (the Xtensa linker truncates the accented repo path):
```bash
./tools/install-local.sh                       # amc → ~/.local (ASCII runtime)
export PATH="$HOME/.platformio/packages/toolchain-xtensa-esp-elf/bin:$PATH"
examples/mcu/esp32s3-flash.sh heartbeat.am /dev/ttyUSB0   # build (ASCII cache) + flash @0x0
# serial console: 115200 8N1
```
See `runtime/embedded/boards/esp32s3-ramload/README.md`.
