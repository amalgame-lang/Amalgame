#include <stdint.h>
extern void* amc_malloc(uint32_t); extern void amc_free(void*);
extern int  sched_block(void* obj, uint64_t deadline);
extern int  sched_wake(void* obj);
extern uint64_t sched_now(void);
extern uint32_t sched_int_disable(void); extern void sched_int_restore(uint32_t);
extern uint32_t sched_ms_to_ticks(uint32_t ms);
#define FOREVER 0xFFFFFFFFu
static uint64_t deadline(uint32_t ticks){ return (ticks==FOREVER)?0:(sched_now()+ticks); }
/* ---- semaphore ---- */
typedef struct { volatile int count; volatile int max; } sem_t;
void* sem_create(uint32_t max, uint32_t init){ sem_t* s=amc_malloc(sizeof *s); s->count=init; s->max=max; return s; }
void  sem_delete(void* h){ amc_free(h); }
int   sem_take(void* h, uint32_t ticks){ sem_t* s=h; uint64_t dl=deadline(ticks);
  for(;;){ uint32_t l=sched_int_disable(); if(s->count>0){ s->count--; sched_int_restore(l); return 1; } sched_int_restore(l);
    if(!sched_block(s, dl)) return 0; } }
int   sem_give(void* h){ sem_t* s=h; uint32_t l=sched_int_disable(); if(s->count<s->max) s->count++; sched_int_restore(l); sched_wake(s); return 1; }
/* ---- queue ---- */
typedef struct { uint8_t* buf; uint32_t isz, len, head, tail; volatile uint32_t cnt; } queue_t;
void* queue_create(uint32_t len, uint32_t isz){ queue_t* q=amc_malloc(sizeof *q); q->buf=amc_malloc(len*isz); q->isz=isz; q->len=len; q->head=q->tail=q->cnt=0; return q; }
static void cp(uint8_t* d, const uint8_t* s, uint32_t n){ while(n--) *d++=*s++; }
int  queue_send(void* h, const void* item, uint32_t ticks){ queue_t* q=h; uint64_t dl=deadline(ticks);
  for(;;){ uint32_t l=sched_int_disable(); if(q->cnt<q->len){ cp(q->buf+q->tail*q->isz, item, q->isz); q->tail=(q->tail+1)%q->len; q->cnt++; sched_int_restore(l); sched_wake(q); return 1; } sched_int_restore(l);
    if(!sched_block(q, dl)) return 0; } }
int  queue_recv(void* h, void* out, uint32_t ticks){ queue_t* q=h; uint64_t dl=deadline(ticks);
  for(;;){ uint32_t l=sched_int_disable(); if(q->cnt>0){ cp(out, q->buf+q->head*q->isz, q->isz); q->head=(q->head+1)%q->len; q->cnt--; sched_int_restore(l); sched_wake(q); return 1; } sched_int_restore(l);
    if(!sched_block(q, dl)) return 0; } }
uint32_t queue_waiting(void* h){ return ((queue_t*)h)->cnt; }
/* ---- recursive mutex (binary, no owner-tracking needed for cooperative) ---- */
void* mutex_create(void){ return sem_create(1,1); }
int   mutex_lock(void* h){ return sem_take(h, FOREVER); }
int   mutex_unlock(void* h){ return sem_give(h); }
/* ---- event group ---- */
typedef struct { volatile uint32_t bits; } eg_t;
void* eg_create(void){ eg_t* e=amc_malloc(sizeof *e); e->bits=0; return e; }
uint32_t eg_set(void* h, uint32_t b){ eg_t* e=h; uint32_t l=sched_int_disable(); e->bits|=b; uint32_t v=e->bits; sched_int_restore(l); sched_wake(e); return v; }
uint32_t eg_clear(void* h, uint32_t b){ eg_t* e=h; uint32_t l=sched_int_disable(); e->bits&=~b; uint32_t v=e->bits; sched_int_restore(l); return v; }
uint32_t eg_wait(void* h, uint32_t want, uint32_t ticks){ eg_t* e=h; uint64_t dl=deadline(ticks);
  for(;;){ uint32_t l=sched_int_disable(); if((e->bits&want)==want){ uint32_t v=e->bits; sched_int_restore(l); return v; } sched_int_restore(l);
    if(!sched_block(e, dl)) return e->bits; } }
