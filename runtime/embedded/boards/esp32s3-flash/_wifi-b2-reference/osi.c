#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
/* scheduler + prims (ours) */
extern int  sched_task_create(void(*)(void*),void*,void*,uint32_t);
extern void sched_yield(void); extern void* sched_self(void);
extern int  sched_block(void*,uint64_t); extern uint64_t sched_now(void);
extern void sched_delay_ticks(uint32_t); extern uint32_t sched_ms_to_ticks(uint32_t);
extern uint32_t sched_int_disable(void); extern void sched_int_restore(uint32_t);
extern void* sem_create(uint32_t,uint32_t); extern void sem_delete(void*); extern int sem_take(void*,uint32_t); extern int sem_give(void*);
extern void* queue_create(uint32_t,uint32_t); extern int queue_send(void*,const void*,uint32_t); extern int queue_recv(void*,void*,uint32_t); extern uint32_t queue_waiting(void*);
extern void* mutex_create(void); extern int mutex_lock(void*); extern int mutex_unlock(void*);
extern void* eg_create(void); extern uint32_t eg_set(void*,uint32_t); extern uint32_t eg_clear(void*,uint32_t); extern uint32_t eg_wait(void*,uint32_t,uint32_t);
extern void* amc_malloc(size_t); extern void amc_free(void*); extern void* amc_calloc(size_t,size_t); extern uint32_t amc_heap_free(void);
/* interrupts (ours, interrupts.c): generic level-1 dispatch table (B3). */
extern void amc_set_isr(int n, void *fn, void *arg);
extern void amc_route_intr(uint32_t source, uint32_t num);
/* osi struct (from the IDF header — re-declared minimally to avoid include chain) */
#include "wifi_osi_struct.h"
/* ---- impls ---- */
static int   m1(void){return 1;}
static bool  env_is_chip(void){return 1;}
/* MAC ISR wiring (B3): route the matrix + register the blob's handler on our
 * generic level-1 dispatch. set_intr(cpu_no,source,num,prio) ignores cpu_no
 * (single core) and prio (all level-1); set_isr(n,fn,arg) registers by CPU int. */
static void  set_intr_w(int32_t cpu_no, uint32_t source, uint32_t num, int32_t prio){
    (void)cpu_no; (void)prio; amc_route_intr(source, num);
}
static void  set_isr_w(int32_t n, void *fn, void *arg){ amc_set_isr((int)n, fn, arg); }
static void  v_u32(uint32_t x){(void)x;}
static void  noop(void){}
static bool  is_from_isr(void){ uint32_t ps; __asm__ volatile("rsr.ps %0":"=r"(ps)); return ((ps>>0)&0xF)!=0; } /* INTLEVEL!=0 ~ in isr/crit */
static void* spin_create(void){ static int d; return &d; }
static void  spin_del(void*l){(void)l;}
static uint32_t wint_dis(void*mux){(void)mux; return sched_int_disable();}
static void  wint_res(void*mux,uint32_t t){(void)mux; sched_int_restore(t);}
static void  yield_isr(void){}
static void* thr_sem_get(void){ static void* s; if(!s) s=sem_create(1,0); return s; }
static int32_t sem_take_(void*s,uint32_t t){return sem_take(s,t);}
static int32_t sem_give_(void*s){return sem_give(s);}
static int32_t mtx_lock(void*m){return mutex_lock(m);}
static int32_t mtx_unlock(void*m){return mutex_unlock(m);}
static int32_t q_send(void*q,void*i,uint32_t t){return queue_send(q,i,t);}
static int32_t q_send_isr(void*q,void*i,void*hp){(void)hp; return queue_send(q,i,0);}
static int32_t q_recv(void*q,void*i,uint32_t t){return queue_recv(q,i,t);}
static uint32_t eg_wait_(void*e,uint32_t b,int c,int a,uint32_t t){(void)a; uint32_t r=eg_wait(e,b,t); if(c) eg_clear(e,b); return r;}
typedef void (*taskfn)(void*);
static int32_t task_create(void*fn,const char*nm,uint32_t depth,void*p,uint32_t prio,void*handle){(void)nm;(void)prio;
  void* stk=amc_malloc(depth?depth:4096); int id=sched_task_create((taskfn)fn,p,stk,depth?depth:4096);
  if(handle)*(void**)handle=(void*)(intptr_t)(id+1); return id>=0;}
