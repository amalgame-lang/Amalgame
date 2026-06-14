/*
 * CODE Language Runtime - EMBEDDED / freestanding profile
 * Copyright (c) 2026 Bastien MOUGET
 *
 * Phase 0 spike (docs/proposals/amc-embedded.md §4, §5, §12).
 *
 * This is the freestanding sibling of runtime/_runtime.h. It exposes the
 * SAME symbol surface that c_gen.am emits (code_alloc, code_string,
 * code_string_concat, code_int_to_string, Console_WriteLine, ...) so that
 * generated C is byte-identical between targets — but it is welded to NO
 * host: no <gc.h>, no <stdio.h>, no <stdlib.h>, no malloc, no libc string.
 *
 *   - Allocation        : region-per-tick bump arena (§5), reset = top = 0.
 *   - persist(...)       : a separate never-reset region (the escape valve).
 *   - Console / putc     : ARM semihosting (works under QEMU & a real probe).
 *
 * Selection in Phase 1 will be driver-emitted:
 *     #include "runtime/embedded/_runtime.h"
 *     #define AMC_TARGET_CORTEX_M 1
 *     #define AMC_ALLOC_ARENA 1
 * For the spike we just assume arena + semihosting.
 */
#ifndef CODE_RUNTIME_EMBEDDED_H
#define CODE_RUNTIME_EMBEDDED_H

#include <stdint.h>
#include <stddef.h>

/* ------------------------------------------------------------------ *
 *  Base types — IDENTICAL names to the hosted runtime so cgen output
 *  does not change. (int default narrowing is a typechecker concern,
 *  not a typedef one; explicit i64/f64 stay honored.)
 * ------------------------------------------------------------------ */
typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef double   f64;
typedef float    f32;
typedef uint8_t  u8;
typedef char*    code_string;
typedef int      code_bool;   /* no <stdbool.h>; matches amc's code_bool lowering */

#ifndef AMC_ARENA_SIZE
#define AMC_ARENA_SIZE   8192   /* per-tick scratch; overridable via amalgame.toml [target] */
#endif
#ifndef AMC_PERSIST_SIZE
#define AMC_PERSIST_SIZE 4096   /* globals + setup{} + persist(...) */
#endif

/* ------------------------------------------------------------------ *
 *  Tiny freestanding mem/str helpers (do NOT pull newlib's libc).
 * ------------------------------------------------------------------ */
static inline size_t amc_strlen(const char* s) {
    size_t n = 0; if (!s) return 0; while (s[n]) n++; return n;
}
static inline void amc_memcpy(void* d, const void* s, size_t n) {
    u8* dd = (u8*)d; const u8* ss = (const u8*)s;
    for (size_t i = 0; i < n; i++) dd[i] = ss[i];
}
static inline int amc_strcmp(const char* a, const char* b) {
    if (!a) a = "";
    if (!b) b = "";
    while (*a && (*a == *b)) { a++; b++; }
    return (int)(u8)*a - (int)(u8)*b;
}

/* ------------------------------------------------------------------ *
 *  §5 region-per-tick arena + persistent region.
 * ------------------------------------------------------------------ */
static u8     amc_arena[AMC_ARENA_SIZE];
static size_t amc_arena_top = 0;
static size_t amc_arena_hwm = 0;   /* high-water mark, for the spike report */

static u8     amc_persist[AMC_PERSIST_SIZE];
static size_t amc_persist_top = 0;

/* set by persist(...) so the allocation inside the expr lands persistent */
static int    amc_alloc_persistent = 0;

static inline void amc_oom(void); /* fwd: traps on overflow */

#define AMC_ALIGN_UP(n) (((n) + 7u) & ~((size_t)7u))

static inline void* code_alloc(size_t n) {
    n = AMC_ALIGN_UP(n);
    if (amc_alloc_persistent) {
        if (amc_persist_top + n > AMC_PERSIST_SIZE) amc_oom();
        void* p = &amc_persist[amc_persist_top];
        amc_persist_top += n;
        return p;
    }
    if (amc_arena_top + n > AMC_ARENA_SIZE) amc_oom();
    void* p = &amc_arena[amc_arena_top];
    amc_arena_top += n;
    if (amc_arena_top > amc_arena_hwm) amc_arena_hwm = amc_arena_top;
    return p;
}

