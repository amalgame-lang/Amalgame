/*
 * Minimal Cortex-M3 startup for the amc MCU Phase 0 spike.
 * Target: QEMU 'lm3s6965evb' (Stellaris LM3S6965, Cortex-M3, no FPU).
 *
 * Hand-written stand-in for what amc's driver will emit / link under
 * --target=cortex-m3 with --nostartfiles. No newlib crt0, no _start:
 * we own the vector table and the reset path end to end.
 */
#include <stdint.h>

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;

/* the amc program entry — defined in blink.c (stands in for amc_main()) */
extern int amc_main(void);

void Reset_Handler(void);
void Default_Handler(void) { for (;;) {} }

/* Cortex-M vector table: [0]=initial SP, [1]=reset, then faults.
 * QEMU loads this at 0x00000000; SP and PC are fetched from here. */
__attribute__((section(".isr_vector"), used))
uint32_t *const vector_table[] = {
    (uint32_t *)&_estack,         /* initial stack pointer            */
    (uint32_t *)Reset_Handler,    /* reset                            */
    (uint32_t *)Default_Handler,  /* NMI                              */
    (uint32_t *)Default_Handler,  /* HardFault                        */
    (uint32_t *)Default_Handler,  /* MemManage                        */
    (uint32_t *)Default_Handler,  /* BusFault                         */
    (uint32_t *)Default_Handler,  /* UsageFault                       */
};

void Reset_Handler(void) {
    /* copy .data (initialized globals) from flash LMA to RAM VMA */
    uint32_t *src = &_sidata, *dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;
    /* zero .bss (the arena & persist buffers live here) */
    for (dst = &_sbss; dst < &_ebss; ) *dst++ = 0;

    amc_main();

    /* amc_main returns via semihosting exit; belt-and-braces */
    for (;;) {}
}
