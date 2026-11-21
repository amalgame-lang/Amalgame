#include <stdint.h>
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
extern int register_chipv7_phy(const void* init, void* cal, int mode);
extern char* get_phy_version_str(void);
extern const uint8_t phy_init_data[];
extern int uart_tx_one_char(unsigned char c);
extern void ets_update_cpu_frequency(uint32_t mhz);
static void p(const char*s){for(;*s;++s){if(*s=='\n')uart_tx_one_char('\r');uart_tx_one_char((unsigned char)*s);}}
static void hx(uint32_t v){p("0x");for(int i=28;i>=0;i-=4){uint32_t n=(v>>i)&0xF;uart_tx_one_char(n<10?'0'+n:'a'+n-10);}}
static void dly(int n){for(volatile int i=0;i<n;i++)__asm__ volatile("nop");}
static uint8_t cal_data[1904];
int amc_main(void){
  /* B0b-4: CPU -> PLL 80MHz so APB=80MHz (PHY needs the modem clock tree up) */
  REG32(0x600C0010) = REG32(0x600C0010) & ~0x3u;
  REG32(0x600C0060) = (REG32(0x600C0060) & ~(3u<<10)) | (1u<<10);
  ets_update_cpu_frequency(80);
  p("CPU->PLL80; PHY power on\n");
  REG32(0x60008090) &= ~(1u<<17); dly(8000);
  REG32(0x60026014) |= 0x78078Fu;
  REG32(0x60026018) |= 0x2A1Fu; REG32(0x60026018) &= ~0x2A1Fu;
  REG32(0x60008094) &= ~(1u<<28); dly(8000);
  p("register_chipv7_phy(init,&cal,FULL)...\n");
  int r = register_chipv7_phy(phy_init_data, cal_data, 2);
  p("PHY returned r="); hx((uint32_t)r); p("\n");
  p("*** PHY CALIBRATED ***\n");
  for(;;){}
}
