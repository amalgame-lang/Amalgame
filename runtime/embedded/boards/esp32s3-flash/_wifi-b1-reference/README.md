# B1 (PHY) bring-up — WIP reference

First real WiFi blob bring-up. **The hard part is solved: `libphy.a` links and
`register_chipv7_phy` executes on silicon.** Calibration doesn't complete yet
(needs more of the IDF pre-init sequence) — this dir captures the working pieces.

## Done
- **Linking the PHY blob is tiny.** `nm` on `esp_phy/lib/esp32s3/libphy.a`: of 136
  undefined symbols, 290 are defined inside the archive and 2367 are ROM symbols.
  Net external surface = **5 functions** (`phy_adapter.c`): `phy_enter_critical`/
  `phy_exit_critical` (interrupt mask), `phy_printf`/`coex_pti_print`/`sprintf`
  (stubs). `chip7_phy_init_ctrl` is a COMMON symbol → the linker allocates it.
  Link with the ROM scripts: `-T esp32s3.rom.ld -T …rom.libc.ld -T …rom.libgcc.ld`
  + `-lgcc`. PHY `.text` pulled in is only ~26 KB (fits one IROM page).
- **`register_chipv7_phy` runs.** Reached it on silicon; it enters the blob.
- **Pre-init sequence** (`phy_test.c`, from `esp_phy/src/phy_init.c::esp_phy_enable`
  + `esp_wifi_bt_power_domain_on`):
  ```c
  REG(0x60008090) &= ~(1<<17);   // RTC_CNTL_DIG_PWC: clear WIFI_FORCE_PD (power up RF)
  delay; REG(0x60026014) |= 0x78078F;          // SYSTEM_WIFI_CLK_EN: WiFi/BT common clock
  REG(0x60026018) |= 0x2A1F; REG(0x60026018) &= ~0x2A1F;  // SYSCON_WIFI_RST_EN: modem reset pulse
  REG(0x60008094) &= ~(1<<28);   // RTC_CNTL_DIG_ISO: clear WIFI_FORCE_ISO (de-isolate)
  register_chipv7_phy(NULL, cal_data /*1904 B*/, PHY_RF_CAL_FULL=2);
  ```
  - `NULL` init_data: tolerated (no crash). `cal_data` MUST be non-NULL (esp32s3,
    unlike the hxx port) — `esp_phy_calibration_data_t` = version[4]+mac[6]+
    opaque[1894] = **1904 bytes**; passing NULL faults StoreProhibited @ vaddr 0.

## Remaining (calibration hangs in `register_chipv7_phy`)
Even with power-domain-on + clock + de-isolate, the call doesn't return. Likely
still missing (compare `esp_phy_enable`/`esp_phy_load_cal_and_init` in the IDF):
- proper **init_data** (the default 128-byte `esp_phy_init_data_t` from
  `phy_init_data.bin`) — try a real one instead of NULL;
- `phy_module_enable()` specifics / `phy_module_has_clock_bits` requirements;
- RF regulator / BBPLL state the calibration polls on;
- possibly running PHY calibration from **IRAM** (timing) rather than flash XIP.

## Build (manual, for iteration)
```bash
IDF=~/.platformio/packages/framework-espidf; BD=<this board>
xtensa-esp32s3-elf-gcc -mlongcalls -Os -ffreestanding -nostartfiles -Wl,--gc-sections \
  -I<runtime> -T $BD/board.ld \
  -T $IDF/components/esp_rom/esp32s3/ld/esp32s3.rom.ld \
  -T $IDF/components/esp_rom/esp32s3/ld/esp32s3.rom.libc.ld \
  -T $IDF/components/esp_rom/esp32s3/ld/esp32s3.rom.libgcc.ld \
  $BD/crt0.S phy_startup.c phy_test.c phy_adapter.c \
  $IDF/components/esp_phy/lib/esp32s3/libphy.a -lgcc -o phypl.elf
xtensa-esp32s3-elf-objcopy -O binary phypl.elf phypl.bin
# flash $BD stub @0x0 + phypl.bin @0x80000
```
