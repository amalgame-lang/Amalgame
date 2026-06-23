# esp32s3-flash — flash-XIP board for `amc --target=esp32s3` (B0c, WIP)

Boots from **flash with the cache (XIP)** via a reused ESP-IDF 2nd-stage
bootloader (no FreeRTOS in the bootloader — it only sets up cache/MMU and loads
the app). Unlike the `esp32s3-ramload` board (≤40 KB RAM image), code/constants
live in flash and execute/read through the cache, which is what gives room for
the WiFi blobs + lwIP later. This is step **B0c** of
[esp32s3-wifi-routeB](../../../../docs/proposals/esp32s3-wifi-routeB.md).

## Status — flash XIP boot **proven on silicon**, clean console **WIP**

Verified on an ESP32-S3-DevKitM:
- `.text`→IROM (`0x42000020`), `.rodata`→DROM (`0x3C000020`), `.data/.bss`→DRAM
  SRAM (`0x3FC88000`); image built with `esptool elf2image` (MMU-page aligned).
- Flashed `bootloader.bin@0x0` (reused from an ESPHome esp32s3 build) +
  `partition-table@0x8000` (factory app) + this app `@0x10000`.
- The app **runs from flash**: both an asm-only smoke test and the full C
  runtime (with `crt0.S` setting up the stack) produce continuous UART output,
  and the ROM banner appears exactly once (no reset loop) → IROM code execution
  + DRAM load + C stack all work.

**Remaining (the only blocker for a readable "hello"):** the reused bootloader
changes the clock (console UART is disabled in that build, `CONFIG_ESP_CONSOLE_
UART_NUM=-1`, so it doesn't restore the UART divider), leaving UART0 at a
non-115200 baud. The app must re-pin UART0 to a known clock/divider — the **same
fix as B0b-4** (force UART0 SCLK to XTAL + set CLKDIV for 115200). First attempts
silenced the UART (`SCLK_DIV_NUM`/field-semantics need confirming on silicon by
reading back the bootloader's `UART0_CLK_CONF`/`CLKDIV`). Once clean, fold back
B0a/B0b-1/B0b-2/B0b-3 (they're board-agnostic).

## Files
- **`board.ld`** — XIP layout: IROM/DROM flash regions + DRAM SRAM; `_stack_top`
  at top of the DRAM bank.
- **`crt0.S`** — entry stub: the bootloader's hand-off SP isn't reliable, so set
  SP + a windowed PS, then `call4 _start_c`.
- **`startup.c`** — `_start_c`: watchdogs off, systimer on, zero `.bss`, run
  `amc_main()`.

## Build + flash (manual, until wired into the amc driver)
```bash
RT=~/.local/share/amalgame/runtime/embedded; BD=$RT/boards/esp32s3-flash
xtensa-esp32s3-elf-gcc -mlongcalls -Os -ffreestanding -nostartfiles \
  -ffunction-sections -fdata-sections -Wl,--gc-sections \
  -I"$RT" -T "$BD/board.ld" "$BD/crt0.S" "$BD/startup.c" app.c -o app.elf
esptool --chip esp32s3 elf2image --flash-size 4MB -o app.bin app.elf
# bootloader.bin from an ESPHome esp32s3 build; partitions.bin = factory app @0x10000
esptool --chip esp32s3 -p /dev/ttyUSB0 write_flash \
  0x0 bootloader.bin  0x8000 partitions.bin  0x10000 app.bin
```
