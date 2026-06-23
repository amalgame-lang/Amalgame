#include <stdint.h>
extern void task_create(void(*)(void*), void*, void*, uint32_t);
extern void sched_start(void);
extern void sched_yield(void);
extern int uart_tx_one_char(unsigned char);
static uint8_t s1[4096], s2[4096];
static void taskA(void* a){ (void)a; for(int i=0;i<8;i++){ uart_tx_one_char('A'); sched_yield(); } uart_tx_one_char('\r'); uart_tx_one_char('\n'); for(;;) sched_yield(); }
static void taskB(void* a){ (void)a; for(int i=0;i<8;i++){ uart_tx_one_char('B'); sched_yield(); } for(;;) sched_yield(); }
int amc_main(void){
  uart_tx_one_char('S'); uart_tx_one_char(':');
  task_create(taskA, 0, s1, sizeof s1);
  task_create(taskB, 0, s2, sizeof s2);
  sched_start();
  for(;;){}
}
