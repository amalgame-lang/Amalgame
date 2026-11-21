#include <stdint.h>
#include "wifi_creds.h"   /* WIFI_SSID / WIFI_PASS — local, gitignored */
#define REG32(a) (*(volatile uint32_t*)(uintptr_t)(a))
extern int uart_tx_one_char(unsigned char);
extern void amc_intr_init(void);
extern int  sched_task_create(void(*)(void*),void*,void*,uint32_t);
extern void sched_start(void); extern void sched_yield(void);
extern void ets_update_cpu_frequency(uint32_t);
extern int  esp_wifi_init_internal(const void* cfg);
extern int  esp_wifi_set_mode(int mode);
extern int  esp_wifi_start(void);
extern int  esp_wifi_set_config(int ifx, void* conf);
extern int  esp_wifi_connect_internal(void);
struct wifi_osi_funcs_t; extern struct wifi_osi_funcs_t g_wifi_osi_funcs;

typedef struct { uint32_t w[11]; } wpa_crypto_funcs_t;   /* 44 bytes (2 u32 + 9 ptrs) */
typedef struct {
  void* osi_funcs;
  wpa_crypto_funcs_t wpa_crypto_funcs;
  int static_rx_buf_num, dynamic_rx_buf_num, tx_buf_type, static_tx_buf_num, dynamic_tx_buf_num;
  int rx_mgmt_buf_type, rx_mgmt_buf_num, cache_tx_buf_num, csi_enable, ampdu_rx_enable, ampdu_tx_enable, amsdu_tx_enable;
  int nvs_enable, nano_enable, rx_ba_win, wifi_task_core_id, beacon_max_len, mgmt_sbuf_num;
  uint64_t feature_caps;
  _Bool sta_disconnected_pm;
  int espnow_max_encrypt_num, tx_hetb_queue_num;
  _Bool dump_hesigb_enable;
  int magic;
} wifi_init_config_t;

static void p(const char*s){for(;*s;++s){if(*s=='\n')uart_tx_one_char('\r');uart_tx_one_char((unsigned char)*s);} }
static void hx(uint32_t v){p("0x");for(int i=28;i>=0;i-=4){uint32_t n=(v>>i)&0xF;uart_tx_one_char(n<10?'0'+n:'a'+n-10);}}
static void init_task(void* a){ (void)a;
  p("init_task: esp_wifi_init_internal...\n");
  static wifi_init_config_t cfg;
  cfg.osi_funcs=&g_wifi_osi_funcs;
  cfg.wpa_crypto_funcs.w[0]=44; cfg.wpa_crypto_funcs.w[1]=1;   /* size, version (fns wired at connect) */
  cfg.static_rx_buf_num=10; cfg.dynamic_rx_buf_num=32; cfg.tx_buf_type=1;
  cfg.static_tx_buf_num=0; cfg.dynamic_tx_buf_num=32;
  cfg.rx_mgmt_buf_type=0; cfg.rx_mgmt_buf_num=5; cfg.cache_tx_buf_num=0;
  cfg.csi_enable=0; cfg.ampdu_rx_enable=1; cfg.ampdu_tx_enable=1; cfg.amsdu_tx_enable=0;
  cfg.nvs_enable=0; cfg.nano_enable=0; cfg.rx_ba_win=6; cfg.wifi_task_core_id=0;
  cfg.beacon_max_len=752; cfg.mgmt_sbuf_num=32; cfg.feature_caps=0;
  cfg.sta_disconnected_pm=0; cfg.espnow_max_encrypt_num=7; cfg.tx_hetb_queue_num=1;
  cfg.dump_hesigb_enable=0; cfg.magic=0x1F2F3F4F;
  int r=esp_wifi_init_internal(&cfg);
  p("init r="); hx((uint32_t)r); p(r?"  (nonzero)\n":"  (ESP_OK)\n");
  if(r==0){
    int r2=esp_wifi_set_mode(1);   /* WIFI_MODE_STA */
    p("set_mode(STA) r="); hx((uint32_t)r2); p("\n");
    int r3=esp_wifi_start();
    p("start r="); hx((uint32_t)r3); p("\n");
    if(r3==0){
      static uint8_t cfg2[256];                 /* wifi_config_t (STA), zeroed */
      const char* ss=WIFI_SSID; for(int i=0;ss[i];i++) cfg2[i]=(uint8_t)ss[i];
      const char* pw=WIFI_PASS;  for(int i=0;pw[i];i++) cfg2[32+i]=(uint8_t)pw[i];
      cfg2[96]=1;                                /* scan_method = ALL_CHANNEL (hidden AP) */
      int rc=esp_wifi_set_config(0, cfg2);       /* WIFI_IF_STA=0 */
      p("set_config r="); hx((uint32_t)rc); p("\n");
      int rk=esp_wifi_connect_internal();
      p("connect r="); hx((uint32_t)rk); p("\n");
    }
  }
  { extern volatile unsigned g_amc_ticks; unsigned last=0;
    for(;;){ if(g_amc_ticks!=last){ last=g_amc_ticks; uint32_t sp; __asm__ volatile("mov %0,a1":"=r"(sp)); p("alive t="); hx(last); p(" sp="); hx(sp); p("\n"); } sched_yield(); } }
}
static uint8_t istk[49152];
int amc_main(void){
  p("\n=== WiFi B3 drive ===\n");
  { extern int g_log_level; g_log_level=6; }   /* enable blob INFO logs */
  REG32(0x600C0010) &= ~0x3u;                                   /* CPUPERIOD_SEL=0 -> 80MHz */
  REG32(0x600C0060) = (REG32(0x600C0060)&~(3u<<10))|(1u<<10);   /* SOC_CLK_SEL = PLL */
  ets_update_cpu_frequency(80);
  /* WiFi/RF power domain + clock on, BEFORE init (the blob touches MAC regs in init) */
  REG32(0x60008090)&=~(1u<<17);                                /* RTC_CNTL_DIG_PWC: power on WiFi */
  for(volatile int i=0;i<8000;i++);
  REG32(0x60026014)|=0x00FB9FCFu;                                /* SYSTEM_WIFI_CLK_EN */
  REG32(0x60026018)|=0x2A1Fu; REG32(0x60026018)&=~0x2A1Fu;     /* SYSCON_WIFI_RST_EN pulse */
  REG32(0x60008094)&=~(1u<<28);                                /* RTC_CNTL_DIG_ISO: de-isolate */
  for(volatile int i=0;i<8000;i++);
  amc_intr_init();
  sched_task_create(init_task,0,istk,sizeof istk);
  sched_start();
  for(;;){}
}
