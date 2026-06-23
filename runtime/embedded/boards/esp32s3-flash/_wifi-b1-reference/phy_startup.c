#include <stdint.h>
extern uint32_t _sbss,_ebss; extern int amc_main(void);
void _start_c(void){ for(uint32_t*p=&_sbss;p<&_ebss;++p)*p=0u; (void)amc_main(); for(;;){} }