/* §6: implicit at the top of each loop tick. Single owner ⇒ no atomics. */
static inline void amc_arena_reset(void) { amc_arena_top = 0; }

/* §6.1 region {}: a lexical sub-arena — mark on entry, release at the brace.
 * A runtime mark stack (not a C local) so nested regions compose and each
 * push/pop can live in its own spliced compound statement. */
#ifndef AMC_MARK_STACK_DEPTH
#define AMC_MARK_STACK_DEPTH 16
#endif
static size_t amc_mark_stack[AMC_MARK_STACK_DEPTH];
static int    amc_mark_sp = 0;
static inline void amc_arena_push_mark(void) {
    if (amc_mark_sp < AMC_MARK_STACK_DEPTH) { amc_mark_stack[amc_mark_sp] = amc_arena_top; }
    amc_mark_sp++;
}
static inline void amc_arena_pop_mark(void) {
    amc_mark_sp--;
    if (amc_mark_sp >= 0 && amc_mark_sp < AMC_MARK_STACK_DEPTH) { amc_arena_top = amc_mark_stack[amc_mark_sp]; }
}

/* setup {}: allocations inside run once and live forever — route them to the
 * persistent region so the per-tick arena reset never wipes them (§5.2). */
static inline void amc_persist_begin(void) { amc_alloc_persistent++; }
static inline void amc_persist_end(void) { amc_alloc_persistent--; }

/* ── List<T> on the arena ────────────────────────────────────────────────────
 * Same ABI as the hosted AmalgameList (runtime/_runtime.h) so the cgen emits
 * identical New/Add/Get/Set/Count calls on both targets. Backing memory comes
 * from code_alloc, i.e. it follows the active region: a list built inside loop{}
 * is reclaimed at the next arena reset (transient frame/byte buffers); a list
 * built under setup{} / persist(...) lives in the persistent region (the jitter
 * ring). Growth abandons the old buffer — fine for transient lists (reset wipes
 * the waste) and for persistent lists that are pre-sized once in a constructor.
 * No <string.h> dependency here (the embedded prelude pulls only stdint/stddef),
 * so the grow copy is an explicit loop. Tune AMC_ARENA_SIZE / AMC_PERSIST_SIZE in
 * the board [target] config to fit the buffers (the 8K/4K defaults are blink-sized).
 */
typedef struct {
    void** data;
    int    size;
    int    capacity;
} AmalgameList;

static inline AmalgameList* AmalgameList_new(void) {
    AmalgameList* l = (AmalgameList*) code_alloc(sizeof(AmalgameList));
    l->capacity = 8;
    l->size     = 0;
    l->data     = (void**) code_alloc(sizeof(void*) * 8);
    return l;
}

static inline void AmalgameList_add(AmalgameList* l, void* item) {
    if (l->size >= l->capacity) {
        int nc = l->capacity * 2;
        void** nd = (void**) code_alloc(sizeof(void*) * (size_t) nc);
        for (int k = 0; k < l->size; k++) nd[k] = l->data[k];
        l->data     = nd;
        l->capacity = nc;
    }
    l->data[l->size++] = item;
}

static inline void* AmalgameList_get(AmalgameList* l, int i) {
    if (i < 0 || i >= l->size) return (void*) 0;
    return l->data[i];
}

static inline void AmalgameList_set(AmalgameList* l, int index, void* item) {
    if (!l || index < 0 || index >= l->size) return;
    l->data[index] = item;
}

static inline int AmalgameList_count(AmalgameList* l) {
    return l->size;
}

static inline code_string code_strdup(const char* s) {
    if (!s) return NULL;
    size_t n = amc_strlen(s) + 1;
    char* r = (char*)code_alloc(n);
    amc_memcpy(r, s, n);
    return r;
}

/* §5.3 persist(expr): force the allocations done while evaluating expr into
 * the persistent region so the value outlives the per-tick arena reset. The
 * statement-expression brackets the eval with the persistent flag and yields
 * the value with its original type (type-transparent — see the typechecker):
 *     let cfg = persist(new Config())   // survives every loop tick
 * On hosted this is the identity macro (the GC keeps it alive). */
#define persist(x) ({ amc_alloc_persistent++; __typeof__(x) __amc_pv = (x); amc_alloc_persistent--; __amc_pv; })

