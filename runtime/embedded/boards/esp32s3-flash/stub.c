/*
 * stub.c — ESP32-S3 flash-XIP first stage (B0c), for `esp32s3-flash`.
 *
 * A tiny RAM-loaded image (the ROM loads it at flash 0x0 into IRAM/DRAM and
 * jumps to _start; the ROM UART stays at 115200, so this is observable). It sets
 * up the flash cache + MMU itself — no 2nd-stage bootloader, no FreeRTOS, no
 * ESPHome — then jumps to the XIP payload at vaddr 0x42000000 (flashed at
 * 0x80000). The payload's .text runs from flash via the I-cache and its .rodata
 * is read via the D-cache.
 *
 * Discovery that made this work: after a RAM-load boot the ROM leaves the
 * I-cache DISABLED (EXTMEM_ICACHE_CTRL @ 0x600C4060 == 0) while the D-cache is
 * on — so flash data reads work but instruction fetches from 0x42000000 fault
 * until we configure + enable the I-cache. Addresses are ROM functions
 * (esp_rom/esp32s3.rom.ld) and the memory-mapped MMU table (DR_REG_MMU_TABLE).
 *
 * Current layout maps one IROM page (.text, <=64 KB) and one DROM page
 * (.rodata, <=64 KB); grow the mapped page count here + in board.ld when the
 * payload exceeds that.
 */
#include <stdint.h>
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))

extern uint32_t _sbss, _ebss;
extern int  uart_tx_one_char(unsigned char c);
extern void Cache_Invalidate_ICache_All(void);
extern void Cache_Invalidate_DCache_All(void);
extern void Cache_Enable_ICache(uint32_t autoload);
extern void rom_config_instruction_cache_mode(uint32_t size, uint8_t ways, uint8_t line);

#define MMU_TABLE            0x600C5000u   /* entry i at +i*4; val = page | VALID(0) | FLASH(0) */
#define EXTMEM_ICACHE_CTRL1  0x600C4064u   /* bit0 = ICACHE_SHUT_CORE0_BUS (1 = shut)           */
#define SYSTIMER_CONF        0x60023000u   /* CLK_EN[31], UNIT0_WORK_EN[30]                      */
#define XIP_ENTRY            0x42000000u   /* payload vaddr (IROM)                               */
/* Multi-page (B3): IROM entries 0..7 -> flash pages 8..15 (.text, up to 512 KB);
 * DROM entries 8..15 -> flash pages 16..23 (.rodata + .data init, vaddr 0x3C080000). */
#define XIP_IROM_ENTRY0      0u            /* 0x42000000 >> 16 & ... = entry 0 */
#define XIP_DROM_ENTRY0      8u            /* 0x3C080000 -> entry 8            */
#define XIP_TEXT_PAGE0       8u            /* .text   at flash 0x80000  (page 8)  */
#define XIP_RODATA_PAGE0     16u           /* .rodata at flash 0x100000 (page 16) */
#define XIP_NPAGES           8u

static void wdt_off(void) {
    REG32(0x6001F064) = 0x50D83AA1u; REG32(0x6001F048) = 0; REG32(0x6001F064) = 0;
    REG32(0x600080B0) = 0x50D83AA1u; REG32(0x60008098) = 0; REG32(0x600080B0) = 0;
    REG32(0x600080B8) = 0x8F1D312Au; REG32(0x600080B4) = REG32(0x600080B4) | (1u << 31);
}
static void puts_(const char* s) { for (; *s; ++s) uart_tx_one_char((unsigned char)*s); }

int amc_main(void) {
    puts_("\r\n[xip-stub] cache+mmu -> flash payload\r\n");
    rom_config_instruction_cache_mode(16384u, 8, 32);     /* the ROM left the I-cache off */
    for (uint32_t i = 0; i < XIP_NPAGES; ++i) {
        REG32(MMU_TABLE + (XIP_IROM_ENTRY0 + i)*4) = XIP_TEXT_PAGE0 + i;   /* IROM .text   */
        REG32(MMU_TABLE + (XIP_DROM_ENTRY0 + i)*4) = XIP_RODATA_PAGE0 + i; /* DROM .rodata+.data */
    }
    REG32(EXTMEM_ICACHE_CTRL1) &= ~1u;                    /* unshut I-bus0 */
    Cache_Invalidate_ICache_All();
    Cache_Invalidate_DCache_All();                        /* D-cache already enabled by the ROM */
    Cache_Enable_ICache(0u);
    REG32(SYSTIMER_CONF) |= (1u << 31) | (1u << 30);      /* systimer for Mcu.DelayMs/Millis */
    __asm__ volatile ("jx %0" :: "r"(XIP_ENTRY));         /* run from flash */
    for (;;) {}
}
void _start(void) { wdt_off(); for (uint32_t* p = &_sbss; p < &_ebss; ++p) *p = 0u; (void) amc_main(); for (;;) {} }
