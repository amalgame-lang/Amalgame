# amc on bare metal — compiling Amalgame for MCUs

**Status:** design **frozen** (2026-05-31). No code yet. The conceptual
design — backend strategy, memory model, execution model, escape
checking, and the hardware-access (HAL) surface — is settled (decided
with Bastien over the 2026-05-31 design session). What remains is
**plumbing** (toolchain config, linker scripts, flashing) and a
**Phase 0 spike** to prove the runtime bet in QEMU. No open conceptual
decisions.

## TL;DR

amc **already transpiles to C** ([`src/generator/c_gen.am`](../../src/generator/c_gen.am),
~5400 LOC). The blocker for MCUs is not "we need a new code generator" —
it is that the emitted C is welded to a **hosted runtime**: Boehm GC,
full libc (`stdio`/`stdlib`/`math`), and a fixed link line
(`-lgc -lm -lz -ldl -lpthread`). None of that survives on bare metal.

The plan, all of it **opt-in behind `--target=<mcu>` and a no-op on the
default hosted target**:

1. A **target abstraction** in the driver + cgen (`--target=`).
2. A **freestanding runtime profile** (`runtime/embedded/`) — no Boehm
   GC, pluggable allocator.
3. A **region-per-tick memory model** that replaces tracing GC: one
   arena per execution context, reset each `loop` tick, with a
   `persist(…)` escape valve and a conservative escape check.
4. **Toolchain integration** for cross-compilation (arm-none-eabi, esp,
   avr, riscv-none-elf).
5. A **language/stdlib subset** valid on MCU, plus a portable `Mcu`
   **HAL** whose pins are typed constants supplied by per-board
   packages.

This is one new gen *flavor* (a runtime profile + driver target), not a
new gen *backend*.

---

## 0. Governing principle — never break the hosted target

> **The hosted target is frozen. Every embedded behavior is gated
> behind `--target=<mcu>` and must be a no-op on the default target.** A
> hosted build, after all this work, must produce a binary
> *byte-identical* to today.

We never *add* behavior unconditionally — we *branch* behavior behind
the target. Breakage vectors and their guards:

| Vector | Guard |
|---|---|
| New keywords `setup`/`loop`/`region` + intrinsic `persist` | **Soft / contextual** keywords: recognized only in statement position followed by `{` (and `persist` as a call intrinsic). Verified: zero use as identifiers anywhere in the repo. |
| Region-per-tick / arena | Exists **only** under an embedded target. Hosted = Boehm GC unchanged, no arena, no reset. |
| Escape check | Engaged **only** when an arena allocator is active (i.e. embedded). Hosted never sees it. |
| Type defaults (`int`→i16, …) | Per-target override. Hosted keeps `i64`/`f64`. |
| f64 warnings/errors | Only under no-FPU targets. Hosted: nothing. |
| `_runtime.h` `#if` arms | Arms **default to today's hosted behavior** when no profile macro is set. Hosted header byte-identical. |
| Link line | Swapped only under an embedded target. Default `gcc -lgc -lm -lz -ldl -lpthread` intact. |

**The one real risk** is threading a `target` parameter through
`c_gen.am` (5400 LOC) and accidentally drifting hosted output. The net:

> **Golden-C-output test.** Snapshot the generated C for the whole test
> suite, then assert it stays *byte-identical* on the hosted target
> after the refactor. Any drift fails the test.

All target gates in cgen are **additive** — an `if target.embedded {…}`
branch is never taken on hosted, so the hosted path runs exactly the
same code as before.

---

## 1. Current architecture (what couples us to a host)

```
.am ──lexer/parser/resolver/typechecker──▶ AST
    ──c_gen.am──▶ generated .c  (+ #include "runtime/*.h")
    ──gcc -lgc -lm -lz -ldl -lpthread──▶ native ELF
```

Host couplings, with line references:

