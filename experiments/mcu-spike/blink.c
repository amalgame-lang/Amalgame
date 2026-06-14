/*
 * Phase 0 spike payload — a HAND-WRITTEN stand-in for the C that amc's
 * c_gen.am will emit under --target=cortex-m3. No amc changes yet; this
 * proves the freestanding runtime (arena + persist + semihosting) links
 * and runs on bare Cortex-M before we touch the compiler.
 *
 * Equivalent Amalgame source (the execution model of §6):
 *
 *     import Amalgame.Mcu        // (HAL is Phase 1; spike uses Console only)
 *
 *     let history = persist(new Counter())   // setup -> persistent region
 *
 *     setup {
 *       Console.WriteLine("boot: amc on bare metal")
 *     }
 *
 *     loop {                                  // arena reset at the top of each tick
 *       history.n = history.n + 1
 *       let line = "tick=" + history.n.ToString()   // arena churn, dies next tick
 *       Console.WriteLine(line)
 *     }
 *
 * The spike runs a BOUNDED loop (8 ticks) then semihosting-exits, so QEMU
 * returns. A real firmware's loop is infinite.
 */
#include "../../runtime/embedded/_runtime.h"

/* a tiny persistent object — survives every arena reset */
typedef struct { i64 n; } Counter;

/* persistent global, allocated once (modelled as a static for the spike) */
static Counter* history;

static void amc_setup(void) {
    /* history = persist(new Counter()) */
    history = persist((Counter*)code_alloc(sizeof(Counter)));
    history->n = 0;
    Console_WriteLine("boot: amc on bare metal");
}

static void amc_loop_tick(void) {
    amc_arena_reset();                       /* §6: top of tick */
    history->n = history->n + 1;             /* persistent state mutated */
    /* arena churn: concat + int->string, all freed at next reset */
    code_string line = code_string_concat("tick=", code_int_to_string(history->n));
    Console_WriteLine(line);
}

int amc_main(void) {
    amc_setup();
    for (int i = 0; i < 8; i++) amc_loop_tick();

    /* prove persistence survived 8 resets, and report arena high-water */
    Console_Write("final counter (persistent across resets) = ");
    Console_WriteLine(code_int_to_string(history->n));
    Console_Write("arena high-water bytes = ");
    Console_WriteLine(code_int_to_string((i64)amc_arena_hwm));
    Console_Write("arena top after last reset = ");
    Console_WriteLine(code_int_to_string((i64)amc_arena_top));

    amc_exit(0);
    return 0;
}
