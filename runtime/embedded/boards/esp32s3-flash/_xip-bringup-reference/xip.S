    .section .text, "ax"
    .global _xip
    .align 4
_xip:
    movi a2, 0x6
    slli a2, a2, 28          /* 0x60000000 = UART0 FIFO */
1:  movi a3, 0x4F            /* 'O' */
    memw
    s32i a3, a2, 0
    movi a3, 0x4B            /* 'K' */
    memw
    s32i a3, a2, 0
    movi a3, 0x0A            /* '\n' */
    memw
    s32i a3, a2, 0
    movi a4, 1
    slli a4, a4, 18
2:  addi a4, a4, -1
    bnez a4, 2b
    j 1b
