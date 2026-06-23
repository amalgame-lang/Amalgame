/*
 * Board startup — ESP32-S3 flash-XIP image (B0c), entered by a reused ESP-IDF
 * 2nd-stage bootloader. The bootloader has set up the flash cache/MMU, mapped
 * our IROM/DROM, copied the DRAM segment into SRAM, and jumps to _start with a
 * valid stack (its own, high in DRAM). We zero .bss (NOBITS) and enter
 * amc_main(). Watchdog/systimer bring-up mirrors the ramload board.
 *
 * (First B0c slice: no interrupts — just prove code runs from flash XIP. The
 * vector table + interrupt + heap bring-up from B0b-2/B0b-3 fold back in once
 * the boot chain is validated.)
 */
#include <stdint.h>

extern uint32_t _sbss, _ebss;
extern int  amc_main(void);
extern void uart_tx_wait_idle(unsigned char uart_no);

#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define TIMG0_WDTCONFIG0   0x6001F048u
#define TIMG0_WDTWPROTECT  0x6001F064u
#define RTC_WDTCONFIG0     0x60008098u
#define RTC_WDTWPROTECT    0x600080B0u
#define RTC_SWD_CONF       0x600080B4u
#define RTC_SWD_WPROTECT   0x600080B8u
#define MWDT_WKEY          0x50D83AA1u
#define SWD_WKEY           0x8F1D312Au
#define SWD_AUTO_FEED_EN   (1u << 31)

#define SYSTIMER_CONF_REG  0x60023000u
#define SYSTIMER_CLK_EN    (1u << 31)
#define SYSTIMER_U0_WORK_EN (1u << 30)

static void esp32s3_disable_watchdogs(void) {
    REG32(TIMG0_WDTWPROTECT) = MWDT_WKEY;
    REG32(TIMG0_WDTCONFIG0)  = 0u;
    REG32(TIMG0_WDTWPROTECT) = 0u;
    REG32(RTC_WDTWPROTECT) = MWDT_WKEY;
    REG32(RTC_WDTCONFIG0)  = 0u;
    REG32(RTC_WDTWPROTECT) = 0u;
    REG32(RTC_SWD_WPROTECT) = SWD_WKEY;
    REG32(RTC_SWD_CONF)     = REG32(RTC_SWD_CONF) | SWD_AUTO_FEED_EN;
}

static void esp32s3_enable_systimer(void) {
    REG32(SYSTIMER_CONF_REG) = REG32(SYSTIMER_CONF_REG) | SYSTIMER_CLK_EN | SYSTIMER_U0_WORK_EN;
}

/* Entered from crt0.S (which has set SP + a windowed PS). */
void _start_c(void) {
    esp32s3_disable_watchdogs();
    esp32s3_enable_systimer();

    for (uint32_t *p = &_sbss; p < &_ebss; ++p) { *p = 0u; }

    (void) amc_main();

    uart_tx_wait_idle(0);
    for (;;) { }
}
