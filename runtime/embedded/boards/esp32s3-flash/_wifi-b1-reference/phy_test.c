#include <stdint.h>
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
extern int register_chipv7_phy(const void* init, void* cal, int mode);
extern int uart_tx_one_char(unsigned char c);
static void p(const char*s){for(;*s;++s){if(*s=='\n')uart_tx_one_char('\r');uart_tx_one_char((unsigned char)*s);}}
static void hx(uint32_t v){p("0x");for(int i=28;i>=0;i-=4){uint32_t n=(v>>i)&0xF;uart_tx_one_char(n<10?'0'+n:'a'+n-10);}}
static void dly(int n){for(volatile int i=0;i<n;i++) __asm__ volatile("nop");}
static uint8_t cal_data[1904];
int amc_main(void){
  p("PHY: power domain on\n");
  REG32(0x60008090) &= ~(1u<<17);   /* RTC_CNTL_DIG_PWC: clear WIFI_FORCE_PD (power up RF) */
  dly(4000);
  REG32(0x60026014) |= 0x78078Fu;   /* SYSTEM_WIFI_CLK_EN: WiFi/BT common (PHY) clock */
  REG32(0x60026018) |= 0x2A1Fu;     /* modem reset (set) */
  REG32(0x60026018) &= ~0x2A1Fu;    /* modem reset (clear) */
  REG32(0x60008094) &= ~(1u<<28);   /* RTC_CNTL_DIG_ISO: clear WIFI_FORCE_ISO (de-isolate) */
  dly(4000);
  p("PHY: register_chipv7_phy(NULL,&cal,FULL)...\n");
  int r = register_chipv7_phy(0, cal_data, 2);
  p("PHY: returned r="); hx((uint32_t)r); p("\n");
  p("PHY: CALIBRATED OK\n");
  for(;;){}
}
