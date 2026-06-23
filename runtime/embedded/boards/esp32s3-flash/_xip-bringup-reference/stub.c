#include <stdint.h>
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
extern uint32_t _sbss, _ebss;
extern int  uart_tx_one_char(unsigned char c);
extern void Cache_Invalidate_ICache_All(void);
extern void Cache_Enable_ICache(uint32_t autoload);
extern void rom_config_instruction_cache_mode(uint32_t size, uint8_t ways, uint8_t line);
#define MMU_TABLE   0x600C5000u
#define EXTMEM_ICACHE_CTRL1 0x600C4064u
static void wdt_off(void){
  REG32(0x6001F064)=0x50D83AA1u; REG32(0x6001F048)=0; REG32(0x6001F064)=0;
  REG32(0x600080B0)=0x50D83AA1u; REG32(0x60008098)=0; REG32(0x600080B0)=0;
  REG32(0x600080B8)=0x8F1D312Au; REG32(0x600080B4)=REG32(0x600080B4)|(1u<<31);
}
static void m(char c){ uart_tx_one_char((unsigned char)c); }
int amc_main(void){
  m('\r');m('\n');m('X');m('2');m('\r');m('\n');
  rom_config_instruction_cache_mode(16384u, 8, 32);   /* set up I-cache (mode+memory) — ROM left it off */
  REG32(MMU_TABLE+0) = 8u;                             /* 0x42000000 -> flash page 8 */
  REG32(EXTMEM_ICACHE_CTRL1) &= ~1u;                  /* I-bus0 on */
  Cache_Invalidate_ICache_All();
  Cache_Enable_ICache(0u);
  m('g');m('o');m('>');
  __asm__ volatile("jx %0" :: "r"(0x42000000u));
  for(;;){}
}
void _start(void){ wdt_off(); for(uint32_t*p=&_sbss;p<&_ebss;++p)*p=0u; (void)amc_main(); for(;;){} }
