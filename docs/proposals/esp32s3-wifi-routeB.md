# amc on ESP32-S3 — native WiFi, bare-metal, no FreeRTOS (Route B)

**Status:** B0 substrate in progress. Slice 1 + B0a + B0b-1 + **B0b-2 (Xtensa
interrupts)** + **B0b-3 (DRAM heap)** done and verified on real silicon
(ESP32-S3-DevKitM). **B0c flash-XIP boot proven on silicon** (branch
`esp32s3-flash-xip`) — only a clean console (UART re-pin, = B0b-4) remains. Next:
finish the B0c console, then B1 (PHY).

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
- **B0b-2** — **Xtensa interrupts**, the difficulty cliff, descended in two
  on-silicon slices:
  - *Slice A* — own vector table in IRAM (`boards/esp32s3-ramload/vectors.S`):
    the CPU-mandated window over/underflow handlers copied verbatim from the
    Xtensa standard, exception/high-prio vectors as panic stubs, laid out by
    `.org` from a 0x400-aligned base; `interrupts.c` points `VECBASE` at it. The
    1 Hz busy-wait heartbeat survived the relocation untouched → the whole table
    is valid (every windowed C call exercises the new handlers).
  - *Slice B* — periodic **systimer alarm** (comparator 0 on counter unit 0,
    `TARGET0_CONF`/`COMP0_LOAD`/`INT_ENA`/`INT_CLR`) routed through the
    **interrupt matrix** (`SYSTIMER_TARGET0` map reg `0x600C20E4` ← CPU int 13,
    a level-1 EXTERN_LEVEL line; source `ETS_SYSTIMER_TARGET0_INTR_SOURCE = 57`)
    into a minimal **level-1 dispatch**. The dispatch saves the full interrupted
    window (a0, a2–a15) + SAR + loop regs + PS/EPC1 onto the interruptee stack,
    runs the C ISR via an in-range `call4` with a clean C-callable `PS`
    (WOE|UM|INTLEVEL=1, EXCM=0), restores and `rfe`s. The C ISR clears the
    (level-triggered) source and bumps `g_amc_ticks`. Exposed to Amalgame as
    `Mcu.WaitTick()` (sleeps the CPU in `waiti` until the next tick) +
    `Mcu.Ticks()`. **Acceptance met: `examples/mcu/heartbeat-irq.am` beats at
    1 Hz driven by the timer interrupt, not a busy-wait** — and coexists with the
    old busy-wait heartbeat (the dispatch preserves the interruptee correctly).

  Build-driver change: `LinkEmbedded` (`src/main.am`) now also compiles+links any
  `vectors.S` / `interrupts.c` sitting beside a board's `startup.c`, so
  `amc build --target=esp32s3` picks them up automatically. No Rust, no FreeRTOS;
  we hand-rolled a ~40-line level-1 handler instead of porting ESP-IDF's
  2238-line FreeRTOS-coupled `xtensa_vectors.S`.

  **Known limitations (fine for now, revisit before WiFi):** the systimer alarm
  is wired as a fixed 1 Hz system tick for every esp32s3 program; only the one
  level-1 source is handled (no general handler table yet); alloca/syscall/real
  exceptions panic-spin (`movsp`/alloca isn't emitted by amc-generated C, as
  Slice A confirmed); medium/high-priority (level ≥ 2) interrupts are panic
  stubs.

- **B0b-3** — **DRAM heap** (`boards/esp32s3-ramload/heap.c`): a compact
  first-fit free-list allocator with bidirectional coalescing over a 320 KB bank
  of free low internal SRAM (`0x3FC88000..0x3FCD8000`, carved in `board.ld` as
  `_heap_start.._heap_end`; that range is free in flash-boot — our DIRAM image
  sits at `0x3FCD8700..0x3FCE7700`, ROM stack ~`0x3FCE9700`, ROM `.bss/.data`
  above `0x3FCED710`). `amc_malloc`/`amc_free`/`amc_calloc` + `amc_heap_*`
  introspection, lazy-init, 16-byte aligned (ABI + DMA). On-silicon acceptance
  (`test_heap.c`): allocs of 16…50000 B pattern-verified, 16-aligned; after
  freeing everything the heap **fully coalesces back to one block** (free==total
  ⇒ no leak/fragmentation bug). Auto-compiled by the build driver (same
  beside-`startup.c` pickup as B0b-2). The WiFi blobs' `wifi_osi_funcs_t`
  malloc/free (B2) will bind to these.

ARM/hosted codegen path stays byte-identical (the embedded §0 line holds; the
pre-existing golden-c drift is unrelated, confirmed by stash/rebuild). Bundled
cortex-m3 (QEMU) board unaffected — the extra-source pickup is a no-op when a
board ships no `vectors.S`/`interrupts.c`.

