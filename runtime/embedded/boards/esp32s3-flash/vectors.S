/*
 * vectors.S — minimal Xtensa LX7 (ESP32-S3) exception/interrupt vector table
 * for the bare-metal, no-FreeRTOS amc runtime (Route B, step B0b-2).
 *
 * Why this exists: to take a timer interrupt we must point VECBASE at our own
 * table in IRAM. But VECBASE relocation moves the WHOLE table at once — the
 * CPU-mandated window over/underflow handlers included — so the table must be
 * fully valid or the first windowed call after the switch faults. This file
 * provides that complete table; interrupts.c installs it (wsr VECBASE).
 *
 * Layout = the fixed Xtensa vector offsets from VECBASE (from the esp32s3
 * core-isa.h: window vectors at 0x0, INTLEVELn at 0x180..0x280, NMI 0x2C0,
 * Kernel 0x300, User 0x340, DoubleExc 0x3C0). One section, .org-placed, so the
 * table is self-contained and VECBASE = &_amc_vecbase.
 *
 *   Window over/underflow (0x000..0x17F): copied verbatim from ESP-IDF
 *     components/xtensa/xtensa_vectors.S (these are pure CPU mechanics,
 *     identical across every Xtensa config — no RTOS dependency).
 *   INTLEVEL2..6 / NMI / Kernel / Double (0x180..0x3FF): panic stubs. We never
 *     enable medium/high-priority interrupts, and any exception here is a bug,
 *     so they spin (a stuck heartbeat is the visible symptom).
 *   User exception / level-1 interrupt (0x340): the dispatch path. In step
 *     B0b-2 (Slice B) it routes EXCCAUSE==0 (level-1 interrupt) to the C ISR;
 *     real exceptions go to panic. For Slice A it simply panics (no interrupt
 *     is enabled yet — this vector is only reached on a fault).
 */

/* EXCCAUSE values (Xtensa ISA, from xtensa/corebits.h). */
#define EXCCAUSE_SYSCALL    1       /* SYSCALL instruction = "spill all windows" */
#define EXCCAUSE_LEVEL1INT  4       /* a level-1 interrupt, not a real exception */
#define EXCCAUSE_ALLOCA     5       /* movsp (alloca) — needs the window fixup   */

/* Interrupt-frame layout used by the level-1 dispatch (below). 96 bytes,
 * 16-aligned, carved from the interruptee's own stack. */
#define IFR_SZ      96
#define IFR_A0      0
#define IFR_A2      8
#define IFR_A3      12
#define IFR_A4      16
#define IFR_A5      20
#define IFR_A6      24
#define IFR_A7      28
#define IFR_A8      32
#define IFR_A9      36
#define IFR_A10     40
#define IFR_A11     44
#define IFR_A12     48
#define IFR_A13     52
#define IFR_A14     56
#define IFR_A15     60
#define IFR_PS      64
#define IFR_PC      68
#define IFR_SAR     72
#define IFR_LBEG    76
#define IFR_LEND    80
#define IFR_LCOUNT  84
/* PS for the C ISR: WOE | UM | INTLEVEL=1, EXCM cleared. */
#define IFR_PS_C    0x00040021

    .section .exc_vectors.text, "ax"
    .balign  0x400
    .global  _amc_vecbase
_amc_vecbase:

/* ---- Window vectors (verbatim Xtensa standard) ---------------------------- */
    .org    0x000
    .global _amc_WindowOverflow4
_amc_WindowOverflow4:
    s32e    a0, a5, -16
    s32e    a1, a5, -12
    s32e    a2, a5,  -8
    s32e    a3, a5,  -4
    rfwo

    .org    0x040
    .global _amc_WindowUnderflow4
_amc_WindowUnderflow4:
    l32e    a0, a5, -16
    l32e    a1, a5, -12
    l32e    a2, a5,  -8
    l32e    a3, a5,  -4
    rfwu

    .org    0x080
    .global _amc_WindowOverflow8
_amc_WindowOverflow8:
    s32e    a0, a9, -16
    l32e    a0, a1, -12
    s32e    a1, a9, -12
    s32e    a2, a9,  -8
    s32e    a3, a9,  -4
    s32e    a4, a0, -32
    s32e    a5, a0, -28
    s32e    a6, a0, -24
    s32e    a7, a0, -20
    rfwo

    .org    0x0C0
    .global _amc_WindowUnderflow8
