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

**Remaining (the only blocker for a readable console):** the reused bootloader
has its console UART disabled (`CONFIG_ESP_CONSOLE_UART_NUM=-1`), so it changes
the clock but never restores the UART0 divider/framing — leaving UART0 in a state
our app can neither **read** nor **reconfigure**:

- *Can't read it:* output streams continuously (ROM banner once, no reset loop),
  but a clean continuous `'X'` (0x58) never decodes to 0x58 at **any** baud from
  9600→2,000,000 under 8N1 → non-standard baud **and/or non-8N1 frame**.
- *Can't reconfigure it:* **any** write to `UART0_CLK_CONF` (`0x60000078`) — XTAL
  (`SCLK_SEL=3`) or PLL-80M (`SCLK_SEL=1`), ±`RST_CORE` — **silences TX** (even
  raw FIFO writes). The chosen source clock isn't routed/ungated to UART0 without
  a system/PCR clock-gate we haven't wired (clock-tree task, TRM + PCR regs).

**Recommended next step — use a clean bootloader with the console enabled**
(sidesteps all the above): an ESP-IDF esp32s3 bootloader built with
`CONFIG_ESP_CONSOLE_UART_NUM=0` + `BOOTLOADER_LOG_LEVEL=INFO`; then boot is
observable and UART0 is in a known 115200 8N1 state, so the existing flash-XIP
app prints directly. (Check the other ESPHome build `vmc-controller`'s sdkconfig;
else build one with idf.py.) Then re-pin UART (= B0b-4) only if a clock change is
wanted. After clean console: fold back B0a/B0b-1/B0b-2/B0b-3 (board-agnostic) and
wire into the amc target selection.

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