static int32_t task_create_pin(void*fn,const char*nm,uint32_t d,void*p,uint32_t pr,void*h,uint32_t core){(void)core; return task_create(fn,nm,d,p,pr,h);}
static void  task_delete(void*h){(void)h;}
static void  task_delay(uint32_t t){sched_delay_ticks(t);}
static int32_t ms2tick(uint32_t ms){return sched_ms_to_ticks(ms);}
static int32_t max_prio(void){return 25;}
static void* mem(size_t s){return amc_malloc(s);}
static void* zmem(size_t s){void*p=amc_calloc(1,s);return p;}
static void* cmem(size_t n,size_t s){return amc_calloc(n,s);}
static void* rmem(void*p,size_t s){(void)p; return amc_malloc(s);} /* simplistic realloc */
static int32_t evt_post(const char*b,int32_t id,void*d,size_t n,uint32_t t){(void)b;(void)id;(void)d;(void)n;(void)t;return 0;}
static uint32_t freeheap(void){return amc_heap_free();}
static uint32_t rng(void){return REG32(0x600260B0u);} /* WDEV_RND_REG */
static int rng_buf(uint8_t*b,size_t n){for(size_t i=0;i<n;i++)b[i]=(uint8_t)REG32(0x600260B0u);return 0;}
static unsigned long rnd_ul(void){return REG32(0x600260B0u);}
/* phy (B1) */
extern int register_chipv7_phy(const void*,void*,int); extern const uint8_t phy_init_data[]; static uint8_t s_cal[1904]; static int s_phy_done;
static void phy_en(void){ if(s_phy_done)return; s_phy_done=1;
  REG32(0x60008090)&=~(1u<<17); for(volatile int i=0;i<8000;i++); REG32(0x60026014)|=0x78078Fu;
  REG32(0x60026018)|=0x2A1Fu; REG32(0x60026018)&=~0x2A1Fu; REG32(0x60008094)&=~(1u<<28); for(volatile int i=0;i<8000;i++);
  register_chipv7_phy(phy_init_data,s_cal,2); }
static void phy_dis(void){}
static int  country(const char*c){(void)c;return 0;}
static int  read_mac(uint8_t*mac,unsigned t){(void)t; /* base MAC from efuse BLK1 (ROM layout) */ uint32_t l=REG32(0x60007044),h=REG32(0x60007048);
  mac[0]=h>>8; mac[1]=h; mac[2]=l>>24; mac[3]=l>>16; mac[4]=l>>8; mac[5]=l; return 0;}
