#include <stdint.h>
extern int  setjmp(void* buf);
extern void longjmp(void* buf, int v);
extern void _sched_bootstrap(void* sp_top, void(*tramp)(void*), void* t);
enum { ST_READY=0, ST_BLOCKED=1, ST_DEAD=2 };
typedef struct { void(*entry)(void*); void* arg; void* sp_top; int started; int state;
                 void* blocked_on; uint64_t deadline; int woke_ok; uint32_t ctx[64]; } task_t;
#define MAXT 8
static task_t tasks[MAXT]; static int ntask; static int cur;
void sched_yield(void);
extern void amc_timers_check(void);   /* timers.c — fire expired ETS software timers */
/* systimer (16 MHz) read */
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
uint64_t sched_now(void){ REG32(0x60023004)=(1u<<30); while(!(REG32(0x60023004)&(1u<<29))){} return ((uint64_t)REG32(0x60023040)<<32)|REG32(0x60023044); }
/* critical section = raise INTLEVEL */
uint32_t sched_int_disable(void){ uint32_t ps; __asm__ volatile("rsil %0,3":"=r"(ps)); return ps; }
void     sched_int_restore(uint32_t ps){ __asm__ volatile("wsr.ps %0\n rsync\n"::"r"(ps):"memory"); }

static void trampoline(void* tv){ task_t* t=(task_t*)tv; t->entry(t->arg); t->state=ST_DEAD; for(;;) sched_yield(); }
int  sched_task_create(void(*e)(void*), void* arg, void* stk, uint32_t sz){
  if(ntask>=MAXT) return -1; task_t* t=&tasks[ntask]; t->entry=e; t->arg=arg; t->started=0;
  t->state=ST_READY; t->blocked_on=0; t->deadline=0; t->sp_top=(void*)(((uintptr_t)stk+sz)&~15u); return ntask++;
}
/* tasks[] is shared between tasks (block/yield) and the WiFi MAC ISR (sched_wake
 * via _queue_send_from_isr). Every mutation runs in a critical section — without
 * it the scheduler state races and a task can be resumed with a bad context. */
/* block current task on obj until deadline (0=forever); returns woke_ok (1 event, 0 timeout) */
int sched_block(void* obj, uint64_t deadline){
  uint32_t l=sched_int_disable();
  tasks[cur].blocked_on=obj; tasks[cur].deadline=deadline; tasks[cur].woke_ok=0; tasks[cur].state=ST_BLOCKED;
  sched_int_restore(l);
  sched_yield(); return tasks[cur].woke_ok;
}
/* wake (at most one) task blocked on obj; returns 1 if woke someone. Called from ISRs. */
int sched_wake(void* obj){
  uint32_t l=sched_int_disable(); int r=0;
  for(int i=0;i<ntask;i++) if(tasks[i].state==ST_BLOCKED && tasks[i].blocked_on==obj){
    tasks[i].state=ST_READY; tasks[i].woke_ok=1; tasks[i].blocked_on=0; tasks[i].deadline=0; r=1; break; }
  sched_int_restore(l); return r;
}
static void check_timeouts(void){
  uint32_t l=sched_int_disable();
  uint64_t now=sched_now();
  for(int i=0;i<ntask;i++) if(tasks[i].state==ST_BLOCKED && tasks[i].deadline && now>=tasks[i].deadline){
    tasks[i].state=ST_READY; tasks[i].woke_ok=0; tasks[i].blocked_on=0; tasks[i].deadline=0; }
  sched_int_restore(l);
}
void sched_yield(void){
  if(cur>=0 && setjmp(tasks[cur].ctx)) return;
  int start=(cur<0)?(ntask-1):cur;
  for(;;){
    check_timeouts();
    amc_timers_check();   /* fire any expired ETS software timers (B3, timers.c) */
    int n=-1; for(int i=1;i<=ntask;i++){ int k=(start+i)%ntask; if(tasks[k].state==ST_READY){ n=k; break; } }
    if(n>=0){ cur=n; task_t* t=&tasks[n];
      if(!t->started){ t->started=1; _sched_bootstrap(t->sp_top, trampoline, t); }
      else longjmp(t->ctx,1); return; }
    /* nothing ready: idle until a timeout or ISR wakes someone */
    __asm__ volatile("waiti 0");
  }
}
void sched_start(void){ cur=-1; sched_yield(); for(;;){} }
uint32_t sched_ms_to_ticks(uint32_t ms){ return ms*16000u; }   /* 16 MHz systimer */
void* sched_self(void){ return &tasks[cur]; }
void  sched_delay_ticks(uint32_t t){ sched_block((void*)0, sched_now()+t); }
