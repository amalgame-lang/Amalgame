#include <stdint.h>
extern int uart_tx_one_char(unsigned char c);
uint32_t phy_enter_critical(void){ uint32_t ps; __asm__ volatile("rsil %0, 3":"=r"(ps)); return ps; }
void     phy_exit_critical(uint32_t ps){ __asm__ volatile("wsr.ps %0\n rsync\n"::"r"(ps):"memory"); }
int  phy_printf(const char* f, ...){ if(f) for(const char*p=f;*p;++p){ if(*p=='\n')uart_tx_one_char('\r'); uart_tx_one_char((unsigned char)*p);} return 0; }
void coex_pti_print(void){}
int  sprintf(char* s, const char* f, ...){ (void)f; if(s) s[0]=0; return 0; }
