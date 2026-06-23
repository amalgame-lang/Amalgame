#include <stdint.h>
extern uint32_t _sbss, _ebss;
extern int amc_main(void);
extern void uart_tx_wait_idle(unsigned char);
extern void amc_intr_init(void);   /* interrupts.c — VECBASE + systimer alarm + matrix (B0b-2) */
void _start_c(void){
    for (uint32_t* p=&_sbss; p<&_ebss; ++p) *p = 0u;   /* .data is empty for this program */
    amc_intr_init();               /* vectors + interrupts (vector table in IROM, cached) */
    (void)amc_main();
    uart_tx_wait_idle(0);
    for(;;){}
}
