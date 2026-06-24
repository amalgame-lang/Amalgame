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

## ▶ Next
- The `wifi` task runs its own MAC/PHY HW bring-up and then stalls (no panic) —
  next: trace its osi calls (`_phy_enable`/B1 calib, `ic_set_interrupt_handler`,
  the init handshake sem/queue) to find where it spins, get init to return
  ESP_OK, then `esp_wifi_set_mode(STA)` / `set_config` / `esp_wifi_start` /
  `esp_wifi_connect_internal` → connected event. Then B4 (lwIP NO_SYS) for IP.
- Provide the real `g_wifi_default_wpa_crypto_funcs` (libwpa_supplicant) for the
  WPA2 handshake at connect.
