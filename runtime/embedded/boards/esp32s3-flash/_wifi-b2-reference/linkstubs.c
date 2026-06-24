#include <stdint.h>
#include <stddef.h>
/* mesh / espnow / mt — not used in STA: stubs */
int  g_mt; void* mt_get_peer_info(void){return 0;}
int  g_espnow_user_oui;
void ieee80211_init_mesh_assoc_ie(void){}
int  ieee80211_vnd_mesh_quick_get(void){return 0;} void ieee80211_vnd_mesh_quick_set(void){}
int  ieee80211_vnd_mesh_roots_get(void){return 0;} void ieee80211_vnd_mesh_roots_set(void){}
void mesh_clear_parent_candidate(void){} void* mesh_get_parent_candidate(void){return 0;}
void* mesh_get_parent_monitor_config(void){return 0;} int mesh_get_rssi_threshold(void){return 0;}
void mesh_set_ie_crypto_config(void){} void mesh_set_parent_candidate(void){}
void mesh_set_parent_monitor_config(void){} void mesh_set_rssi_threshold(void){}
/* regulatory data: zeroed tables (blob reads them; OK for first link/bring-up) */
uint8_t regdomain_table[512]; uint8_t regulatory_data[512];
/* event base symbol the blobs reference */
const char* WIFI_EVENT = "WIFI_EVENT";
/* debug printf -> UART (B3 bring-up: surface the blobs' diagnostics) */
#include <stdarg.h>
extern int wlog_vprintf(const char*,va_list);
int pp_printf(const char*f,...){ va_list a; va_start(a,f); wlog_vprintf(f,a); va_end(a); return 0; }
int net80211_printf(const char*f,...){ va_list a; va_start(a,f); wlog_vprintf(f,a); va_end(a); return 0; }
int puts(const char*s){(void)s;return 0;}
int hexstr2bin(const char*hex,uint8_t*buf,size_t len){(void)hex;(void)buf;(void)len;return 0;}
