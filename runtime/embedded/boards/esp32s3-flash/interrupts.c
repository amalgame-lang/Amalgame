/*
 * interrupts.c — ESP32-S3 bare-metal interrupt bring-up (Route B, step B0b-2)
 * for the no-FreeRTOS amc runtime.
 *
 *   1. Install our own vector table (VECBASE -> _amc_vecbase, from vectors.S).
 *   2. Program the systimer's comparator 0 as a periodic 1 Hz alarm on counter
 *      unit 0 (the same XTAL-clocked counter the runtime already reads).
 *   3. Route that source through the interrupt matrix to a level-1 CPU
 *      interrupt, enable it in INTENABLE, and unmask interrupts (PS.INTLEVEL=0).
 *   4. The level-1 dispatch in vectors.S calls amc_isr_level1() here, which
 *      clears the (level-triggered) systimer source and bumps g_amc_ticks.
 *
 * All register addresses verified against the ESP-IDF esp32s3 SoC headers:
 *   systimer base 0x60023000  (systimer_reg.h)
 *   interrupt matrix base 0x600C2000, systimer_target0 map @ +0x0E4
 *                             (interrupt_core0_reg.h, reg_base.h)
 *   source ETS_SYSTIMER_TARGET0_INTR_SOURCE = 57 (interrupts.h) — informational;
 *   the matrix is programmed by the *map register*, not the source number.
 */
#include <stdint.h>

#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))

/* ── systimer ──────────────────────────────────────────────────────────── */
#define SYS_BASE            0x60023000u
#define SYS_CONF            (SYS_BASE + 0x00)   /* CLK_EN[31] U0_WORK_EN[30] TARGET0_WORK_EN[24] */
#define SYS_TARGET0_HI      (SYS_BASE + 0x1c)
#define SYS_TARGET0_LO      (SYS_BASE + 0x20)
#define SYS_TARGET0_CONF    (SYS_BASE + 0x34)   /* PERIOD[25:0] PERIOD_MODE[30] UNIT_SEL[?] */
#define SYS_COMP0_LOAD      (SYS_BASE + 0x50)   /* write 1 to latch CONF into the comparator */
#define SYS_INT_ENA         (SYS_BASE + 0x64)   /* TARGET0_INT_ENA[0] */
#define SYS_INT_CLR         (SYS_BASE + 0x6c)   /* TARGET0_INT_CLR[0] */
#define SYS_TARGET0_WORK_EN (1u << 24)
#define SYS_TARGET0_PERIOD_MODE (1u << 30)
#define SYS_COMP0_LOAD_BIT  (1u << 0)
#define SYS_TARGET0_INT_BIT (1u << 0)

/* The systimer counter is XTAL-clocked at 16 MHz (matches AMC_SYSTIMER_HZ in
 * _runtime.h; confirmed on silicon by the 1 Hz busy-wait heartbeat). */
#define SYS_HZ              16000000u

/* ── interrupt matrix ──────────────────────────────────────────────────── */
#define INTMTX_BASE         0x600C2000u
#define INTMTX_SYSTIMER_T0  (INTMTX_BASE + 0x0E4)

/* CPU interrupt we route the systimer onto: 13 is level-1, EXTERN_LEVEL on the
 * ESP32-S3 (core-isa.h: XCHAL_INT13_LEVEL=1, type EXTERN_LEVEL) — a plain
 * level-triggered peripheral interrupt, exactly what the systimer alarm is. */
#define CPU_INT_NUM         13

/* Tick counter bumped by the ISR — the heartbeat is driven off this. */
volatile uint32_t g_amc_ticks = 0;

extern uint32_t _amc_vecbase;        /* vectors.S (0x400-aligned IRAM table) */

/* ── level-1 handler table (B3: the WiFi MAC ISR rides this) ─────────────
 * The CPU has 32 interrupt lines. We dispatch every enabled+pending level-1
 * line to a registered handler. The systimer registers a built-in handler;
 * the WiFi blobs register theirs via osi `_set_isr` -> amc_set_isr(). The
 * table lives in DRAM (handlers may sit in flash); the dispatcher + the
 * systimer handler live in IRAM (reachable by the copied vector table). */
typedef void (*amc_isr_fn)(void *arg);
static amc_isr_fn g_isr_fn[32];
static void      *g_isr_arg[32];

/* Register/route a CPU interrupt for a peripheral source. osi `_set_isr` calls
 * amc_set_isr(n,fn,arg); osi `_set_intr` calls amc_route_intr(source,num) which
 * programs the matrix map register and unmasks the CPU int. */
void amc_set_isr(int n, void *fn, void *arg) {
    if ((unsigned)n < 32u) { g_isr_fn[n] = (amc_isr_fn)fn; g_isr_arg[n] = arg; }
}
void amc_route_intr(uint32_t source, uint32_t num) {
    REG32(INTMTX_BASE + source * 4u) = num;              /* source -> CPU int `num` */
    uint32_t ie; __asm__ volatile ("rsr.intenable %0" : "=r"(ie));
    ie |= (1u << num);
    __asm__ volatile ("wsr.intenable %0\n rsync\n" :: "r"(ie) : "memory");
}

