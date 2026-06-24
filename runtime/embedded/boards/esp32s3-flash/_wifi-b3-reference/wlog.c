/* wlog.c — minimal printf to UART for the WiFi blobs' log path (B3 debug).
 * Supports %c %s %d %i %u %x %X %p %% with optional width/zero-pad. Enough to
 * surface the blobs' diagnostics (which explain INVALID_ARG etc.). */
#include <stdint.h>
#include <stdarg.h>
extern int uart_tx_one_char(unsigned char c);
static void oc(char c){ if(c=='\n') uart_tx_one_char('\r'); uart_tx_one_char((unsigned char)c); }
static void os(const char*s){ if(!s)s="(null)"; while(*s) oc(*s++); }
static void onum(uint32_t v,uint32_t base,int up,int sgn,int width,char pad){
  char b[32]; int n=0; int neg=0; if(sgn && (int32_t)v<0){neg=1; v=(uint32_t)(-(int32_t)v);}
  if(!v) b[n++]='0';
  while(v){ uint32_t d=v%base; b[n++]= d<10 ? '0'+d : (up?'A':'a')+d-10; v/=base; }
  int len=n+(neg?1:0); for(int i=len;i<width;i++) oc(pad);
  if(neg) oc('-'); while(n--) oc(b[n]);
}
int wlog_vprintf(const char*f, va_list ap){
  for(;*f;f++){
    if(*f!='%'){ oc(*f); continue; }
    f++; char pad=' '; int width=0;
    if(*f=='0'){ pad='0'; f++; }
    while(*f>='0'&&*f<='9'){ width=width*10+(*f-'0'); f++; }
    if(*f=='l') f++;          /* ignore length mods */
    if(*f=='l') f++;
    if(*f=='z'||*f=='h') f++;
    switch(*f){
      case 'c': oc((char)va_arg(ap,int)); break;
      case 's': os(va_arg(ap,const char*)); break;
      case 'd': case 'i': onum((uint32_t)va_arg(ap,int),10,0,1,width,pad); break;
      case 'u': onum(va_arg(ap,uint32_t),10,0,0,width,pad); break;
      case 'x': onum(va_arg(ap,uint32_t),16,0,0,width,pad); break;
      case 'X': onum(va_arg(ap,uint32_t),16,1,0,width,pad); break;
      case 'p': os("0x"); onum(va_arg(ap,uint32_t),16,0,0,8,'0'); break;
      case '%': oc('%'); break;
      default: oc('%'); if(*f) oc(*f); break;
    }
  }
  return 0;
}
int wlog_printf(const char*f,...){ va_list a; va_start(a,f); wlog_vprintf(f,a); va_end(a); return 0; }