static void wifi_reset_mac(void){ REG32(0x60026018)|=0x7; REG32(0x60026018)&=~0x7; }
static void wifi_clk_en(void){ REG32(0x60026014)|=0x78078Fu; }
static void wifi_clk_dis(void){}
static void rtc_iso_en(void){} static void rtc_iso_dis(void){}
static int64_t now_us(void){return (int64_t)(sched_now()/16ull);}
/* nvs stubs: get -> not found(-1), set/open/commit -> ok(0) */
static int nvs_geti8(uint32_t h,const char*k,int8_t*o){(void)h;(void)k;(void)o;return -1;}
static int nvs_seti8(uint32_t h,const char*k,int8_t v){(void)h;(void)k;(void)v;return 0;}
static int nvs_getu8(uint32_t h,const char*k,uint8_t*o){(void)h;(void)k;(void)o;return -1;}
static int nvs_setu8(uint32_t h,const char*k,uint8_t v){(void)h;(void)k;(void)v;return 0;}
static int nvs_getu16(uint32_t h,const char*k,uint16_t*o){(void)h;(void)k;(void)o;return -1;}
static int nvs_setu16(uint32_t h,const char*k,uint16_t v){(void)h;(void)k;(void)v;return 0;}
static int nvs_open(const char*n,unsigned m,uint32_t*o){(void)n;(void)m; if(o)*o=1; return 0;}
static void nvs_close(uint32_t h){(void)h;}
static int nvs_commit(uint32_t h){(void)h;return 0;}
static int nvs_setblob(uint32_t h,const char*k,const void*v,size_t l){(void)h;(void)k;(void)v;(void)l;return 0;}
static int nvs_getblob(uint32_t h,const char*k,void*v,size_t*l){(void)h;(void)k;(void)v;(void)l;return -1;}
static int nvs_erase(uint32_t h,const char*k){(void)h;(void)k;return 0;}
static int get_time(void*t){(void)t;return 0;}
static uint32_t slowclk(void){return 28639;} /* ~ RTC slow clk cal default */
static void logw(unsigned l,const char*t,const char*f,...){(void)l;(void)t;(void)f;}
static void logwv(unsigned l,const char*t,const char*f,va_list a){(void)l;(void)t;(void)f;(void)a;}
static uint32_t logts(void){return (uint32_t)now_us()/1000u;}
/* timers: minimal software timer (stub arm/disarm for link; real impl later) */
static void tmr_arm(void*t,uint32_t ms,bool rp){(void)t;(void)ms;(void)rp;}
static void tmr_disarm(void*t){(void)t;}
static void tmr_done(void*t){(void)t;}
static void tmr_setfn(void*t,void*fn,void*arg){(void)t;(void)fn;(void)arg;}
static void tmr_arm_us(void*t,uint32_t us,bool rp){(void)t;(void)us;(void)rp;}
static void* wifi_create_q(int len,int isz){ return queue_create(len,isz); }
static void wifi_del_q(void*q){(void)q;}
/* coex: all stubs */
static int ci(void){return 0;} static void cv(void){} static uint32_t cu(void){return 0;}
static int coex_wifi_req(uint32_t a,uint32_t b,uint32_t c){(void)a;(void)b;(void)c;return 0;}
static int coex_wifi_rel(uint32_t a){(void)a;return 0;}
static int coex_chan(uint8_t a,uint8_t b){(void)a;(void)b;return 0;}
static int coex_dur(uint32_t a,uint32_t*b){(void)a; if(b)*b=0; return 0;}
static int coex_pti(uint32_t a,uint8_t*b){(void)a; if(b)*b=0; return 0;}
static void coex_bit(uint32_t a,uint32_t b){(void)a;(void)b;}
static void coex_cond(uint32_t a,bool b){(void)a;(void)b;}
static int coex_iset(uint32_t a){(void)a;return 0;} static uint32_t coex_iget(void){return 0;}
static uint8_t coex_period(void){return 0;} static void* coex_phase(void){return 0;}
static int coex_restart(void){return 0;}
static int coex_reg_cb(int a,int(*cb)(int)){(void)a;(void)cb;return 0;}
static int coex_reg_start(int(*cb)(void)){(void)cb;return 0;}
static int coex_flex_set(uint8_t a){(void)a;return 0;} static uint8_t coex_flex_get(void){return 0;}
static void* coex_phase_idx(int i){(void)i;return 0;}

