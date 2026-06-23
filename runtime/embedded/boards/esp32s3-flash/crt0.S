/* crt0.S — flash-XIP entry stub (B0c). The reused ESP-IDF bootloader jumps to
 * our entry but does NOT leave a stack pointer we can rely on (unlike the ROM
 * hand-off the ramload board uses). So before any C runs we set SP to the top of
 * our DRAM stack and a clean windowed-ABI PS (WOE on), then call the C startup.
 * _stack_top comes from board.ld.
 *
 * Literals kept explicit/synthesised so -mlongcalls can't strand a literal pool
 * we have no home for (same lesson as vectors.S). */
    .section .flash.text, "ax"
    .align  4
    .literal .Lstk, _stack_top
    .global _start
_start:
    l32r    a1, .Lstk              /* SP = top of DRAM stack */
    movi    a0, 1
    slli    a0, a0, 18             /* PS = WOE (bit 18); INTLEVEL=0, EXCM=0 */
    wsr.ps  a0
    rsync
    movi    a0, 0                  /* top of call stack (no return) */
    .begin  no-longcalls
    call4   _start_c               /* enter C startup (windowed, in-range) */
    .end    no-longcalls
.Lhang:
    j       .Lhang
