/*
 * Board startup — QEMU 'lm3s6965evb' (Stellaris LM3S6965, Cortex-M3).
 * Canonical Phase 1 board asset for amc --target=cortex-m3 (QEMU dev/CI).
 *
 * Owns the reset path end to end (linked with -nostartfiles): vector table,
 * .data copy, .bss zero, then calls amc_main() — the entry amc emits for an
 * embedded build. On return it issues a semihosting SYS_EXIT so QEMU stops.
 */
#include <stdint.h>

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;
extern int amc_main(void);

void Reset_Handler(void);
void Default_Handler(void) { for (;;) {} }

__attribute__((section(".isr_vector"), used))
uint32_t *const vector_table[] = {
    (uint32_t *)&_estack,
    (uint32_t *)Reset_Handler,
    (uint32_t *)Default_Handler,  /* NMI       */
    (uint32_t *)Default_Handler,  /* HardFault */
    (uint32_t *)Default_Handler,  /* MemManage */
    (uint32_t *)Default_Handler,  /* BusFault  */
    (uint32_t *)Default_Handler,  /* UsageFault*/
};

/* ARM semihosting SYS_EXIT so a bounded program lets QEMU return. */
static void semihost_exit(int code) {
    uint32_t block[2] = { 0x20026u /* ADP_Stopped_ApplicationExit */, (uint32_t)code };
    register int op __asm__("r0") = 0x18;            /* SYS_EXIT */
    register void* arg __asm__("r1") = block;
    __asm__ volatile ("bkpt 0xAB" : "+r"(op) : "r"(arg) : "memory");
    for (;;) {}
}

void Reset_Handler(void) {
    uint32_t *src = &_sidata, *dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;
    for (dst = &_sbss; dst < &_ebss; ) *dst++ = 0;

    int rc = amc_main();
    semihost_exit(rc);
}