_amc_WindowUnderflow8:
    l32e    a0, a9, -16
    l32e    a1, a9, -12
    l32e    a2, a9,  -8
    l32e    a7, a1, -12
    l32e    a3, a9,  -4
    l32e    a4, a7, -32
    l32e    a5, a7, -28
    l32e    a6, a7, -24
    l32e    a7, a7, -20
    rfwu

    .org    0x100
    .global _amc_WindowOverflow12
_amc_WindowOverflow12:
    s32e    a0,  a13, -16
    l32e    a0,  a1,  -12
    s32e    a1,  a13, -12
    s32e    a2,  a13,  -8
    s32e    a3,  a13,  -4
    s32e    a4,  a0,  -48
    s32e    a5,  a0,  -44
    s32e    a6,  a0,  -40
    s32e    a7,  a0,  -36
    s32e    a8,  a0,  -32
    s32e    a9,  a0,  -28
    s32e    a10, a0,  -24
    s32e    a11, a0,  -20
    rfwo

    .org    0x140
    .global _amc_WindowUnderflow12
_amc_WindowUnderflow12:
    l32e    a0,  a13, -16
    l32e    a1,  a13, -12
    l32e    a2,  a13,  -8
    l32e    a11, a1,  -12
    l32e    a3,  a13,  -4
    l32e    a4,  a11, -48
    l32e    a5,  a11, -44
    l32e    a6,  a11, -40
    l32e    a7,  a11, -36
    l32e    a8,  a11, -32
    l32e    a9,  a11, -28
    l32e    a10, a11, -24
    l32e    a11, a11, -20
    rfwu

/* ---- High-priority interrupt + exception vectors (panic stubs) ------------ */
    .org    0x180                    /* Level 2 */
    j       _amc_panic
    .org    0x1C0                    /* Level 3 */
    j       _amc_panic
    .org    0x200                    /* Level 4 */
    j       _amc_panic
    .org    0x240                    /* Level 5 */
    j       _amc_panic
    .org    0x280                    /* Level 6 / Debug */
    j       _amc_panic
    .org    0x2C0                    /* NMI / Level 7 */
    j       _amc_panic

    .org    0x300                    /* Kernel exception */
    j       _amc_panic

    .org    0x340                    /* User exception + level-1 interrupt */
    .global _amc_UserExceptionVector
_amc_UserExceptionVector:
    wsr.excsave1 a0                  /* free a0 (holds interruptee a0)          */
    j       _amc_user_dispatch

    .org    0x3C0                    /* Double exception */
    j       _amc_panic

/* ---- Out-of-line bodies (past the 0x400 vector area) ---------------------- */
    .org    0x400

/* User-exception / level-1 dispatch. On entry: a0 saved in EXCSAVE1, a1 = sp,
 * PS.EXCM=1. We only handle level-1 interrupts (EXCCAUSE==4); everything else
 * (real fault, syscall, alloca) panics — amc-generated C does not alloca, as
 * Slice A confirmed (the heartbeat ran through VECBASE relocation untouched). */
    .global _amc_user_dispatch
    .align  4
_amc_user_dispatch:
    rsr.exccause a0
    beqi    a0, EXCCAUSE_LEVEL1INT, _amc_level1_int
    beqi    a0, EXCCAUSE_SYSCALL,   _amc_syscall_spill
    j       _amc_panic

/* Xtensa SYSCALL (EXCCAUSE=1): the windowed-ABI "spill all register windows"
 * request used by ROM setjmp/longjmp (and any blob spill). The OS must spill the
 * live windows to their stack save areas, skip the 3-byte `syscall`, and rfe.
 * a0 is already in EXCSAVE1 (saved by the user vector). We carve a tiny scratch
 * frame BELOW the ABI spill save area for PS/EPC1, run the spill with WOE set and
 * EXCM cleared (the macro relies on window-overflow exceptions firing), restore,
 * and return 0 in a2 (the spill is syscall 0). */
    .align  4
