#include <stdint.h>
/* phy_enter/exit_critical: mask interrupts (single-core). enter returns old PS. */
uint32_t phy_enter_critical(void){ uint32_t ps; __asm__ volatile("rsil %0, 3":"=r"(ps)); return ps; }
void     phy_exit_critical(uint32_t ps){ __asm__ volatile("wsr.ps %0\n rsync\n"::"r"(ps):"memory"); }
/* debug/coex prints: stubs (no formatting, no BT coexistence). */
int  phy_printf(const char* fmt, ...){ (void)fmt; return 0; }
void coex_pti_print(void){}
/* sprintf: stub (PHY uses it for debug strings only). */
int  sprintf(char* s, const char* fmt, ...){ (void)fmt; if(s) s[0]=0; return 0; }
