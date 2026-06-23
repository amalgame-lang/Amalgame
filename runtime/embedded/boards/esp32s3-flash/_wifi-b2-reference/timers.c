/* timers.c — ETS software timers for the Route-B WiFi OS adapter (B3).
 *
 * The WiFi blobs schedule work through `ets_timer_*` (osi `_timer_arm/_disarm/
 * _done/_setfn/_arm_us`). Each timer is an ETSTimer the blob owns (5 words:
 * next, expire, period, func, arg — esp32s3 rom/ets_sys.h); we only ever touch
 * it through these functions, so the layout below mirrors that struct exactly.
 *
 * Driver: cooperative — `amc_timers_check()` is called from the scheduler's
 * yield point (sched.c), i.e. in task context whenever any task blocks/yields.
 * That matches the non-ISR ets_timer semantics (callbacks run in task context,
 * not in an interrupt). Resolution is the yield cadence, which is fine for the
 * WiFi stack's ms-scale timers (beacon/scan/assoc timeouts).
 *
 * Time base: a 32-bit microsecond clock derived from the 16 MHz systimer.
 * Deadlines compare with a wrap-safe signed difference (`(int32_t)(now-exp)>=0`),
 * good for any interval well under ~71 min — all WiFi timers qualify. */
#include <stdint.h>
#include <stddef.h>

typedef void (*amc_tmrfn)(void *arg);

/* Mirror of the esp32s3 ROM ETSTimer (do not reorder — the blob allocates it). */
typedef struct amc_tmr {
    struct amc_tmr *next;     /* timer_next  */
    uint32_t        expire;   /* timer_expire — absolute us deadline (our clock) */
    uint32_t        period;   /* timer_period — 0 = one-shot, else reload us      */
    amc_tmrfn       func;     /* timer_func  */
    void           *arg;      /* timer_arg   */
} amc_tmr;

extern uint64_t sched_now(void);            /* systimer ticks (16 MHz)            */
extern uint32_t sched_int_disable(void);    /* critical section (rsil)            */
extern void     sched_int_restore(uint32_t);

static amc_tmr *g_head;                      /* singly-linked active list          */

static inline uint32_t now_us(void) { return (uint32_t)(sched_now() / 16u); }

/* list helpers (caller holds the critical section) */
static void list_remove(amc_tmr *t) {
    amc_tmr **pp = &g_head;
    while (*pp) { if (*pp == t) { *pp = t->next; t->next = NULL; return; } pp = &(*pp)->next; }
}
static int list_present(amc_tmr *t) {
    for (amc_tmr *p = g_head; p; p = p->next) if (p == t) return 1;
    return 0;
}
static void list_add(amc_tmr *t) {
    if (!list_present(t)) { t->next = g_head; g_head = t; }
}

void amc_timer_setfn(void *timer, void *fn, void *arg) {
    amc_tmr *t = (amc_tmr *)timer;
    uint32_t s = sched_int_disable();
    t->func = (amc_tmrfn)fn; t->arg = arg; t->next = NULL;
    sched_int_restore(s);
}

static void arm_common(amc_tmr *t, uint32_t us, int repeat) {
    uint32_t s = sched_int_disable();
    t->expire = now_us() + us;
    t->period = repeat ? us : 0u;
    list_add(t);
    sched_int_restore(s);
}
void amc_timer_arm(void *timer, uint32_t ms, _Bool repeat) {
    arm_common((amc_tmr *)timer, ms * 1000u, repeat);
}
void amc_timer_arm_us(void *timer, uint32_t us, _Bool repeat) {
    arm_common((amc_tmr *)timer, us, repeat);
}
void amc_timer_disarm(void *timer) {
    uint32_t s = sched_int_disable();
    list_remove((amc_tmr *)timer);
    sched_int_restore(s);
}
void amc_timer_done(void *timer) {
    amc_tmr *t = (amc_tmr *)timer;
    uint32_t s = sched_int_disable();
    list_remove(t); t->func = NULL;
    sched_int_restore(s);
}

/* Fire all expired timers. Called from the scheduler's yield point (task
 * context). A callback may re-arm/disarm timers, so each fire is decided under
 * the lock (remove one-shots / reschedule periodics before unlocking), the
 * callback runs unlocked, then we restart the walk. */
void amc_timers_check(void) {
    for (;;) {
        uint32_t s = sched_int_disable();
        uint32_t now = now_us();
        amc_tmr *fire = NULL;
        for (amc_tmr *p = g_head; p; p = p->next) {
            if ((int32_t)(now - p->expire) >= 0) { fire = p; break; }
        }
        if (!fire) { sched_int_restore(s); return; }
        amc_tmrfn fn = fire->func; void *arg = fire->arg;
        if (fire->period) fire->expire = now + fire->period;   /* reschedule */
        else              list_remove(fire);                   /* one-shot   */
        sched_int_restore(s);
        if (fn) fn(arg);
    }
}
