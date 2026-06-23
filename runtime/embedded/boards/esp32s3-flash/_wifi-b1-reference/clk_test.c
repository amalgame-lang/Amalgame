#include <stdint.h>
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
extern int uart_tx_one_char(unsigned char c);
extern void ets_update_cpu_frequency(uint32_t mhz);
static void p(const char*s){for(;*s;++s){if(*s=='\n')uart_tx_one_char('\r');uart_tx_one_char((unsigned char)*s);}}
static void dec(uint32_t v){char b[12];int n=0;if(!v){uart_tx_one_char('0');return;}while(v){b[n++]='0'+v%10;v/=10;}while(n--)uart_tx_one_char(b[n]);}
static uint64_t st(void){REG32(0x60023004)=(1u<<30);while(!(REG32(0x60023004)&(1u<<29))){}return ((uint64_t)REG32(0x60023040)<<32)|REG32(0x60023044);}
static void busy(uint32_t n){while(n--)__asm__ volatile("nop");}
int amc_main(void){
  p("CLK test (systimer=16MHz absolute)\n");
  uint64_t a=st(); busy(2000000); uint64_t b=st();
  p("@boot dt_ticks="); dec((uint32_t)(b-a)); p("\n");
  REG32(0x600C0010) = REG32(0x600C0010) & ~0x3u;                 /* CPUPERIOD_SEL=0 -> 80MHz */
  REG32(0x600C0060) = (REG32(0x600C0060) & ~(3u<<10)) | (1u<<10);/* SOC_CLK_SEL=1 PLL */
  ets_update_cpu_frequency(80);
  p("switched CPU->PLL 80MHz\n");
  uint64_t c=st(); busy(2000000); uint64_t d=st();
  p("@PLL  dt_ticks="); dec((uint32_t)(d-c)); p("  (should be ~half if 2x faster)\n");
  p("CLK ok, UART survived\n");
  for(;;){}
}
