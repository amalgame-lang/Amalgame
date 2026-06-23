/*
 * Board startup — ESP32-S3 RAM-loaded image (bundled board for
 * `amc --target=esp32s3`). Linked with -nostartfiles, so this owns the
 * reset path.
 *
 * The ROM 1st-stage loader has already: set up clocks/UART0, copied our
 * segments into internal SRAM, established a stack, and left its windowed-
 * ABI exception vectors installed (VECBASE → ROM). It then jumps to
 * `_start` (the ELF entry, recorded in the esp_image header by
 * `esptool elf2image`). So all we must do is zero .bss (NOBITS — the ROM
 * does not load it) and enter amc_main(). .data is already at its final
 * DRAM address (the ROM loaded it there), so there is no .data copy.
 *
 * Entered exactly like the ESP-IDF 2nd-stage bootloader's call_start_cpu0:
 * a plain windowed-ABI C function running on the ROM stack with ROM window
 * handlers active — the first `entry` rotates cleanly (windowbase=0,
 * windowstart=1 at ROM hand-off).
 */
#include <stdint.h>

extern uint32_t _sbss, _ebss;
extern int  amc_main(void);
extern void uart_tx_wait_idle(unsigned char uart_no);   /* ROM, addr from board.ld */
extern void amc_intr_init(void);                        /* interrupts.c (B0b-2)   */

/* ── Watchdogs ──────────────────────────────────────────────────────────
 * The ROM leaves three watchdogs armed; with no app feeding them the chip
 * reboots after ~hundreds of ms (we saw TG0WDT_SYS_RST in slice 1). For a
 * sustained bare-metal loop we disable all three. Register addresses and
 * write-protect keys are from the ESP-IDF esp32s3 SoC headers
 * (soc/timer_group_reg.h, soc/rtc_cntl_reg.h):
 *   TIMERGROUP0 base 0x6001F000 — MWDT
 *   RTC_CNTL    base 0x60008000 — RWDT + super-watchdog (SWD)
 */
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define TIMG0_WDTCONFIG0   0x6001F048u
#define TIMG0_WDTWPROTECT  0x6001F064u
#define RTC_WDTCONFIG0     0x60008098u
#define RTC_WDTWPROTECT    0x600080B0u
#define RTC_SWD_CONF       0x600080B4u
#define RTC_SWD_WPROTECT   0x600080B8u
#define MWDT_WKEY          0x50D83AA1u   /* TIMG + RTC RWDT unlock key */
#define SWD_WKEY           0x8F1D312Au   /* super-watchdog unlock key  */
#define WDT_EN_BIT         (1u << 31)    /* TIMG_WDT_EN / RTC_CNTL_WDT_EN */
#define SWD_AUTO_FEED_EN   (1u << 31)    /* RTC_CNTL_SWD_AUTO_FEED_EN     */

static void esp32s3_disable_watchdogs(void) {
    /* Timer Group 0 main watchdog */
    REG32(TIMG0_WDTWPROTECT) = MWDT_WKEY;
    REG32(TIMG0_WDTCONFIG0)  = 0u;                       /* EN + all stages off */
    REG32(TIMG0_WDTWPROTECT) = 0u;
    /* RTC watchdog — clear the WHOLE config: EN *and* FLASHBOOT_MOD_EN (the
     * ROM arms the RWDT in flash-boot mode, so just clearing EN isn't enough —
     * it re-bites after its long timeout, which we saw as RTCWDT_RTC_RST). */
    REG32(RTC_WDTWPROTECT) = MWDT_WKEY;
    REG32(RTC_WDTCONFIG0)  = 0u;
    REG32(RTC_WDTWPROTECT) = 0u;
    /* RTC super-watchdog: keep it permanently auto-fed so it never bites */
    REG32(RTC_SWD_WPROTECT) = SWD_WKEY;
    REG32(RTC_SWD_CONF)     = REG32(RTC_SWD_CONF) | SWD_AUTO_FEED_EN;
}

/* Systimer — a 52-bit up-counter clocked off the XTAL, independent of the
 * CPU frequency. We enable its clock so the runtime's Mcu_Millis/DelayMs have
 * a real timebase (the rate is calibrated empirically for now). */
#define SYSTIMER_CONF_REG  0x60023000u
#define SYSTIMER_CLK_EN    (1u << 31)
#define SYSTIMER_U0_WORK_EN (1u << 30)

static void esp32s3_enable_systimer(void) {
    REG32(SYSTIMER_CONF_REG) = REG32(SYSTIMER_CONF_REG) | SYSTIMER_CLK_EN | SYSTIMER_U0_WORK_EN;
}

void _start(void) {
    esp32s3_disable_watchdogs();
    esp32s3_enable_systimer();

    for (uint32_t *p = &_sbss; p < &_ebss; ++p) { *p = 0u; }

    amc_intr_init();          /* B0b-2: relocate VECBASE to our IRAM table */

    (void) amc_main();

    /* No semihosting exit on real silicon: drain UART0 and spin. */
    uart_tx_wait_idle(0);
    for (;;) { }
}
