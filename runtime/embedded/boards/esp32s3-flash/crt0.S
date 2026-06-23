/* crt0.S — flash-XIP entry stub (B0c/B3). The stub (RAM-loaded) sets up cache/MMU
 * and `jx`-jumps to a FIXED address 0x42000000, so `_start` MUST be the very first
 * byte of .text.start. We jump over an inline literal pool (so l32r back-references
 * stay valid even with -mlongcalls — no separate .literal section to get stranded),
 * set SP, copy initialised `.data` from its flash image (DROM-readable, _data_lma)
 * into SRAM (_data_vma.._data_end), then call the C startup. */
    .section .text.start, "ax"
    .align  4
    .global _start
_start:
    j       .Lreal                  /* 0x42000000: relative branch over the pool */
    .align  4
.Lstk:  .word _stack_top
.Lsrc:  .word _data_lma
.Ldst:  .word _data_vma
.Lend:  .word _data_end
.Lvsrc: .word _vec_lma          /* vector image in DROM-mapped flash      */
.Lvdst: .word _vec_dram_dst     /* DRAM-view alias of the IRAM vec region */
.Lvsz:  .word _vec_size
.Lreal:
    l32r    a1, .Lstk               /* SP = top of DRAM stack */
    movi    a0, 1
    slli    a0, a0, 18              /* WOE (bit 18) */
    addi    a0, a0, 0x20           /* | UM (bit 5): user-vector mode, so level-1 ints
                                     * + exceptions take the User vector (0x340) where
                                     * our dispatch lives — not Kernel (0x300=panic). */
    wsr.ps  a0                      /* PS = WOE|UM; INTLEVEL=0, EXCM=0 */
    rsync
    /* copy .data: a2=src (DROM image), a3=dst (DRAM), a4=end (DRAM) */
    l32r    a2, .Lsrc
    l32r    a3, .Ldst
    l32r    a4, .Lend
.Lcopy:
    bge     a3, a4, .Ldone
    l32i    a5, a2, 0
    s32i    a5, a3, 0
    addi    a2, a2, 4
    addi    a3, a3, 4
    j       .Lcopy
.Ldone:
    /* copy vectors+ISR: a2=src (DROM image), a3=dst (DRAM-view), a4=end */
    l32r    a2, .Lvsrc
    l32r    a3, .Lvdst
    l32r    a4, .Lvsz
    add     a4, a4, a3
.Lvcopy:
    bge     a3, a4, .Lvdone
    l32i    a5, a2, 0
    s32i    a5, a3, 0
    addi    a2, a2, 4
    addi    a3, a3, 4
    j       .Lvcopy
.Lvdone:
    memw                        /* ensure SRAM writes land before fetch via I-alias */
    movi    a0, 0
    .begin  no-longcalls
    call4   _start_c
    .end    no-longcalls
1:  j 1b
