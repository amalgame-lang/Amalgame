    .section .text, "ax"
    .global _sched_bootstrap
    .align 4
_sched_bootstrap:                 /* a2=sp_top, a3=trampoline, a4=task */
    entry a1, 32
    mov   a1, a2                  /* switch SP to the task's stack top */
    mov   a6, a4                  /* arg0 (task) for the call4 */
    callx4 a3                     /* trampoline(task) — runs on the new stack */
1:  j 1b