static inline void amc_set_vecbase(void *base) {
    __asm__ volatile ("wsr.vecbase %0\n rsync\n" :: "r"(base) : "memory");
}
static inline void amc_set_intenable(uint32_t mask) {
    __asm__ volatile ("wsr.intenable %0\n rsync\n" :: "r"(mask) : "memory");
}
static inline void amc_enable_interrupts(void) {
    uint32_t old;
    __asm__ volatile ("rsil %0, 0\n" : "=r"(old));   /* PS.INTLEVEL = 0 */
    (void)old;
}

/* Observable panic: vectors.S _amc_panic builds a clean PS and call4's here with
 * EXCCAUSE/EPC1/EXCVADDR. In IRAM (.iram.text) so the vector's PC-relative call4
 * reaches it; prints via wlog_printf (flash, reached by inline-literal longcall). */
extern int wlog_printf(const char*, ...);
extern int uart_tx_one_char(unsigned char c);
static volatile int g_in_panic;
/* Manual (non-variadic) output — a variadic call from the panic's window context
 * drops its stack-spilled args, so print via single-arg uart_tx_one_char only. */
__attribute__((section(".iram.text")))
static void pstr(const char *s) { for (; *s; ++s) uart_tx_one_char((unsigned char)*s); }
__attribute__((section(".iram.text")))
static void phex(uint32_t v) {
    pstr("0x");
    for (int i = 28; i >= 0; i -= 4) { uint32_t n = (v >> i) & 0xF; uart_tx_one_char(n < 10 ? '0' + n : 'a' + n - 10); }
}
__attribute__((section(".iram.text")))
void amc_panic_c(uint32_t cause, uint32_t epc, uint32_t vaddr) {
    if (g_in_panic) { for (;;) {} }     /* re-entrant fault: spin, don't garble */
    g_in_panic = 1;
    pstr("\n*** PANIC c="); phex(cause);
    pstr(" epc=");          phex(epc);
    pstr(" va=");           phex(vaddr);
    pstr(" lastpc=");       phex(REG32(0x3FC88F00u));
    pstr(" ***\n");
    for (;;) {}
}

/* Built-in systimer handler: clear the level-triggered source (so the CPU line
 * deasserts) and record the tick. Runs from IRAM. No UART — the application
 * owns the heartbeat output. */
__attribute__((section(".iram.text")))
static void amc_systimer_isr(void *arg) {
    (void)arg;
    REG32(SYS_INT_CLR) = SYS_TARGET0_INT_BIT;
    g_amc_ticks++;
}

/* Level-1 dispatch, called from vectors.S. Reads the pending CPU interrupts,
 * masks to the enabled set, and calls each line's registered handler. The WiFi
 * MAC ISR (a blob, in flash) is dispatched here exactly like the systimer.
 *
 * Lives in IRAM (.iram.text): the vector table is copied to internal SRAM and
 * VECBASE points at its instruction-bus alias, so the dispatch's PC-relative
 * `call4` must reach this in the same SRAM block — not in flash (out of range).
 * Compile this TU with -mtext-section-literals so the 32-bit constants and the
 * g_isr_* base addresses are pooled inline in IRAM, reachable by l32r. The
 * handler pointers themselves are loaded from DRAM and called indirectly
 * (callx), so flash-resident handlers are reached fine. */
__attribute__((section(".iram.text")))
void amc_isr_level1(void) {
    uint32_t pend, ena;
    __asm__ volatile ("rsr.interrupt %0" : "=r"(pend));
    __asm__ volatile ("rsr.intenable %0" : "=r"(ena));
    pend &= ena;
    while (pend) {
        int n = __builtin_ctz(pend);
        pend &= ~(1u << n);
        amc_isr_fn fn = g_isr_fn[n];
        if (fn) fn(g_isr_arg[n]);
    }
}

static void amc_systimer_alarm_init(uint32_t period_ticks) {
    REG32(SYS_INT_ENA) &= ~SYS_TARGET0_INT_BIT;          /* mask while configuring */
    REG32(SYS_INT_CLR)  =  SYS_TARGET0_INT_BIT;          /* clear any stale flag    */
    /* Periodic mode: comparator reloads target = counter + period every fire.
     * PERIOD is [25:0]; 16e6 (1 s @ 16 MHz) fits (< 2^26 = 67e6). */
    REG32(SYS_TARGET0_CONF) = SYS_TARGET0_PERIOD_MODE | (period_ticks & 0x03FFFFFFu);
    REG32(SYS_COMP0_LOAD)   = SYS_COMP0_LOAD_BIT;        /* latch CONF into comparator */
    REG32(SYS_CONF)        |= SYS_TARGET0_WORK_EN;        /* enable comparator 0 */
    REG32(SYS_INT_ENA)     |= SYS_TARGET0_INT_BIT;        /* unmask systimer INT */
}

void amc_intr_init(void) {
    amc_set_vecbase((void *)&_amc_vecbase);

    amc_systimer_alarm_init(SYS_HZ);                      /* 1 Hz */

    g_isr_fn[CPU_INT_NUM]  = amc_systimer_isr;            /* register the systimer */
    g_isr_arg[CPU_INT_NUM] = 0;
    REG32(INTMTX_SYSTIMER_T0) = CPU_INT_NUM;             /* route source -> CPU int */
    amc_set_intenable(1u << CPU_INT_NUM);                /* enable just that CPU int */
    amc_enable_interrupts();                             /* PS.INTLEVEL = 0 */
}