> **Flashing/observing gotcha (cost a while to spot):** flash with esptool then
> read the console with a **single pyserial session that holds the port open
> across the auto-reset** (`mon.py`: drop DTR, pulse RTS, then read). A separate
> `esptool run` + `cat /dev/ttyUSB0` races the USB re-enumeration and the baud
> esptool leaves behind — it drops/garbles output and *looks* like the firmware
> hung (it sent us down a systimer rabbit hole that wasn't real). 115200 8N1.

## Roadmap

| Step | What | Key references (on disk, ESP-IDF) |
|---|---|---|
| ~~B0b-2~~ | ~~Interrupts: VECBASE→IRAM vector table, interrupt matrix, systimer alarm IRQ~~ ✅ done | `xtensa_vectors.S` (window vecs), `core-isa.h`, `interrupts.h`, `systimer_reg.h`, `interrupt_core0_reg.h` |
| ~~B0b-3~~ | ~~DRAM heap (malloc/free) for the blobs~~ ✅ done | `soc.h` (DRAM bounds), `bootloader.ld` (free ranges) |
| **B0b-4** | 240 MHz clocks + WiFi clock-gating | `clk_tree_ll.h`, `system_reg.h` (recon below ↓) |
| B0c | Flash XIP + cache (room for blobs+lwIP) | `bootloader`, MMU/cache regs |
| B1 | PHY/RF init + calibration | `esp_phy/src/phy_init.c`, NVS |
| B2 | `wifi_osi_funcs_t` + minimal blocking scheduler | `esp_wifi/.../esp_adapter.c` |
| B3 | `esp_wifi_init/start/connect` (STA) → got-IP | `esp_wifi.h`, `esp_private/wifi.h` |
| B4 | lwIP `NO_SYS=1` on WiFi RX/TX hooks | `esp_private/wifi.h` (tx/rx) |
| B5 | MQTT → Home Assistant; reimplement the VMC controller | — |

## NEXT SESSION starts here — B0b-4 (240 MHz clocks), then B0c

B0b-2 and B0b-3 are done (see "Done"). The substrate now has: valid IRAM vector
table + systimer alarm/matrix/level-1 dispatch + interrupt-driven tick, and a
working 320 KB DRAM heap (`amc_malloc`/`free`/`calloc`).

**B0b-4 — bump CPU to 240 MHz. Recon already done on silicon:**

- Current state the ROM leaves us: **CPU runs from XTAL at 40 MHz**
  (`SYSTEM_SYSCLK_CONF_REG` @ `0x600C0060`, `SOC_CLK_SEL[11:10] = 0`). PLL select
  is already 480 MHz (`SYSTEM_CPU_PER_CONF_REG` @ `0x600C0010`,
  `PLL_FREQ_SEL[2] = 1`, `CPUPERIOD_SEL[1:0] = 0`). Measured ~2 busy-iter/µs at
  -O0, consistent with 40 MHz (timed against the 16 MHz systimer).
- To select 240 MHz (per `clk_tree_ll.h`): set `CPUPERIOD_SEL = 2`, then
  `SOC_CLK_SEL = 1` (PLL). Verify BBPLL 480 M is actually locked first (flash
  boots at 80 MHz "clock div:1", which implies PLL up — but confirm, don't
  assume). Then update the ROM's ticks-per-µs notion if any ROM delay is used.
- ⚠️ **The catch that made me defer this to a supervised run:** on XTAL the APB
  clock is 40 MHz and the **ROM set the UART0 baud divider for APB = 40 MHz**.
  Switching CPU to PLL takes APB to 80 MHz → **the console garbles** unless we
  also re-divide UART0 (ROM `uart_div_modify(0, new_div)`) or move UART0 to the
  XTAL clock source so its baud is APB-independent. Do this in the same change,
  or the board *looks* dead. Recoverable by reflash, but not worth doing blind.

**B0c — the real gate (flash XIP + cache). Boot PROVEN on silicon; clean console
WIP.** The size wall: WiFi blobs + lwIP don't fit the ~40 KB RAM-load image. We
must boot from flash with the cache (XIP). **Decision: reuse a prebuilt ESP-IDF
2nd-stage bootloader** (no FreeRTOS in the *bootloader*; it only sets up cache +
loads the app) rather than hand-roll cache/MMU init. We already have working
esp32s3 bootloaders on this machine from the ESPHome VMC builds, e.g.
`/home/neitsab/vmc-build/.esphome/build/vmc-loopback-test/.pioenvs/vmc-loopback-test/bootloader.bin`
(esptool `image_info`: ESP32-S3, 4 MB, DIO, 80 MHz, entry `0x403c8914`).

Flash contract (from the ESPHome `flasher_args.json`, our target too):
```
0x0000  bootloader.bin            (reuse ESPHome's)
0x8000  partition-table.bin       (minimal: nvs + phy_init + factory @0x10000)
0x10000 amc app (flash-XIP image)  ← the new work
```
A minimal **factory**-app partition table (so the bootloader boots it directly,
no OTA/otadata needed) builds cleanly with the IDF tool — generated + verified:
`gen_esp32part.py` on `nvs(0x9000,24K) / phy_init(0xf000,4K) / factory(0x10000,3M)`.

**Done (branch `esp32s3-flash-xip`):** the new bundled board
`runtime/embedded/boards/esp32s3-flash/` (`board.ld` + `crt0.S` + `startup.c`).
Layout (matches ESPHome's `firmware.bin` `image_info`):
```
DROM  @0x3C000020  flash-mapped .rodata   (esptool 64 KB MMU-page aligns the file)
IROM  @0x42000020  flash-mapped .text
DRAM  @0x3FC88000  .data/.bss in SRAM (+ stack at top of the bank)
```
**Validated on an ESP32-S3-DevKitM:** flashed the reused bootloader@0x0 + a
factory partition table@0x8000 + our app@0x10000. The app **runs from flash** —
an asm-only smoke test and the full C runtime both stream continuous UART output
and the ROM banner appears exactly once (no reset loop) → IROM code executes via
cache, DROM/DRAM load, and `crt0.S` sets up the stack the bootloader does *not*
leave reliably (a plain-C entry crashes; `crt0` sets SP + windowed PS then
`call4 _start_c`). esp_image is valid (checksum + SHA256). **The size wall is
breakable.**

**The one remaining nut — clean console (same as B0b-4).** The reused bootloader
has its console UART disabled (`CONFIG_ESP_CONSOLE_UART_NUM=-1`), so it changes
the clock but does *not* restore the UART divider → UART0 ends up at a
non-115200 baud (output streams but is unreadable at standard bauds; identical
garbage at 230400/234375 ⇒ a non-standard rate). The app must re-pin UART0 to a
known clock: set `UART0_CLK_CONF` (`0x60000078`) `SCLK_SEL=3` (XTAL) + a valid
`SCLK_DIV_NUM`, and `UART0_CLKDIV` (`0x60000014`) for 115200. First attempts
**silenced** the UART — likely `SCLK_DIV_NUM=0` is invalid (field semantics: the
IDF LL writes `div-1`, so 0 ⇒ divide-by-1, but the silence suggests otherwise).
Next session: on silicon, **read back** the bootloader's `UART0_CLK_CONF` +
`CLKDIV`, compute the actual source freq, then set the divider — or force XTAL
with `SCLK_DIV_NUM=1`. This is the identical fix B0b-4 needs.

After clean console: fold back B0a/B0b-1/2/3 (watchdogs, systimer, vectors, heap
— all board-agnostic) into the flash board, wire it into the amc target
selection, and merge `esp32s3-flash-xip`. Only then does B1 (PHY) become
possible.

### B0b-2 reference notes (for when interrupts need extending)

- The level-1 dispatch handles exactly one source today. To generalize: read the
  CPU interrupt status (`rsr.interrupt` & `INTENABLE`), index a small handler
  table, and clear edge/software sources via `INTCLEAR` (level sources clear at
  the peripheral, as the systimer does). Keep INTLEVEL=1 during dispatch.
- Medium/high-priority vectors (level 2–6, NMI) are panic stubs; the WiFi MAC
  uses level-1 (`ETS_WIFI_MAC_INTR_SOURCE`), so level-1 is likely enough, but
  PHY/clock paths may want a higher level later → fill those vectors then.
- The verbatim window over/underflow handlers in `vectors.S` are config-agnostic
  Xtensa standard; don't touch them. `_xt_context_save`/`restore` from
  `xtensa_context.S` were deliberately **not** ported — the inline save in the
  dispatch is enough for a single non-nesting level-1 handler.

## Build + flash

ASCII-path build (the Xtensa linker truncates the accented repo path):
```bash
./tools/install-local.sh                       # amc → ~/.local (ASCII runtime)
export PATH="$HOME/.platformio/packages/toolchain-xtensa-esp-elf/bin:$PATH"
examples/mcu/esp32s3-flash.sh heartbeat-irq.am /dev/ttyUSB0  # IRQ-driven heartbeat
# (or heartbeat.am for the busy-wait version)
```
Observe with a port-held-open monitor, **not** `cat`/`esptool run` (see the
gotcha note above):
```bash
python - <<'PY'   # drop DTR, pulse RTS to reset, then read 115200 8N1
import time, serial
s = serial.Serial("/dev/ttyUSB0", 115200, timeout=0.1)
s.setDTR(False); s.setRTS(True); time.sleep(0.1); s.setRTS(False)
t=time.time()
while time.time()-t < 8:
    d = s.read(4096)
    if d: print(d.decode("utf-8","replace").replace("\r",""), end="", flush=True)
PY
```
See `runtime/embedded/boards/esp32s3-ramload/README.md`.
