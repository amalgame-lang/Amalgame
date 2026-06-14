# amc MCU — Phase 0 spike

Proves the runtime bet from [docs/proposals/amc-embedded.md](../../docs/proposals/amc-embedded.md)
**before** touching the compiler: that amc's emitted-style C can link and
run on bare Cortex-M with a **freestanding runtime** — no Boehm GC, no
`<stdio.h>`, no `malloc`, none of the hosted link line.

## What's here

| File | Role |
|---|---|
| [`../../runtime/embedded/_runtime.h`](../../runtime/embedded/_runtime.h) | **The durable deliverable.** Freestanding runtime profile (§4/§5): region-per-tick bump **arena**, `persist(...)` region, `code_*` string helpers, `Console_*`/`code_putc` over **ARM semihosting**. Same symbol names as the hosted runtime. |
| `startup.c` | Cortex-M3 vector table + `Reset_Handler` (`.data` copy, `.bss` zero, call `amc_main`). We own the reset path (`-nostartfiles`). |
| `lm3s6965.ld` | Linker script for QEMU `lm3s6965evb` (flash @0x0, SRAM @0x20000000). |
| `blink.c` | **Hand-written stand-in for amc's cgen output.** Carries the equivalent Amalgame source in a comment; exercises `setup`/`loop`/arena-reset/`persist`. |
| `build.sh` | Cross-compile + link + objcopy + **host-coupling audit** (`nm` must show no `GC_*`/stdio). |
| `run.sh` | Run under QEMU with semihosting. |

## Build (no special privileges)

```
./build.sh
```

Result (verified 2026-06-08, arm-none-eabi-gcc 12.2.1):

```
   text    data     bss     dec     hex filename
   1580       0   12308   13888    3640 blink.elf
   clean — no GC_*, no stdio
```

- `.text` = 1.5 KB — the whole program incl. runtime.
- `.bss` = 12 KB = the 8 KB arena + 4 KB persist region (tunable via
  `AMC_ARENA_SIZE` / `AMC_PERSIST_SIZE`).
- **No undefined symbols, no host runtime pulled in.** ← the Phase 0 gate.

## Run (needs QEMU — one-time install)

```
sudo apt-get install -y qemu-system-arm
./run.sh
```

Expected semihosting output:

```
boot: amc on bare metal
tick=1
tick=2
...
tick=8
final counter (persistent across resets) = 8
arena high-water bytes = <small, bounded — proves reset works>
arena top after last reset = ...
```

The key observation: the **arena high-water mark stays bounded to one
tick's worth** across all 8 ticks (reset works), while the **persistent
counter survives** every reset (the `persist` region is never wiped).

## What this does NOT do (deferred to Phase 1)

No amc changes: no `--target` in the driver, no `setup`/`loop`/`region`
soft keywords in the lexer, no escape check, no target descriptor, no
`Amalgame.Mcu` HAL package. The spike is the hand-written proof that the
runtime story holds; Phase 1 wires the compiler to emit this automatically.
