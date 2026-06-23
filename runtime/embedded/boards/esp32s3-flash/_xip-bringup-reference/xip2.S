    .section .text, "ax"
    .global _xipmain
    .align 4
_xipmain:
    movi a2, 0x6
    slli a2, a2, 28              /* a2 = 0x60000000 UART FIFO */
.Lloop:
    movi a5, 0x3C
    slli a5, a5, 8
    addi a5, a5, 1             /* a5 = 0x3C01 */
    slli a5, a5, 16            /* a5 = 0x3C010000 = msg in DROM */
.Lc:
    l8ui a3, a5, 0              /* read string byte from DROM (D-cache) */
    beqz a3, .Ldelay
    memw
    s32i a3, a2, 0             /* push to UART FIFO */
    addi a5, a5, 1
    j .Lc
.Ldelay:
    movi a4, 1
    slli a4, a4, 20
.Ld:
    addi a4, a4, -1
    bnez a4, .Ld
    j .Lloop
    .section .rodata, "a"
    .align 4
    .global msg
msg:
    .asciz "hello from flash XIP (.text=IROM, this string=DROM) ok\r\n"
