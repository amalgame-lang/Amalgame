# B3 — driving the WiFi blobs (in progress)

Assemble the whole Route-B stack (`_wifi-b2-reference/` osi+sched+prims+timers+
linkstubs + the B1 phy_adapter + the WiFi blobs) into one flash-XIP payload and
drive `esp_wifi_init_internal` → set mode/config STA → start → connect.

## Files
- **`build_wifi.sh`** — per-TU compile + link of everything (our runtime + the
  blobs in a `--start-group`) against the ROM lds. Produces `wifi.elf`.
- **`wifi_drive.c`** — the driver: clock→PLL80 + WiFi power/clock on, `amc_intr_init`,
  then a scheduler `init_task` that builds a `wifi_init_config_t` and calls
  `esp_wifi_init_internal`. (Plus a `mon_task` heartbeat for liveness.)
- **`wlog.c`** — minimal vprintf to UART; the blobs' log path (osi `_log_write`/
  `_log_writev`, `pp_printf`, `net80211_printf`) routes here so blob diagnostics
  are visible. Essential for bring-up.

## ✅ Done on silicon
- **Full stack LINKS** (zero undefined): ~202 KB `.text`, 31 KB `.rodata`, ~5 KB
  `.data`, ~31 KB `.bss`. board.ld now routes the blob sections: the IRAM ones
  (`.iram1*`, `.wifi*iram*`) fold into flash IROM (cache always on, no flash
  writes while WiFi runs); `.rodata_wlog_*`→DROM; `.dram1*`→`.data`.
- **`esp_wifi_init_internal` runs and validates** the config: fixed the
  re-declared `wifi_init_config_t` (the `wpa_crypto_funcs_t` is **44 bytes** =
  2×u32 + 9 fn-ptrs, not 48 → feature_caps@120, magic@144), and set
  `wpa_crypto_funcs.size=44, version=1` (the real crypto fns are only needed at
  WPA2 connect).
- **Cooperative scheduling under the blobs WORKS.** The key unlock: the ROM
  `setjmp`/`longjmp` (and blob spill paths) use the Xtensa **`syscall` = "spill
  all register windows"** mechanism, which faulted (EXCCAUSE=1) in our minimal
  vector table. Added the syscall→`SPILL_ALL_WINDOWS` handler in `vectors.S`
  (+ an observable panic that prints EXCCAUSE/EPC/EXCVADDR). Now init creates the
  `wifi` task (3584 B stack), blocks on its init sem, and the scheduler correctly
  switches to the other tasks and **runs the wifi task**.

## ✅✅ esp_wifi_init_internal -> ESP_OK, set_mode(STA) -> ESP_OK, start in progress
The fix to get init to ESP_OK was `_wifi_create_queue`: it must return a
`wifi_static_queue_t* {handle, storage}` (the blob uses `->handle`), not the raw
queue. With that, the full IDF init log prints and `esp_wifi_init_internal`
returns 0. Then `esp_wifi_set_mode(WIFI_MODE_STA=1)` returns 0, and
`esp_wifi_start()` runs: **PHY enabled + calibrated** (`phy calib done`) and the
radio sets the channel (`ht20 freq=2412, chan=1`).

## ✅✅✅ esp_wifi_start() -> ESP_OK
`esp_wifi_start` had spun in `hal_init` (0x42025302) polling MAC reg 0x60033d14 —
the WiFi clock enable value 0x78078F (enough for the PHY) was missing the WiFiMAC
clock bit, so the MAC peripheral was gated off (reads return 0). Using the full
`SYSTEM_WIFI_CLK_EN` mask **0x00FB9FCF** (in osi.c `phy_en`/`wifi_clk_en`) fixed
it. Now on silicon:
```
set_intr src=2/0 num=0 ; set_isr n=0        <- MAC ISR registered on CPU int 0
mode : sta (3c:0f:02:d2:09:54)              <- real efuse MAC
enable tsf
start r=0x00000000 (ESP_OK)
```
So init -> set_mode(STA) -> start all return ESP_OK, PHY calibrated, MAC ISR wired
on our generic level-1 dispatch, radio on channel 1.

## ▶ Next — post-start stability, then connect
After start, the system idles a few seconds then faults: `IllegalInstruction` at
`sched_yield`'s `retw` (a window underflow restoring a corrupt frame). It is
stack/timing sensitive — more task stack survives longer (1 tick @3584 -> 6 @12K),
and adding an unrelated print shifts when it hits. Our cooperative switch spills
register windows to the task stack on every setjmp (heavier than a FreeRTOS
context save), so the wifi task's 3584 B is too tight under bursty blob load;
`task_create` now enforces a 12 KB minimum (helps but the root stability of the
wifi task under beacon-reception load still needs nailing — likely a stack
high-water or a subtle spill-handler edge). Then: `esp_wifi_set_config`
(SSID/pass — **need the SSID from the user**; password is "8uckf@stVSH") and
`esp_wifi_connect_internal` -> connected, wiring the real
`g_wifi_default_wpa_crypto_funcs` (libwpa_supplicant) for the WPA2 handshake.
Then B4 (lwIP NO_SYS) for DHCP/IP.

## Debug aids in place (durable)
- Observable panic (vectors.S `_amc_panic` -> `amc_panic_c`): prints
  EXCCAUSE/EPC1/EXCVADDR + the last sampled PC, using **non-variadic** manual
  UART output (a variadic call from the panic's window context drops its
  stack-spilled args). Diagnosed every blob crash this way.
- **PC sampler**: the level-1 dispatch stores the interrupted PC to scratch word
  `0x3FC88F00`; any task (or the panic) can read it to locate a busy-wait. This
  pinpointed the `hal_init` spin.
- `wlog.c` routes the blob log path to UART (the IDF `I (..) wifi:` lines).
- osi.c can be patched with `[osi] ...` traces (set_isr/set_intr/phy/sem/queue)
  for bring-up; keep them out of committed osi.c.