/* For a value already living in the arena, deep-copy it persistent: */
static inline code_string code_persist_string(const char* s) {
    int save = amc_alloc_persistent; amc_alloc_persistent = 1;
    code_string r = code_strdup(s);
    amc_alloc_persistent = save;
    return r;
}

/* ------------------------------------------------------------------ *
 *  String helpers — same names as hosted, arena-backed.
 * ------------------------------------------------------------------ */
static inline code_string code_string_concat(code_string a, code_string b) {
    if (!a) a = "";
    if (!b) b = "";
    size_t la = amc_strlen(a), lb = amc_strlen(b);
    char* r = (char*)code_alloc(la + lb + 1);
    amc_memcpy(r, a, la);
    amc_memcpy(r + la, b, lb + 1);
    return r;
}
static inline code_bool code_string_equals(code_string a, code_string b) {
    return amc_strcmp(a, b) == 0;
}
/* no vsnprintf: tiny base-10 int formatter into the arena */
static inline code_string code_int_to_string(i64 n) {
    char tmp[24];
    int i = 23; tmp[i--] = '\0';
    int neg = (n < 0);
    uint64_t u = neg ? (uint64_t)(-(n + 1)) + 1u : (uint64_t)n;
    if (u == 0) tmp[i--] = '0';
    while (u) { tmp[i--] = (char)('0' + (u % 10)); u /= 10; }
    if (neg) tmp[i--] = '-';
    return code_strdup(&tmp[i + 1]);
}

/* String.From* family — same names cgen emits for .ToString()/interpolation,
 * arena-backed. (Collections/regex/etc. stay feature-gated off; a program
 * that uses them fails to link freestanding — by design, §8.) */
