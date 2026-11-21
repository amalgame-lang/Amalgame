/*
 * heap.c — minimal runtime heap for the ESP32-S3 bare-metal amc runtime
 * (Route B, step B0b-3). The WiFi blobs allocate at runtime through the
 * wifi_osi_funcs_t malloc/free pointers, so we need a real allocator over a
 * DRAM region before B2. This is a compact first-fit free-list allocator with
 * boundary-free coalescing — enough to bring up malloc/free/calloc now; a
 * caps-aware (DMA/internal) allocator can come later when B0c gives real room.
 *
 * The heap bank is carved by board.ld (_heap_start.._heap_end, ~320 KB of free
 * low internal SRAM). Init is lazy (first allocation) so startup ordering is
 * untouched. 16-byte alignment throughout (header is 16 B → payloads 16-aligned,
 * which the Xtensa ABI and DMA both like).
 */
#include <stdint.h>
#include <stddef.h>

extern uint8_t _heap_start;
extern uint8_t _heap_end;

#define HEAP_ALIGN   16u
#define HDR_SZ       16u                 /* keeps payload 16-aligned */

typedef struct blk {
    uint32_t     size;                   /* total block size incl header, 16-mult */
    uint32_t     _pad;                   /* (header padded to 16 B)               */
    struct blk*  next;                   /* free-list link (valid only when free) */
} blk_t;

static blk_t*   g_free;                  /* address-ordered free list */
static int      g_init;

/* The free list is shared between tasks and the WiFi MAC ISR (which allocates RX
 * buffers via osi _malloc). Guard every mutation with a critical section — raise
 * PS.INTLEVEL to mask level-1 interrupts (systimer + MAC), restore on exit.
 * Without this the list corrupts and hands out wild pointers. */
static inline uint32_t heap_lock(void)   { uint32_t ps; __asm__ volatile("rsil %0, 3" : "=r"(ps)); return ps; }
static inline void heap_unlock(uint32_t ps) { __asm__ volatile("wsr.ps %0\n rsync\n" :: "r"(ps) : "memory"); }
static uint32_t g_total, g_used;

static inline uint32_t align_up(uint32_t v) {
    return (v + (HEAP_ALIGN - 1u)) & ~(HEAP_ALIGN - 1u);
}

static void heap_init(void) {
    uint8_t* s = (uint8_t*)align_up((uint32_t)(uintptr_t)&_heap_start);
    uint8_t* e = &_heap_end;
    blk_t* b = (blk_t*)s;
    b->size = (uint32_t)(e - s) & ~(HEAP_ALIGN - 1u);
    b->next = NULL;
    g_free  = b;
    g_total = b->size;
    g_used  = 0;
    g_init  = 1;
}

void* amc_malloc(size_t want) {
    if (!g_init) heap_init();
    if (want == 0) return NULL;
    uint32_t need = align_up((uint32_t)want) + HDR_SZ;

    uint32_t lock = heap_lock();
    blk_t* prev = NULL;
    blk_t* cur  = g_free;
    while (cur) {
        if (cur->size >= need) {
            /* split if the remainder can hold a header + a minimal payload */
            if (cur->size >= need + HDR_SZ + HEAP_ALIGN) {
                blk_t* rest = (blk_t*)((uint8_t*)cur + need);
                rest->size  = cur->size - need;
                rest->next  = cur->next;
                cur->size   = need;
                if (prev) prev->next = rest; else g_free = rest;
            } else {
                if (prev) prev->next = cur->next; else g_free = cur->next;
            }
            g_used += cur->size;
            heap_unlock(lock);
            return (uint8_t*)cur + HDR_SZ;
        }
        prev = cur;
        cur  = cur->next;
    }
    heap_unlock(lock);
    return NULL;                          /* out of memory */
}

void amc_free(void* p) {
    if (!p) return;
    blk_t* b = (blk_t*)((uint8_t*)p - HDR_SZ);
    uint32_t lock = heap_lock();
    g_used -= b->size;

    /* insert address-ordered, then coalesce with neighbours */
    blk_t* prev = NULL;
    blk_t* cur  = g_free;
    while (cur && cur < b) { prev = cur; cur = cur->next; }
    b->next = cur;
    if (prev) prev->next = b; else g_free = b;

    /* coalesce with next if adjacent */
    if (cur && (uint8_t*)b + b->size == (uint8_t*)cur) {
        b->size += cur->size;
        b->next  = cur->next;
    }
    /* coalesce with prev if adjacent */
    if (prev && (uint8_t*)prev + prev->size == (uint8_t*)b) {
        prev->size += b->size;
        prev->next  = b->next;
    }
    heap_unlock(lock);
}

void* amc_calloc(size_t n, size_t sz) {
    uint32_t total = (uint32_t)n * (uint32_t)sz;
    if (n != 0 && total / n != sz) return NULL;     /* overflow */
    void* p = amc_malloc(total);
    if (p) {
        uint8_t* q = (uint8_t*)p;
        for (uint32_t i = 0; i < total; ++i) q[i] = 0;
    }
    return p;
}

/* Introspection (used by the B0b-3 acceptance test and later by heap_caps-style
 * queries). free_bytes is approximate: total minus in-use payload+headers. */
uint32_t amc_heap_total(void) { if (!g_init) heap_init(); return g_total; }
uint32_t amc_heap_used(void)  { if (!g_init) heap_init(); return g_used;  }
uint32_t amc_heap_free(void)  { if (!g_init) heap_init(); return g_total - g_used; }
