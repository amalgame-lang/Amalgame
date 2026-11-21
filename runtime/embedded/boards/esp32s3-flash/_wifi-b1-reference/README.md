# B1 (PHY) bring-up — ✅ DONE on silicon

First real WiFi blob bring-up — **the WiFi PHY/RF is initialised and calibrated**
on bare-metal, ESPHome-free, from flash XIP. `register_chipv7_phy` returns `0x1`
(= `ESP_CAL_DATA_CHECK_FAIL`, the normal "save new calibration" result of a first
full calibration) → "PHY CALIBRATED". `clk_test.c` is the B0b-4 clock test.

## The winning sequence (`phy_test.c`)
```c
/* B0b-4: CPU -> PLL 80 MHz so APB = 80 MHz (the PHY needs the modem clock tree up;
   the missing piece — at 40 MHz XTAL the RF calibration hung forever). */
REG(0x600C0010) &= ~0x3;                       // SYSTEM_CPU_PER_CONF: CPUPERIOD_SEL=0 (80 MHz)
REG(0x600C0060) = (REG(0x600C0060)&~(3<<10))|(1<<10);  // SYSTEM_SYSCLK_CONF: SOC_CLK_SEL=PLL
ets_update_cpu_frequency(80);                  // ROM 0x40001a4c (fix ticks/us for ets_delay_us)
/* power up RF + clock + reset + de-isolate (esp_wifi_bt_power_domain_on) */
REG(0x60008090) &= ~(1<<17); delay;           // RTC_CNTL_DIG_PWC: clear WIFI_FORCE_PD
REG(0x60026014) |= 0x78078F;                   // SYSTEM_WIFI_CLK_EN: WiFi/BT common clock
REG(0x60026018) |= 0x2A1F; REG(0x60026018) &= ~0x2A1F;  // SYSCON_WIFI_RST_EN: modem reset pulse
REG(0x60008094) &= ~(1<<28); delay;           // RTC_CNTL_DIG_ISO: clear WIFI_FORCE_ISO
register_chipv7_phy(phy_init_data, cal_data /*1904B*/, PHY_RF_CAL_FULL=2);  // -> returns
```
UART stays readable across the clock switch (the ROM UART is XTAL-sourced, so the
APB 40→80 change doesn't move its baud).

## Adapter + linking (still the small surface)

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

## Real default init_data — solved (compile `phy_init_data.c` in isolation)
`esp_phy_get_init_data()` reads from a flash partition/embed; the *default* bytes
come from `esp_phy/esp32s3/phy_init_data.c`. Compile it standalone to get the
`phy_init_data` symbol (no need to hunt the `.bin`):
```bash
xtensa-esp32s3-elf-gcc -c -DCONFIG_ESP_PHY_MAX_TX_POWER=20 \
  -I<esp_phy/include> -I<esp_phy/esp32s3/include> -I<esp_common/include> \
  -I<esp_rom/include> -I<esp_hw_support/include> \
  $IDF/components/esp_phy/esp32s3/phy_init_data.c -o phy_init_data.o
```
Then link `phy_init_data.o` and pass `phy_init_data` (a `const
esp_phy_init_data_t`, 128 B) as the first arg. The 128 bytes (MAX_TX_POWER=20):
`00 00 50 50 50 4c 4c 48 4c 48 48 44 4a 46 46 42 00 00 00 ff×45 00×46 74`.

## Remaining — calibration still hangs even with real init_data + cal_data
With init_data (real) + cal_data (1904 B) + power-domain-on + clock + de-isolate,
`register_chipv7_phy` **still doesn't return**. So it's not init/cal buffers — the
RF calibration is polling on hardware that isn't ready. Most likely **the clock
tree / RF PLL**: our stub leaves the CPU at 40 MHz XTAL (APB 40 MHz); the PHY
calibration expects the modem clock tree (APB 80 MHz / the modem PLL source) up.
That's essentially **B0b-4** (240 MHz / PLL clock bring-up), now a prerequisite
for B1. Also possible: extra analog-i2c (`phy_i2c_init`) setup the IDF does, and
`get_phy_version_str()` returns empty until the PHY is registered.

**Next:** bring up the clock tree (CPU→PLL, APB 80 MHz, modem clock source) per
`esp_hw_support/.../rtc_clk.c` + `modem_clock.c`, then retry the calibration. This
is finicky RF/clock work — iterate on silicon, watching `phy_printf` (routed to
the ROM UART here).

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