static inline code_string String_FromInt(i64 n) { return code_int_to_string(n); }
static inline code_string String_FromBool(code_bool b) { return b ? "true" : "false"; }
static inline code_string String_FromByte(i64 b) {
    char* s = (char*)code_alloc(2);
    s[0] = (char)(b & 0xFF); s[1] = '\0';
    return s;
}
static inline code_string String_FromCodepoint(i64 cp) {
    char* s;
    if (cp < 0x80) { s = (char*)code_alloc(2); s[0] = (char)cp; s[1] = '\0'; }
    else if (cp < 0x800) {
        s = (char*)code_alloc(3);
        s[0] = (char)(0xC0 | (cp >> 6)); s[1] = (char)(0x80 | (cp & 0x3F)); s[2] = '\0';
    } else {
        s = (char*)code_alloc(4);
        s[0] = (char)(0xE0 | (cp >> 12));
        s[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        s[2] = (char)(0x80 | (cp & 0x3F)); s[3] = '\0';
    }
    return s;
}
/* Crude float→string without libm/soft-float dependency on FPU-less parts:
 * integer part + 3 decimals. f64 on no-FPU targets is discouraged (§7). */
static inline code_string String_FromFloat(f64 n) {
    i64 ip = (i64)n;
    f64 frac = n - (f64)ip;
    if (frac < 0) { frac = -frac; }
    i64 milli = (i64)(frac * 1000.0);
    code_string a = code_int_to_string(ip);
    code_string b = code_int_to_string(milli);
    return code_string_concat(code_string_concat(a, "."), b);
}

/* ------------------------------------------------------------------ *
 *  §4 / §8 Console + code_putc via ARM semihosting.
 *  bkpt 0xAB is the semihosting trap; op in r0, arg in r1.
 * ------------------------------------------------------------------ */
#define AMC_SYS_WRITEC 0x03
#define AMC_SYS_WRITE0 0x04
#define AMC_SYS_EXIT   0x18

static inline int amc_semihost(int op, void* arg) {
    register int r0 __asm__("r0") = op;
    register void* r1 __asm__("r1") = arg;
    __asm__ volatile ("bkpt 0xAB"
                      : "+r"(r0)
                      : "r"(r1)
                      : "memory");
    return r0;
}
/* Board header (set by the driver as AMC_BOARD_HEADER for an external
 * --board build) is pulled in HERE — after the base types/semihosting,
 * before the Console + Mcu defaults — so it can define AMC_HAVE_CONSOLE
 * (e.g. Console over UART) and/or AMC_HAVE_MCU_BOARD and supply the real
 * bodies, suppressing the QEMU/semihosting defaults below. */
#ifdef AMC_BOARD_HEADER
#include AMC_BOARD_HEADER
#endif

/* Console default = ARM semihosting. A board can override it (e.g. route to
 * a UART) by defining AMC_HAVE_CONSOLE + its own Console_Write/WriteLine. */
#ifndef AMC_HAVE_CONSOLE
static inline void code_putc(char c) {
    char ch = c;
    amc_semihost(AMC_SYS_WRITEC, &ch);
}
static inline void Console_Write(code_string s) {
    if (!s) return;
    amc_semihost(AMC_SYS_WRITE0, (void*)s);
}
static inline void Console_WriteLine(code_string s) {
    Console_Write(s);
    code_putc('\n');
}
#endif

/* clean exit so QEMU returns instead of hanging */
static inline void amc_exit(int code) {
    uint32_t block[2] = { 0x20026u /* ADP_Stopped_ApplicationExit */, (uint32_t)code };
    amc_semihost(AMC_SYS_EXIT, block);
    for (;;) {}
}
static inline void amc_oom(void) {
    Console_WriteLine("!! arena/persist overflow");
    amc_exit(1);
}

/* ------------------------------------------------------------------ *
 *  §9 Amalgame.Mcu HAL — default "virtual board" implementation.
 *
 *  A real board (e.g. amalgame-mcu-nucleo-f767zi) overrides these by
 *  defining AMC_HAVE_MCU_BOARD and supplying its own Mcu_* (libopencm3 /
 *  STM32 HAL / pico-sdk). The default below targets QEMU/dev: it logs pin
 *  operations over semihosting so a blink is *observable* with -nographic,
 *  and tracks pin levels so DigitalRead/Toggle behave. Pins/levels/modes
 *  are plain int for now (the opaque typed Pin is a documented follow-up).
 *
 *  (The board header — if any — was already pulled in above, before the
 *  Console default, so it may define AMC_HAVE_MCU_BOARD to suppress this.)
 * ------------------------------------------------------------------ */
#ifndef AMC_HAVE_MCU_BOARD

#ifndef AMC_MCU_MAX_PINS
#define AMC_MCU_MAX_PINS 64
#endif
static u8       amc_pin_level[AMC_MCU_MAX_PINS];
static uint32_t amc_millis_counter = 0;

static inline int  Mcu_High(void)   { return 1; }
static inline int  Mcu_Low(void)    { return 0; }
static inline int  Mcu_Output(void) { return 1; }
static inline int  Mcu_Input(void)  { return 0; }

static inline void amc_mcu_log(const char* what, i64 pin, i64 val) {
    Console_Write((code_string)what);
    Console_Write(" pin=");
    Console_Write(code_int_to_string(pin));
    Console_Write(" =");
    Console_WriteLine(code_int_to_string(val));
}

static inline void Mcu_PinMode(i64 pin, i64 mode) { amc_mcu_log("PINMODE", pin, mode); }
static inline void Mcu_DigitalWrite(i64 pin, i64 level) {
    if (pin >= 0 && pin < AMC_MCU_MAX_PINS) { amc_pin_level[pin] = (u8)(level ? 1 : 0); }
    amc_mcu_log("GPIO", pin, level ? 1 : 0);
}
static inline i64 Mcu_DigitalRead(i64 pin) {
    if (pin >= 0 && pin < AMC_MCU_MAX_PINS) { return amc_pin_level[pin]; }
    return 0;
}
static inline void Mcu_Toggle(i64 pin) {
    i64 nv = 0;
    if (pin >= 0 && pin < AMC_MCU_MAX_PINS) { nv = amc_pin_level[pin] ? 0 : 1; amc_pin_level[pin] = (u8)nv; }
    amc_mcu_log("TOGGLE", pin, nv);
}
/* On the virtual board DelayMs is a no-op (QEMU has no wall clock here);
 * Millis returns a monotonically increasing tick so timing code progresses. */
static inline void Mcu_DelayMs(i64 ms) { (void)ms; amc_millis_counter += (uint32_t)ms; }
static inline i64  Mcu_Millis(void)    { return (i64)(amc_millis_counter++); }

#endif /* AMC_HAVE_MCU_BOARD */

#endif /* CODE_RUNTIME_EMBEDDED_H */
