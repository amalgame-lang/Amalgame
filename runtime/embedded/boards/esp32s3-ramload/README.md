# esp32s3-ramload — bundled board for `amc --target=esp32s3`

Bare-metal **ESP32-S3** (Xtensa LX7), no ESP-IDF app framework, no FreeRTOS.
The image is **RAM-loaded**: the first-stage ROM loader reads the image at
flash offset `0x0`, copies its segments into internal SRAM, and jumps to
`_start` — no flash XIP / cache setup / 2nd-stage bootloader needed for a
console-and-timer "hello". This is slice 1 + B0a/B0b-1 of the
[amc-embedded](../../../../docs/proposals/amc-embedded.md) ESP32-S3 line.

## Files
- **`board.ld`** — RAM-load linker script. SRAM regions mirror the ESP-IDF
  esp32s3 2nd-stage bootloader layout (so we land in the window the ROM keeps
  free): IRAM `.text` @ `0x403C8700` (len `0xA000`), DRAM `.rodata/.data/.bss`
  @ `0x3FCE2700` (len `0x5000`). Also provides the ROM UART symbol addresses
  (`uart_tx_one_char`, `uart_tx_wait_idle`) used by the runtime Console.
- **`startup.c`** — `_start`: disables the 3 watchdogs (TG0 MWDT, RTC RWDT in
  flash-boot mode, RTC super-WDT), enables the systimer clock, zeroes `.bss`,
  calls `amc_main()`. Register addresses are from the ESP-IDF esp32s3 SoC
  headers (`soc/{timer_group,rtc_cntl,systimer}_reg.h`).

Console + a real `Mcu_DelayMs`/`Mcu_Millis` timebase (systimer @ 16 MHz) live
in `runtime/embedded/_runtime.h` behind `#if defined(__XTENSA__)`.

## Build + flash
> ⚠️ The Xtensa toolchain truncates **non-ASCII paths** (the `é` in
> `…/Développement/…` breaks the linker). Build from an **ASCII path**: use the
> amc installed under `~/.local` (`./tools/install-local.sh`) and build in a
> plain-ASCII dir. See `examples/mcu/esp32s3-flash.sh`.

```bash
# toolchain + esptool on PATH (here: from a PlatformIO ESP-IDF install)
export PATH="$HOME/.platformio/packages/toolchain-xtensa-esp-elf/bin:$PATH"

amc build --target=esp32s3 heartbeat.am -o heartbeat   # → heartbeat.bin
esptool --chip esp32s3 -p /dev/ttyUSB0 write_flash 0x0 heartbeat.bin
# serial console: 115200 8N1
```

## Status / TODO (Route B → native WiFi, no FreeRTOS)
- [x] slice 1 — target, `esptool elf2image`, hello over ROM UART
- [x] B0a — watchdogs off, sustained loop
- [x] B0b-1 — systimer timebase (`Mcu.DelayMs`/`Millis`), 1 Hz heartbeat
- [ ] B0b-2 — interrupts: VECBASE in IRAM + interrupt matrix + systimer alarm IRQ
- [ ] B0b-3 — DRAM heap (malloc/free) ; B0b-4 — 240 MHz clocks
- [ ] B0c flash XIP/cache ; B1 PHY ; B2 osi_funcs+scheduler ; B3 esp_wifi ; B4 lwIP NO_SYS ; B5 MQTT
