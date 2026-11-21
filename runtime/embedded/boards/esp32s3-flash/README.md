# esp32s3-flash — flash-XIP board for ESP32-S3 (B0c)

Runs amc programs **from flash with the cache (XIP)** — code/constants live in
flash and execute/read through the I-/D-cache, lifting the ~40 KB RAM-load size
wall ahead of the WiFi blobs + lwIP. **No 2nd-stage bootloader, no FreeRTOS, no
ESPHome**, and observable (keeps the ROM UART at 115200). Validated on an
ESP32-S3-DevKitM (`amalgame heartbeat` streams from flash after a full erase).
Step **B0c** of [esp32s3-wifi-routeB](../../../../docs/proposals/esp32s3-wifi-routeB.md).

## Two-stage image
```
0x0      stub  (RAM-loaded by the ROM)  — sets up cache/MMU, jumps to the payload
0x80000  payload (raw XIP blob)          — the amc program: .text/.literal -> IROM,
                                           .rodata -> DROM, .bss -> DRAM SRAM
```
- **`stub.c` / `stub.ld`** — first stage. The ROM loads it to IRAM/DRAM and jumps
  to `_start`. It configures the I-cache (the ROM leaves it **off** after a
  RAM-load boot — D-cache is on), writes the MMU table directly (IROM entry 0 →
  flash page 8 = `.text`; DROM entry 1 → flash page 9 = `.rodata`), unshuts the
  I-bus, enables the I-cache, enables the systimer, and `jx`-jumps to
  `0x42000000`. Uses only ROM functions (`rom_config_instruction_cache_mode`,
  `Cache_*`) + the memory-mapped MMU at `0x600C5000`.
- **`board.ld`** — payload linker: `.text`+`.literal` → IROM `0x42000000`
  (LMA flash blob offset 0), `.rodata` → DROM `0x3C010000` (LMA 0x10000),
  `.bss` → DRAM SRAM `0x3FC88000`, `_stack_top` at top of the bank.
- **`crt0.S`** — payload entry at `0x42000000`: `call4 _start_c` (SP is inherited
  from the stub — `jx` preserves it).
- **`startup.c`** — `_start_c`: zero `.bss` (amc programs have **empty `.data`**,
  so no copy), run `amc_main()`.

`_xip-bringup-reference/` keeps the on-silicon bring-up journey (the I-cache-off
discovery via a DBUS read, the IROM-only and IROM+DROM proofs, the winning
sequence).

## Build + flash
```bash
export PATH="$HOME/.platformio/packages/toolchain-xtensa-esp-elf/bin:$PATH"
examples/mcu/esp32s3-flash-xip.sh heartbeat.am /dev/ttyUSB0   # build XIP + flash
# console: 115200 8N1  ->  "[xip-stub] cache+mmu ..." then "amalgame heartbeat"
```

## Limits / TODO
- **One MMU page each:** `.text` ≤ 64 KB and `.rodata` ≤ 64 KB (IROM entry 0,
  DROM entry 1). Grow the mapped page count in `stub.c` + the regions in
  `board.ld` when the runtime (WiFi) exceeds that.
- **Not yet folded in:** B0a watchdogs + B0b-1 systimer are done in the stub;
  **B0b-2 interrupts** and **B0b-3 heap** are not wired into this board yet.
  B0b-2's vector table must live in an **IRAM section copied from flash by
  `crt0`** (interrupt vectors can't be XIP).
- **Not yet an amc target:** built via `esp32s3-flash-xip.sh` for now; a native
  `amc build --target=esp32s3-flash` (two-stage + merge) is the next step.