wifi_osi_funcs_t g_wifi_osi_funcs = {
  ._version=ESP_WIFI_OS_ADAPTER_VERSION,
  ._env_is_chip=env_is_chip, ._set_intr=(void*)set_intr_w, ._clear_intr=(void*)noop, ._set_isr=(void*)set_isr_w,
  ._ints_on=v_u32, ._ints_off=v_u32, ._is_from_isr=(void*)is_from_isr,
  ._spin_lock_create=spin_create, ._spin_lock_delete=spin_del,
  ._wifi_int_disable=wint_dis, ._wifi_int_restore=wint_res, ._task_yield_from_isr=yield_isr,
  ._semphr_create=sem_create, ._semphr_delete=(void*)sem_delete, ._semphr_take=sem_take_, ._semphr_give=sem_give_,
  ._wifi_thread_semphr_get=thr_sem_get,
  ._mutex_create=mutex_create, ._recursive_mutex_create=mutex_create, ._mutex_delete=(void*)sem_delete,
  ._mutex_lock=mtx_lock, ._mutex_unlock=mtx_unlock,
  ._queue_create=queue_create, ._queue_delete=(void*)amc_free, ._queue_send=q_send, ._queue_send_from_isr=q_send_isr,
  ._queue_send_to_back=q_send, ._queue_send_to_front=q_send, ._queue_recv=q_recv, ._queue_msg_waiting=queue_waiting,
  ._event_group_create=eg_create, ._event_group_delete=(void*)amc_free, ._event_group_set_bits=eg_set,
  ._event_group_clear_bits=eg_clear, ._event_group_wait_bits=eg_wait_,
  ._task_create_pinned_to_core=task_create_pin, ._task_create=task_create, ._task_delete=task_delete,
  ._task_delay=task_delay, ._task_ms_to_tick=ms2tick, ._task_get_current_task=sched_self, ._task_get_max_priority=max_prio,
  ._malloc=mem, ._free=amc_free, ._event_post=evt_post, ._get_free_heap_size=freeheap, ._rand=rng,
  ._dport_access_stall_other_cpu_start_wrap=noop, ._dport_access_stall_other_cpu_end_wrap=noop,
  ._wifi_apb80m_request=noop, ._wifi_apb80m_release=noop, ._phy_disable=phy_dis, ._phy_enable=phy_en,
  ._phy_update_country_info=country, ._read_mac=read_mac,
  ._timer_arm=tmr_arm, ._timer_disarm=tmr_disarm, ._timer_done=tmr_done, ._timer_setfn=tmr_setfn, ._timer_arm_us=tmr_arm_us,
  ._wifi_reset_mac=wifi_reset_mac, ._wifi_clock_enable=wifi_clk_en, ._wifi_clock_disable=wifi_clk_dis,
  ._wifi_rtc_enable_iso=rtc_iso_en, ._wifi_rtc_disable_iso=rtc_iso_dis, ._esp_timer_get_time=now_us,
  ._nvs_set_i8=nvs_seti8, ._nvs_get_i8=nvs_geti8, ._nvs_set_u8=nvs_setu8, ._nvs_get_u8=nvs_getu8,
  ._nvs_set_u16=nvs_setu16, ._nvs_get_u16=nvs_getu16, ._nvs_open=nvs_open, ._nvs_close=nvs_close, ._nvs_commit=nvs_commit,
  ._nvs_set_blob=nvs_setblob, ._nvs_get_blob=nvs_getblob, ._nvs_erase_key=nvs_erase,
  ._get_random=rng_buf, ._get_time=get_time, ._random=rnd_ul, ._slowclk_cal_get=slowclk,
  ._log_write=logw, ._log_writev=logwv, ._log_timestamp=logts,
  ._malloc_internal=mem, ._realloc_internal=rmem, ._calloc_internal=cmem, ._zalloc_internal=zmem,
  ._wifi_malloc=mem, ._wifi_realloc=rmem, ._wifi_calloc=cmem, ._wifi_zalloc=zmem,
  ._wifi_create_queue=wifi_create_q, ._wifi_delete_queue=wifi_del_q,
  ._coex_init=ci, ._coex_deinit=cv, ._coex_enable=ci, ._coex_disable=cv, ._coex_status_get=cu,
  ._coex_condition_set=coex_cond, ._coex_wifi_request=coex_wifi_req, ._coex_wifi_release=coex_wifi_rel,
  ._coex_wifi_channel_set=coex_chan, ._coex_event_duration_get=coex_dur, ._coex_pti_get=coex_pti,
  ._coex_schm_status_bit_clear=coex_bit, ._coex_schm_status_bit_set=coex_bit, ._coex_schm_interval_set=coex_iset,
  ._coex_schm_interval_get=coex_iget, ._coex_schm_curr_period_get=coex_period, ._coex_schm_curr_phase_get=coex_phase,
  ._coex_schm_process_restart=coex_restart, ._coex_schm_register_cb=coex_reg_cb, ._coex_register_start_cb=coex_reg_start,
  ._coex_schm_flexible_period_set=coex_flex_set, ._coex_schm_flexible_period_get=coex_flex_get,
  ._coex_schm_get_phase_by_idx=coex_phase_idx,
  ._magic=ESP_WIFI_OS_ADAPTER_MAGIC,
};
wifi_osi_funcs_t* g_osi_funcs_p = &g_wifi_osi_funcs;   /* the blobs read this */
