#include <stdint.h>
extern uint32_t _sbss, _ebss;
extern int amc_main(void);
extern void uart_tx_wait_idle(unsigned char);
void _start_c(void){
    for (uint32_t* p=&_sbss; p<&_ebss; ++p) *p = 0u;   /* .data is empty for this program */
    (void)amc_main();
    uart_tx_wait_idle(0);
    for(;;){}
}
