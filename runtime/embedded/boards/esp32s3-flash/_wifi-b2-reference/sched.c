#include <stdint.h>
extern int  setjmp(void* buf);          /* ROM newlib */
extern void longjmp(void* buf, int v);  /* ROM newlib */
extern void _sched_bootstrap(void* sp_top, void(*tramp)(void*), void* t);
typedef struct { void(*entry)(void*); void* arg; void* sp_top; int started; int alive; uint32_t ctx[64]; } task_t;
#define MAXT 6
static task_t tasks[MAXT]; static int ntask; static int cur;  /* bss; cur set to -1 in sched_start */
void sched_yield(void);
static void trampoline(void* tv){ task_t* t=(task_t*)tv; t->entry(t->arg); t->alive=0; for(;;) sched_yield(); }
void task_create(void(*entry)(void*), void* arg, void* stack, uint32_t size){
  task_t* t=&tasks[ntask++]; t->entry=entry; t->arg=arg; t->started=0; t->alive=1;
  t->sp_top=(void*)(((uintptr_t)stack + size) & ~15u);
}
void sched_yield(void){
  if (cur>=0 && setjmp(tasks[cur].ctx)) return;   /* later resumed here */
  int start=(cur<0)?(ntask-1):cur, n=-1;
  for(int i=1;i<=ntask;i++){ int k=(start+i)%ntask; if(tasks[k].alive){ n=k; break; } }
  if(n<0){ for(;;){} }            /* no runnable task */
  cur=n; task_t* t=&tasks[n];
  if(!t->started){ t->started=1; _sched_bootstrap(t->sp_top, trampoline, t); }
  else longjmp(t->ctx, 1);
}
void sched_start(void){ cur=-1; sched_yield(); for(;;){} }
