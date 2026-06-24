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

## ▶ Next — esp_wifi_start hangs in MAC bring-up
After the channel set, `esp_wifi_start` busy-waits in blob/ROM code (no osi call
in between — sem/queue/delay are all traced and none fire, so it's a hardware
poll or a ROM `ets_delay_us`-style spin), before registering the MAC ISR
(`_set_isr`/`_set_intr` not yet called). Next debugging step: PC-sampling from
the systimer ISR (store the interrupted EPC1 to a global, print it) or disassemble
to find the spin, then continue: register/enable the MAC interrupt, get
`esp_wifi_start` to return, `esp_wifi_set_config` (SSID/pass), and
`esp_wifi_connect_internal` -> connected. Then B4 (lwIP NO_SYS) for DHCP/IP.

Also still needed for the WPA2 handshake at connect: the real
`g_wifi_default_wpa_crypto_funcs` (libwpa_supplicant) — currently size/version
set, fn-ptrs null (fine until connect).

## Debug aids in place
- Observable panic (vectors.S `_amc_panic` -> `amc_panic_c`): prints
  EXCCAUSE/EPC1/EXCVADDR. Diagnosed the queue crash (LoadProhibited) and the
  syscall-spill (EXCCAUSE=1) this way.
- `wlog.c` routes the blob log path to UART (the IDF `I (..) wifi:` lines).
- osi.c can be patched with `[osi] ...` traces (set_isr/set_intr/phy/sem/queue)
  for bring-up; keep them out of committed osi.c.