_amc_syscall_spill:
    addi    a1, a1, -32             /* scratch frame, clear of the [sp-16] save area */
    rsr.ps  a0
    s32i    a0, a1, 0               /* save PS  */
    rsr.epc1 a0
    s32i    a0, a1, 4               /* save EPC1 */
    movi    a0, 4
    slli    a0, a0, 16              /* 0x40000 (WOE) */
    addi    a0, a0, 0x23           /* | UM(0x20) | INTLEVEL=3 (EXCM cleared) -> 0x40023 */
    wsr.ps  a0
    rsync
    rsr.excsave1 a0                 /* restore REAL a0 so the spill captures it */
    addi    a1, a1, 32              /* sp = interruptee sp for correct spill targets */
    /* SPILL_ALL_WINDOWS (XCHAL_NUM_AREGS==64): noop writes + full window rotation */
    and     a12, a12, a12
    rotw    3
    and     a12, a12, a12
    rotw    3
    and     a12, a12, a12
    rotw    3
    and     a12, a12, a12
    rotw    3
    and     a12, a12, a12
    rotw    4
    addi    a1, a1, -32             /* back to the scratch frame */
    wsr.excsave1 a0                 /* stash real a0 while we reuse a0 as scratch */
    l32i    a0, a1, 0
    wsr.ps  a0                      /* restore PS (EXCM set again) */
    rsync
    l32i    a0, a1, 4
    addi    a0, a0, 3              /* skip the 3-byte syscall instruction */
    wsr.epc1 a0
    addi    a1, a1, 32             /* restore sp */
    movi    a2, 0                  /* spill is syscall 0 -> return 0 */
    rsr.excsave1 a0                /* real a0 back */
    rfe

/* Minimal level-1 interrupt handler. A call4 to the (windowed) C ISR would
 * clobber the interruptee's a4 (CALL4 writes the return address there), and the
 * C code clobbers a2-a15/SAR/loop regs, so we save the full current window
 * explicitly onto the interruptee's stack, run C with a clean PS, then restore
 * and `rfe`. INTLEVEL stays 1 across the C call → no level-1 re-entry; the C
 * ISR clears the peripheral (level-triggered) source before returning. */
    .align  4
