/* sched_stress.c — minimal, blob-free repro of the post-start stability bug.
 *
 * Build like build_wifi.sh but with ONLY: crt0 + vectors + interrupts + startup
 * (or probe_startup) + heap + sched + sched_boot + timers + this file, linked
 * against esp32s3.rom.libc.ld (for setjmp/longjmp). Flash stub@0 + blob@0x80000.
 *
 * Two tasks tight-loop through sched_yield(), i.e. a setjmp+longjmp (cooperative
 * self/other switch) every iteration. It faults after ~18K switches with the
 * systimer IRQ on, or ~300K with probe_startup (no IRQ) — always at sched_yield's
 * retw (IllegalInstruction, i.e. resumed with a corrupt window/frame) or in a ROM
 * spill loop, `lastpc` in the ROM window-spill helper (~0x4002e2xx).
 *
 * IMPORTANT isolation results:
 *  - Pure deep recursion (rec(24) x 100k = 2.4M window over/underflow cycles, no
 *    setjmp/sched/IRQ) runs CLEAN -> the window overflow/underflow handlers in
 *    vectors.S are sound.
 *  - So the bug is in the setjmp/`syscall`-spill path (_amc_syscall_spill in
 *    vectors.S) or the ROM setjmp/longjmp interaction, NOT the window handlers.
 *
 * Next: replace the minimal in-place _amc_syscall_spill with the full IDF-style
 * handler (allocate an XT_STK frame, _xt_context_save-equivalent: save a0/a2/a3 +
 * a12/a13, PS-dance clearing EXCM/raising INTLEVEL/setting WOE, save+restore EPC1,
 * SPILL_ALL_WINDOWS at the interruptee sp, restore, skip syscall+3, a2=(a2?-1:0),
 * rfe) — the known-correct reference is components/xtensa/xtensa_vectors.S
 * (_xt_syscall_exc) + xtensa_context.S (_xt_context_save). Validate against THIS
 * repro (fast) before rebuilding the full WiFi image. */
#include <stdint.h>
extern int sched_task_create(void(*)(void*),void*,void*,uint32_t); extern void sched_start(void); extern void sched_yield(void);
extern int uart_tx_one_char(unsigned char); extern volatile uint32_t g_amc_ticks;
static void p(const char*s){for(;*s;++s){if(*s=='\n')uart_tx_one_char('\r');uart_tx_one_char((unsigned char)*s);} }
static void hx(uint32_t v){p("0x");for(int i=28;i>=0;i-=4){uint32_t n=(v>>i)&0xF;uart_tx_one_char(n<10?'0'+n:'a'+n-10);}}
volatile uint32_t g_iter=0;
static uint32_t __attribute__((noinline)) f3(uint32_t x){ sched_yield(); return x*3+1; }
static uint32_t __attribute__((noinline)) f2(uint32_t x){ return f3(x)+x; }
static uint32_t __attribute__((noinline)) f1(uint32_t x){ return f2(x)^0x5a; }
static uint8_t stk[16384], mstk[8192];
static void stress(void* a){ (void)a; volatile uint32_t acc=0; for(uint32_t i=1;;i++){ g_iter=i; acc=f1(acc+i); } }
static void mon(void* a){ (void)a; uint32_t lt=0; for(;;){ if(g_amc_ticks!=lt){ lt=g_amc_ticks; p("t="); hx(lt); p(" iter="); hx(g_iter); p("\n"); } sched_yield(); } }
int amc_main(void){ p("\nSPILL STRESS\n"); sched_task_create(stress,0,stk,sizeof stk); sched_task_create(mon,0,mstk,sizeof mstk); sched_start(); for(;;){} }
