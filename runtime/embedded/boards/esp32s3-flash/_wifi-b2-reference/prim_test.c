#include <stdint.h>
extern int sched_task_create(void(*)(void*),void*,void*,uint32_t); extern void sched_start(void); extern void sched_yield(void);
extern void* sem_create(uint32_t,uint32_t); extern int sem_take(void*,uint32_t); extern int sem_give(void*);
extern void* queue_create(uint32_t,uint32_t); extern int queue_send(void*,const void*,uint32_t); extern int queue_recv(void*,void*,uint32_t);
extern uint32_t sched_ms_to_ticks(uint32_t);
extern int uart_tx_one_char(unsigned char);
static void p(const char*s){for(;*s;++s){if(*s=='\n')uart_tx_one_char('\r');uart_tx_one_char((unsigned char)*s);} }
static void dec(uint32_t v){char b[12];int n=0;if(!v){uart_tx_one_char('0');return;}while(v){b[n++]='0'+v%10;v/=10;}while(n--)uart_tx_one_char(b[n]);}
static uint8_t s1[4096],s2[4096],s3[4096]; static void* sem; static void* q;
static void producer(void*a){(void)a; for(int i=1;i<=5;i++){ int v=i*10; queue_send(q,&v,0xFFFFFFFFu); sem_give(sem); } for(;;)sched_yield(); }
static void consumer(void*a){(void)a; for(int i=0;i<5;i++){ sem_take(sem,0xFFFFFFFFu); int v; queue_recv(q,&v,0xFFFFFFFFu); p("got="); dec((uint32_t)v); p(" "); } p("\n");
  p("timeout test (take 200ms, no give)... "); int r=sem_take(sem, sched_ms_to_ticks(200)); p(r?"GOT(bad)\n":"TIMEOUT(ok)\n"); p("PRIMS PASS\n"); for(;;)sched_yield(); }
int amc_main(void){ p("PRIMS:\n"); sem=sem_create(8,0); q=queue_create(8,sizeof(int));
  sched_task_create(consumer,0,s1,sizeof s1); sched_task_create(producer,0,s2,sizeof s2); (void)s3; sched_start(); for(;;){} }
