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

/* Called from the level-1 dispatch in vectors.S. Keep it minimal: clear the
 * (level-triggered) systimer source so the CPU interrupt line deasserts, then
 * record the tick. No UART here — the application owns the heartbeat output. */
void amc_isr_level1(void) {
    REG32(SYS_INT_CLR) = SYS_TARGET0_INT_BIT;
    g_amc_ticks++;
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

    REG32(INTMTX_SYSTIMER_T0) = CPU_INT_NUM;             /* route source -> CPU int */
    amc_set_intenable(1u << CPU_INT_NUM);                /* enable just that CPU int */
    amc_enable_interrupts();                             /* PS.INTLEVEL = 0 */
}