_amc_level1_int:
    /* Reserve the interruptee's ABI spill save areas BELOW sp before carving the
     * IFR. Two regions live there and a later window under/overflow restores from
     * them, so clobbering them corrupts the resumed frame (the post-start crash):
     *   [sp-16..sp-1]   base save area — the interruptee's own a0-a3 if its window
     *                   was spilled (e.g. the ROM setjmp helper reads it back).
     *   [sp-48..sp-20]  extra save area — a4-a11 of a call8/call12 function the
     *                   interruptee had called, if that callee was spilled
     *                   (see _amc_WindowOverflow8/12). Reserve 64 (16-aligned). */
    addi    a1, a1, -64
    addi    a1, a1, -IFR_SZ
    s32i    a2,  a1, IFR_A2          /* save live window a2-a15 */
    s32i    a3,  a1, IFR_A3
    s32i    a4,  a1, IFR_A4
    s32i    a5,  a1, IFR_A5
    s32i    a6,  a1, IFR_A6
    s32i    a7,  a1, IFR_A7
    s32i    a8,  a1, IFR_A8
    s32i    a9,  a1, IFR_A9
    s32i    a10, a1, IFR_A10
    s32i    a11, a1, IFR_A11
    s32i    a12, a1, IFR_A12
    s32i    a13, a1, IFR_A13
    s32i    a14, a1, IFR_A14
    s32i    a15, a1, IFR_A15
    rsr.excsave1 a0                  /* interruptee a0 */
    s32i    a0,  a1, IFR_A0
    rsr.ps  a0
    s32i    a0,  a1, IFR_PS
    rsr.epc1 a0
    s32i    a0,  a1, IFR_PC
    /* PC sample (debug): store the interrupted PC to the fixed scratch word
     * 0x3FC88F00 (free gap in the vec_iram DRAM bank) so a task can read where a
     * busy-wait is spinning. a2/a3 already saved above -> free to clobber. Build
     * the address with in-range movis + shifts (no l32r: this .org section can't
     * hold a reachable literal pool). */
    movi    a2, 0x1FE
    slli    a2, a2, 5               /* 0x3FC0 */
    addi    a2, a2, 8               /* 0x3FC8 */
    slli    a2, a2, 16              /* 0x3FC80000 */
    movi    a3, 0x8F
    slli    a3, a3, 8               /* 0x8F00 */
    add     a2, a2, a3              /* 0x3FC88F00 */
    s32i    a0, a2, 0               /* [0x3FC88F00] = interrupted PC */
    rsr.sar a0
    s32i    a0,  a1, IFR_SAR
    rsr.lbeg a0
    s32i    a0,  a1, IFR_LBEG
    rsr.lend a0
    s32i    a0,  a1, IFR_LEND
    rsr.lcount a0
    s32i    a0,  a1, IFR_LCOUNT

    /* a0 = IFR_PS_C (0x40021 = WOE | UM | INTLEVEL=1), built without a literal
     * pool (movi's immediate can't hold 0x40021). */
    movi    a0, 1
    slli    a0, a0, 18              /* WOE (bit 18)            */
    addi    a0, a0, 0x21           /* | UM (bit 5) | INTLEVEL=1 */
    wsr.ps  a0
    rsync

    /* Direct PC-relative call4 (the ISR is a few KB away, well within range).
     * Disable longcalls locally so -mlongcalls does not turn this into an
     * l32r+callx4 needing a literal pool we have no place for in this section. */
    .begin  no-longcalls
    call4   amc_isr_level1
    .end    no-longcalls

    l32i    a0,  a1, IFR_LCOUNT     /* restore */
    wsr.lcount a0
    l32i    a0,  a1, IFR_LEND
    wsr.lend a0
    l32i    a0,  a1, IFR_LBEG
    wsr.lbeg a0
    l32i    a0,  a1, IFR_SAR
    wsr.sar a0
    l32i    a0,  a1, IFR_PC
    wsr.epc1 a0
    l32i    a0,  a1, IFR_PS
    wsr.ps  a0
    l32i    a2,  a1, IFR_A2
    l32i    a3,  a1, IFR_A3
    l32i    a4,  a1, IFR_A4
    l32i    a5,  a1, IFR_A5
    l32i    a6,  a1, IFR_A6
    l32i    a7,  a1, IFR_A7
    l32i    a8,  a1, IFR_A8
    l32i    a9,  a1, IFR_A9
    l32i    a10, a1, IFR_A10
    l32i    a11, a1, IFR_A11
    l32i    a12, a1, IFR_A12
    l32i    a13, a1, IFR_A13
    l32i    a14, a1, IFR_A14
    l32i    a15, a1, IFR_A15
    l32i    a0,  a1, IFR_A0         /* interruptee a0 last (was scratch) */
    addi    a1,  a1, IFR_SZ
    addi    a1,  a1, 64             /* undo the save-area reserve */
    rsync
    rfe

    .global _amc_panic
    .align  4
/* Observable panic: capture cause/epc/vaddr, build a clean PS (WOE|UM,
 * INTLEVEL=15, EXCM=0) so a windowed call is safe, then call4 the C printer
 * (amc_panic_c, in the same IRAM block -> call4 in range). It spins after. */
_amc_panic:
    rsr.exccause a6                 /* call4 args: a6/a7/a8 -> callee a2/a3/a4 */
    rsr.epc1     a7
    rsr.excvaddr a8
    movi    a5, 4
    slli    a5, a5, 16              /* 0x40000 (WOE) */
    addi    a5, a5, 0x2f           /* | UM(0x20) | INTLEVEL=15(0xf) -> 0x4002f */
    wsr.ps  a5
    rsync
    /* Switch to a dedicated panic stack (0x3FC88E00, free gap in the vec_iram
     * DRAM bank) so amc_panic_c's entry/spill doesn't fault on a corrupt sp —
     * built with in-range movis (a2/a3 are free; the args live in a6/a7/a8). */
    movi    a2, 0x1FE
    slli    a2, a2, 5               /* 0x3FC0 */
    addi    a2, a2, 8               /* 0x3FC8 */
    slli    a2, a2, 16              /* 0x3FC80000 */
    movi    a3, 0x8E
    slli    a3, a3, 8               /* 0x8E00 */
    add     a1, a2, a3              /* a1 = 0x3FC88E00 (panic stack top) */
    .begin  no-longcalls
    call4   amc_panic_c
    .end    no-longcalls
1:  j       1b