| Coupling | Location | Why it breaks on MCU |
|---|---|---|
| **Boehm GC** — `#include <gc.h>`, `GC_MALLOC`, `GC_INIT()` | [`runtime/_runtime.h:18`](../../runtime/_runtime.h#L18); `GC_INIT()` at [`src/main.am:685`](../../src/main.am#L685); ~74 `GC_*` sites across `runtime/*.h` | Boehm has no practical bare-metal port. **Blocker #1.** |
| **Hosted libc** — `stdio`, `stdlib`, `math`, `stdarg` | [`runtime/_runtime.h:8-16`](../../runtime/_runtime.h#L8-L16) | `FILE*`/`printf` absent or heavy; `malloc` may not exist. |
| **Fixed link line** `-lgc -lm -lz -ldl -lpthread` | [`src/main.am:1584`](../../src/main.am#L1584) (build), [`src/main.am:1173`](../../src/main.am#L1173) (test) | None of these libs exist on the target. |
| **String = GC'd `char*`** | [`runtime/_runtime.h:31`](../../runtime/_runtime.h#L31); `code_string_format`/`_concat` allocate per op | Pervasive heap churn. |
| **Default scalars `i64`/`f64`** | [`runtime/_runtime.h:20`](../../runtime/_runtime.h#L20) | `f64` is soft-float on M0/AVR (huge); `i64` is multi-word on AVR. |
| **No target notion** in cgen | cgen emits one C flavor | Everything above is unconditional. |

The generator is *structurally fine* for embedded C — it already emits
portable C99. The runtime and driver are the problem.

---

## 2. Strategy: embedded C profile (Option A, decided)

Keep `c_gen.am` as the single source of truth. Thread a **target
profile** from the CLI down to (a) which runtime headers/macros are
emitted, (b) which toolchain is invoked, (c) which language/stdlib
features are permitted.

```
amc --target=cortex-m4 blink.am
  ▶ c_gen.am                       (unchanged in spirit; additive gates)
  ▶ #include "runtime/embedded/_runtime.h"   (no gc.h, no stdio)
  ▶ arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostartfiles …
  ▶ blink.elf  (+ .bin / .hex)
```

Rejected (for the record):

- **Option B — new LLVM IR / asm backend.** A second ~5000-LOC
  generator that duplicates cgen, *still* needs a runtime, and buys
  little until A is proven to plateau on footprint/perf. Deferred
  indefinitely; revisit only if A cannot hit a size/latency budget.
- **Option C — C backend → clang cross.** A minor variant of A. Support
  clang as *one toolchain among several*, not a separate strategy.

---

## 3. Target abstraction

### 3.1 Target descriptor

A named record the driver resolves once and passes around (final home
likely a table in `main.am` next to the stdlib/link-flag logic):

```
Target {
  name:        "cortex-m4"
  arch:        "arm"            // arm | xtensa | riscv | avr
  bits:        32              // 8 | 32
  has_fpu:     true            // gates f64/f32 policy
  libc:        "newlib-nano"   // newlib-nano | esp-libc | avr-libc | none
  cc:          "arm-none-eabi-gcc"
  cc_flags:    ["-mcpu=cortex-m4","-mthumb","-Os","-ffreestanding"]
  ld_flags:    ["-nostartfiles","-T","<linker-script>"]
  default_int: "i32"           // override the i64 default
  allocator:   "arena"         // arena | refcount | static | malloc
  features:    { net:false, threads:false, file_io:false, … }
}
```

### 3.2 Target matrix (the four in-scope classes)

| Target | Arch / bits | libc | FPU | `int` default | Allocator default | Notes |
|---|---|---|---|---|---|---|
| **Cortex-M** (STM32, RP2040, nRF) | arm / 32 | newlib-nano | M4F/M7 yes; M0/M3 no | `i32` | arena | The reference port. **STM32** is the canonical member — backend wraps STM32 HAL or libopencm3. M0/M3: `f64`→soft-float, discouraged. |
| **ESP32** (Xtensa / RISC-V) | xtensa or riscv / 32 | esp-libc (FreeRTOS) | yes | `i32` | **malloc OK** | Easiest first port — has `malloc`, threads, even WiFi (future bridge to `amalgame-net`). |
| **AVR** (Uno/Nano, atmega328p) | avr / 8 | avr-libc | no | `i16` | static pools | Brutal stress test: 2 KB RAM, 8-bit. `i64`/`f64` gated hard. |
| **RISC-V bare metal** (GD32V, ESP32-C3) | riscv / 32 | newlib-nano | optional | `i32` | arena | Reuses most of the Cortex-M path; `-march=rv32imc`. |

**Porting order:** ESP32 (malloc + threads ≈ hosted, fastest win) →
Cortex-M / **STM32** (canonical bare-metal reference) → RISC-V (shares
the Cortex-M path) → AVR (last; forces every hard decision).

---

## 4. Freestanding runtime profile

A `runtime/embedded/` set, selected as **one tree with `#if` arms**
(not N copies) that default to hosted behavior when no profile macro is
defined:

- **No `<gc.h>`.** `code_alloc` routes to the selected allocator (§5).
- **No `<stdio.h>` by default.** `Console.Write*` retargets to a
  board-provided `code_putc(char)` (UART / semihosting / RTT). Apps
  that never print pull in nothing. This makes a basic serial console
  free in Phase 1.
- **No `vsnprintf`-based `code_string_format`** unless the target libc
  provides it; otherwise a tiny integer/format shim.
- Same *type names* (`i64`, `f64`, `code_string`, `code_bool`) so cgen
  output is unchanged — but the header may `typedef` `int` narrower and
  the typechecker may forbid `f64`/`i64` on AVR.

Selection: the driver emits `#include "runtime/embedded/_runtime.h"`
plus a profile macro (`#define AMC_TARGET_AVR 1`, `AMC_ALLOC_ARENA`, …)
that the headers branch on.

---

## 5. Memory model — region-per-tick (decided)

Boehm tracing GC is off the table on bare metal. The model below
replaces it and, in doing so, dissolves three would-be problems at once
(arena reset discipline, string churn, and the need for refcounting in
Phase 1).

### 5.1 The unifying invariant

> Each independent **execution context** (the main loop, or — later —
> each task) owns exactly **one per-tick arena**, reset at *its own*
> tick boundary. **ISRs own no arena**: they are allocation-free and
> touch only persistent state. The **escape rule**: an arena value may
> not be stored anywhere that outlives the context's current tick —
> globals, another task, or ISR-shared state.

### 5.2 Three allocation origins, one rule

| Origin | What it is | Lifetime |
|---|---|---|
| **arena** (default inside `loop`/`region`) | `new Foo()`, `a + b`, a fresh call result | dies at end of tick/region |
| **persistent** | globals, anything `setup` allocates, and explicit `persist(…)` | forever |
| (Phase 2+) refcount / static pools | — | — |

**Rule: `arena ↛ persistent`.** Storing an arena value into a
persistent location is an error. The reverse (reading a global from
`loop`) is always fine.

The arena is a bump allocator; reset is just `top = 0`. Because each
arena has a **single owner**, reset needs **no atomics and no critical
section**, even when an ISR fires mid-reset (ISRs don't touch `top`).

### 5.3 `persist(…)` — the escape valve

The common pattern (accumulate across ticks) needs a marked gate:

```amalgame
let history = new List()        // setup → persistent

loop {
  let s = readSensor()          // arena
  history.Add(s)                // ❌ arena value escapes to persistent 'history'
  history.Add(persist(s))       // ✅ persist(s) allocates in the persistent region
}
```

`persist(expr)` forces allocation in the persistent region. It is also
the **single hand-off tool** between contexts: "this value must outlive
my tick" covers all three cases — a global keeps it, another task
consumes it, or an ISR will see it. On the hosted target `persist(x)`
is the identity (GC manages it) — non-breaking.

### 5.4 Per-target allocator policy

Per-target default, overridable via `--alloc=` / `amalgame.toml`:

| Target | Default allocator |
|---|---|
| ESP32 | `malloc` (libc has one) |
| Cortex-M / RISC-V | `arena` (→ `refcount` opt-in once Phase 2 lands) |
| AVR | `static` pools |

arena/malloc are a runtime macro swap (§4). refcount and static-pools
additionally need cgen support → later phases.

### 5.5 Why other models were not chosen for Phase 1

- **Refcounting** — deterministic and good for long-running programs,
  but requires real cgen work (retain/release insertion, scope-exit
  release, aliasing). Right answer for **Phase 2**, not a Phase 1 gate.
- **Static pools** — zero dynamic alloc, the AVR-survivable option;
  **required for AVR**, a complement to arena elsewhere.
- **Plain malloc** — zero new work where libc ships one → the **ESP32
  default**; lets the first port land fast.

---

## 6. Execution model — `setup` / `loop` / `region` (decided)

`setup`/`loop` are added as **soft keywords** (Arduino-familiar →
adoption), with `region {}` as the underlying primitive they desugar
to. **They also work on the hosted target** (`loop {}` ≡ `while true
{}`, `region {}` ≡ a plain lexical block with GC) — chosen so embedded
logic is **testable on a PC** before flashing. Verified non-breaking:
zero existing identifier use.

```amalgame
setup {                  // runs once → persistent allocations
  let cfg = LoadConfig()
}
loop {                   // each tick → arena reset at the top
  let line = "temp=" + temp.ToString() + "C"   // churn is fine:
  Console.WriteLine(line)                       // all of it dies next tick
}

// loop { … }  ≡  while true { region { … } }
```

This is why **string churn is a non-issue** in Phase 1: every temporary
allocated in a tick dies wholesale at the next reset, so a fixed-buffer
string type is **not needed** for Phase 1.

### 6.1 Nested loops

A `while` inside a `loop` accumulates in the tick's arena until the
outer reset — risking overflow if it allocates a lot per iteration.
`region { }` is the relief valve: a lexical sub-arena freed at `}`.
`loop` is literally "a region that resets each iteration."

### 6.2 ISRs — allocation-free, checked

An ISR preempts asynchronously (possibly mid-tick, mid-reset). It owns
no arena, allocates nothing, and touches only persistent state.

```amalgame
@isr
fn onButton() {          // marked ISR → allocation-free (checked)
  pressed = true         // writes a persistent flag
}

setup {
  let buf = persist(new RingBuffer(64))   // ISR↔main shared state
  Mcu.AttachInterrupt(Pin2, Falling, onButton)
}
loop {
  if pressed { handle(); pressed = false }
}
```

The compiler runs a reachability check: *any function reachable from an
`@isr` root must be allocation-free* (no `new`, no string concat, no
collection growth). Violation = compile error. This turns the standard
embedded discipline ("keep ISRs short, never malloc in an ISR") into a
guarantee — and it is what makes the single-owner, lock-free arena
sound.

(ISR↔main *data races* on shared state are a separate, concurrency
concern: the HAL offers `Mcu.NoInterrupts()/Interrupts()` critical
sections; a `volatile`/`@shared` qualifier is a later add. Documented,
out of the memory model's core.)

### 6.3 Tasks / multi-`loop` — deferred, but generalized

On single-core bare metal there is one superloop. Concurrency only
arises on RTOS targets (ESP32, Cortex-M+FreeRTOS). The model extends
without contradiction:

```amalgame
task sensor  { loop { … } }    // FreeRTOS task — its own arena, own tick
task network { loop { … } }    // another task — another arena
```

- **One arena per task** → single owner per arena → still no atomics,
  even on the **dual-core ESP32** (two tasks on two cores, each its own
  arena).
- The escape rule already covers cross-task: an arena value may not
  cross to another task (it would outlive the producer's tick). Pass
  data through a **queue of persistent values** — `persist(payload)`
  before enqueue.

**Phase 1 ships a single `loop`; `task` is deferred.** The memory model
generalizes cleanly, so single-loop-first paints us into no corner.
ESP32 Arduino mode also exposes a single `loop()`, so this covers the
common case immediately.

### 6.4 The escape check — exact scope (Phase 1)

Local, intraprocedural, **zero annotations, ≈zero false positives**.
Inside a `loop`/`region` body it flags an arena value flowing into a
persistent root:

1. **Direct global assignment** — `g = arenaVal`.
2. **Store through a persistent-rooted lvalue** — `g.field = arenaVal`,
   `g[i] = arenaVal` (the lvalue's base symbol is a global or a `setup`
   binding).
3. **Stdlib collection mutators** — `persistentList.Add(arenaVal)`,
   `persistentMap.Set(k, arenaVal)`, `persistentSet.Add(arenaVal)`. The
   compiler knows the storing methods — exactly `{List.Add (List_add),
   Map.Set (Map_set), Set.Add (Set_add)}`. Read methods
   (`Contains`/`Has`/`Get`/`Size`/`Any`/`All`/`CountIf`) are **not** in
   the set, so they never false-flag.

Value-origin approximation (RHS): a local is "maybe-arena" if bound in
this scope to a fresh allocation (`new`, concat, ctor, a call returning
a heap type); a value read from a global or from `persist(…)` is
persistent. Incoming function args are treated as opaque (no
interprocedural tracking) → neither flagged nor false-flagged.

**Assumed gap (documented → Phase 2 upgrade):** escape through a
**user-defined storing method** (`myObj.Stash(s)`) or **interprocedural
flow** (a function returning its arg, then stored) is not caught in
Phase 1. Upgrade path: `@stores`/`@borrow` annotations or effect
inference — *without changing anything for devs on the common patterns*.

---

## 7. Type & numeric model

- **`int` default per target** (`i16` on AVR, `i32` elsewhere). Explicit
  `i64`/`i32`/`i16` always honored.
- **Floats:** on no-FPU targets `f64` pulls in soft-float (kilobytes).
  Policy: **warn** on `f64` for no-FPU 32-bit targets; **hard error** on
  AVR. FPU targets unaffected. (First-class fixed-point is a separate
  language question, deferred.)
- **Strings:** the GC'd `char*` churn is fine under arena-with-reset (see
  §6). A fixed-buffer string type is a known **post-Phase-1** follow-up,
  not required now.

---

## 8. I/O & stdlib subset

The `features` map gates the typechecker/resolver so unsupported
imports fail at **compile time with a clear message**, not at link.

| Capability | MCU status |
|---|---|
| `Console.Write*` | ✅ retargeted to board `code_putc` (UART/semihosting/RTT) |
| Arithmetic / control flow / structs / methods | ✅ core, unchanged |
| `Collections` (List/Map/Set) | ⚠️ allowed but allocator-bound; document caps |
| `String` ops | ⚠️ allowed under arena; heavy use discouraged |
| `IO` (files) | ❌ off by default (no filesystem) |
| `Net` / `Process` / threads | ❌ off by default (host-only) |
| **GPIO / timers / ADC / I2C / SPI** | 🆕 the `Mcu` HAL — §9 |

---

## 9. Hardware access — the `Amalgame.Mcu` HAL (decided)

To be *useful*, MCU targets need a hardware API. One ergonomic AM-level
facade must map onto five very different backends (Arduino core, STM32
HAL/libopencm3, pico-sdk, ESP-IDF, RISC-V vendor SDKs).

### 9.1 Core principle — portable facade + board-supplied typed `Pin`

The backends do not even agree on what a pin *is* (an int on Arduino, a
`(port,pin)` pair + clock-enable + init struct on STM32, a GPIO number
on RP2040/ESP32). Resolution: the **board package supplies the pin
namespace**; the core `Mcu` API operates on an **opaque, typed `Pin`**.

Pins are **named typed constants** the board provides — reads like
Arduino, but type-safe and free to represent `Pin` however the target
wants:

```amalgame
import Amalgame.Mcu
import Amalgame.Mcu.Board        // resolves to the board pinned in amalgame.toml

let led = Board.LedBuiltin       // : Pin  (an int on AVR, a struct on STM32)
Mcu.PinMode(led, Output)
loop {
  Mcu.Toggle(led)
  Mcu.DelayMs(500)
}
// Mcu.DigitalWrite(42, High)  ❌ error: expected Pin, got int
```

### 9.2 Layering — core declares, board implements

Because the implementation *is* vendor-specific
(`HAL_GPIO_WritePin` vs `gpio_put`), responsibilities split:

| Layer | Contents | Where |
|---|---|---|
| **`Amalgame.Mcu`** (core) | types (`Pin` opaque, `Level{High,Low}`, `PinMode{Output,…}`, `Edge`) + function **declarations** (`extern`) | amc bundle or a base package |
| **`Amalgame.Mcu.Board`** (per board) | concrete `Pin` representation + named pin constants + function **bodies** (vendor mapping) | `amalgame-mcu-<board>` package |

This is the existing package convention, with `Pin` as an opaque type
the board header `typedef`s per target. The same `Board.LedBuiltin`
maps differently:

```c
/* amalgame-mcu-stm32f407 */
typedef struct { GPIO_TypeDef* port; uint16_t pin; } code_Pin;
#define Board_LedBuiltin ((code_Pin){GPIOD, GPIO_PIN_12})
static inline void Mcu_DigitalWrite(code_Pin p, int lvl){
    HAL_GPIO_WritePin(p.port, p.pin, lvl); }

/* amalgame-mcu-uno */
typedef uint8_t code_Pin;
#define Board_LedBuiltin ((code_Pin)13)
static inline void Mcu_DigitalWrite(code_Pin p, int lvl){
    digitalWrite(p, lvl); }
```

```toml
# amalgame-mcu-stm32f407 / amalgame.toml
[stdlib]
classes   = ["Board"]
namespace = "Amalgame.Mcu.Board"
header    = "runtime/Amalgame_Mcu_Board.h"
libs      = ["opencm3_stm32f4"]   # or the vendor HAL
```

Source-level code stays **portable**: flipping `board = "stm32f407"` →
`"uno"` in `amalgame.toml` recompiles the same
`loop { Mcu.Toggle(Board.LedBuiltin) }` for the other chip, untouched.

### 9.3 Surface

**Core `Mcu` (Phase 1 — what ~95% of projects need):**

```
// Digital I/O
Mcu.PinMode(pin, Output | Input | InputPullup | InputPulldown)
Mcu.DigitalWrite(pin, High | Low)
Mcu.DigitalRead(pin): Level
Mcu.Toggle(pin)
// Analog
Mcu.AnalogRead(pin): int          // raw; AnalogReadResolution(bits) normalizes 10/12-bit
Mcu.PwmWrite(pin, duty)
// Timing
Mcu.DelayMs(ms)  Mcu.DelayUs(us)
Mcu.Millis(): int  Mcu.Micros(): int
// Interrupts (see §6.2)
Mcu.AttachInterrupt(pin, Rising | Falling | Change, handler)   // handler @isr
Mcu.DetachInterrupt(pin)
Mcu.NoInterrupts()  Mcu.Interrupts()        // critical section
```

**Sub-modules (Phase 4 — buses, multiple instances, persistent state):**
`Amalgame.Mcu.I2c`, `.Spi`, `.Serial`. Deferred: they carry multiple
instances (`let bus = I2c.Open(0)` in `setup`, persistent) and a heavier
surface. Note `Console.Write*` already retargets to UART via the runtime
profile, so a basic serial console is free in Phase 1.

Backend mapping (core API stays uniform):

| AM call | AVR | STM32 HAL | RP2040 | ESP-IDF |
|---|---|---|---|---|
| `PinMode(p,Output)` | `pinMode` | `RCC_CLK_ENABLE`+`HAL_GPIO_Init` | `gpio_init`+`set_dir` | `gpio_set_direction` |
| `DigitalWrite(p,High)` | `digitalWrite` | `HAL_GPIO_WritePin` | `gpio_put` | `gpio_set_level` |
| `DelayMs` | `delay` | `HAL_Delay` | `sleep_ms` | `vTaskDelay` |
| `AttachInterrupt` | `attachInterrupt` | EXTI + NVIC | `gpio_set_irq_enabled` | `gpio_isr_handler_add` |

---

## 10. Build / toolchain integration (plumbing)

Touch points in [`src/main.am`](../../src/main.am):

1. **Header selection** ([~685](../../src/main.am#L685)): emit the
   embedded `_runtime.h` + profile macros under an embedded target; skip
   `GC_INIT()`.
2. **Compile/link line** ([build ~1584](../../src/main.am#L1584)):
   replace the hardcoded `gcc … -lgc -lm -lz -ldl -lpthread` with the
   descriptor's `cc` + `cc_flags` + `ld_flags`; **no** host libs.
3. **Output products:** `.elf`, plus `objcopy` → `.bin`/`.hex`; an
   optional `--flash` shelling to `openocd`/`esptool`/`avrdude`.
4. **`amalgame.toml [target]` block** so a project pins its board:

```toml
[target]
mcu        = "cortex-m4"
board      = "stm32f407"
linker     = "boards/stm32f407.ld"
allocator  = "arena"
arena_size = 8192
```

Toolchain presence checked in a preflight (like the existing gcc
preflight) with an actionable per-arch install hint.

---

## 11. Phasing

**Phase 0 — spike (proof it links).** Hand-write a freestanding
`_runtime.h` (arena alloc, `code_putc` over semihosting); compile a
trivial `loop { Console.WriteLine … }` / blink with
`arm-none-eabi-gcc`; run in QEMU (`qemu-system-arm`). Goal: an ELF that
prints/blinks. No driver changes yet — proves the runtime story.

**Phase 1 — ESP32 + Cortex-M (incl. STM32) MVP.** Target descriptor +
driver plumbing; `runtime/embedded/` with arena (Cortex-M) and malloc
(ESP32); `setup`/`loop`/`region` + `persist` + the conservative escape
check; `@isr` + allocation-free check; `Amalgame.Mcu` core HAL with a
first board package; `--target`, `--alloc`, `.bin`/`.hex` output; the
golden-C hosted regression test. Deliverable: blink + serial-print on
real or QEMU'd hardware.

**Phase 2 — refcount + RISC-V.** cgen refcount insertion for
long-running programs; RISC-V descriptor (reuses the Cortex-M path);
optional escape-check upgrade (`@stores`/`@borrow`).

**Phase 3 — AVR.** Static pools, `i16` int default, hard `f64` ban,
avr-libc. The port that validates every constraint.

**Phase 4 — HAL breadth + tasks + flashing.** Per-board `amalgame-mcu-*`
packages (GPIO/timer/ADC/I2C/SPI), the `task` concurrency model,
`--flash` integration, examples.

---

## 12. Decisions log (frozen 2026-05-31)

| Topic | Decision |
|---|---|
| Backend | Reuse C gen (Option A); no LLVM/asm backend. |
| Compatibility | Hosted frozen; all embedded opt-in behind `--target`; golden-C test guards hosted codegen. |
| Allocation boundary | `setup`/`loop` (soft keywords) + `region {}` primitive; **also work on hosted** (testability). |
| Arena reset | Implicit at the top of each `loop` tick. |
| Allocation origins | arena (default in loop) / persistent (globals, `setup`, `persist(…)`). |
| Escape rule | `arena ↛ persistent`; valve `persist(…)` (identity on hosted). |
| Escape check (Phase 1) | global `=`, persistent-rooted lvalue store, `{List.Add, Map.Set, Set.Add}`; interproc/user-method gap → Phase 2. |
| Memory model invariant | one arena per execution context; single-owner ⇒ lock-free reset. |
| ISRs | `@isr`, allocation-free (reachability-checked); persistent/static state only. |
| Tasks | deferred to Phase 4; model generalizes (one arena per task, escape extends cross-task). |
| Strings | arena churn acceptable; no fixed-buffer type in Phase 1. |
| Floats | target-driven: FPU ✅ / no-FPU warn / AVR hard error. |
| HAL | portable `Mcu` facade + opaque typed `Pin`; named pin constants from per-board package; core declares / board implements. |
| `_runtime.h` | single tree, `#if` arms default to hosted. |

## 13. Remaining (plumbing only — no open conceptual decisions)

- `amalgame.toml [target]` parsing + per-board linker scripts.
- `--flash` integration (openocd / esptool / avrdude).
- The Phase 0 QEMU spike.
- Opaque `Pin` C-representation detail (`typedef code_Pin` per target).
- ISR↔main shared-state concurrency story (`volatile`/`@shared`, critical
  sections) — later add, documented now.

---

## 14. Why this is *not* a new code generator

amc's gen already produces portable C99; the C cross-toolchains for all
four target classes are mature. The leverage is entirely in the
**runtime + driver target layer**. A direct LLVM/asm backend (Option B)
would re-derive what gcc/clang already do for these ISAs while adding a
second 5000-LOC generator to maintain — deferred until (and unless) the
C profile demonstrably can't meet a footprint or latency budget.
