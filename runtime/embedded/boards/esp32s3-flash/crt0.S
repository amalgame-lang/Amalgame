    .section .text.start, "ax"
    .global _start
    .align 4
_start:
    .begin no-longcalls
    call4 _start_c
    .end no-longcalls
1:  j 1b
