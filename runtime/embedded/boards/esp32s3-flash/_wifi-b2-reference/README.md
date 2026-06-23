# B2 (WiFi osi_funcs) — cooperative scheduler core ✅ on silicon

The core of Route B's OS adapter: a **cooperative scheduler** that the WiFi blobs'
`wifi_osi_funcs_t` (122 fn ptrs) will sit on. The hardest conceptual piece —
Xtensa context switching — is solved with the **ROM `setjmp`/`longjmp`** (they
handle the windowed-ABI window spill/restore) + a tiny `_sched_bootstrap` asm to
start a task on its own stack. Validated on silicon (flash XIP): two tasks
alternate (`S:ABABABAB…`).

## Files
- **`sched.c`** — `task_create` / `sched_start` / `sched_yield`. Round-robin over
  alive tasks. `sched_yield` = `setjmp(cur.ctx)` then pick next + (`_sched_bootstrap`
  if unstarted, else `longjmp(next.ctx,1)`). `jmp_buf` ≈ `uint32_t ctx[64]`.
- **`sched_boot.S`** — `_sched_bootstrap(sp_top, trampoline, task)`: `mov a1,sp_top`
  (switch SP to the task stack) then `callx4 trampoline(task)`. The trampoline
  runs `entry(arg)` and yields forever when it returns.

Link with `-T esp32s3.rom.libc.ld` (for `setjmp`/`longjmp` @ 0x4000144c/0x40001440).

⚠️ **`.data` note:** the flash board.ld has no `.data` output section (amc
programs have empty `.data`). Initialised-non-zero statics (e.g. `static int
cur=-1`) become orphan `.data` → a giant `objcopy -O binary`. Keep statics in
`.bss` (init at runtime) **or** add a `.data` section (LMA in DROM, copied by
crt0) — the latter is needed for B2 proper (the blobs/`g_wifi_osi_funcs` setup).

## Remaining for B2 (multi-session)
1. **Blocking primitives** on this core: semaphores (`take`/`give` w/ timeout),
   recursive mutexes, queues (`send`/`recv` + `*_from_isr`), event-groups
   (`set`/`clear`/`wait_bits`), `task_delay` — block = mark waiting + `sched_yield`;
   wake = mark ready (timeouts off the systimer tick; ISR variants defer-wake).
2. **`g_wifi_osi_funcs`** (122 ptrs, v0.8 magic 0xDEADBEAF) — map to the scheduler
   + trivial ones (malloc→`amc_malloc`, log, clock, random, nvs stubs). Ref:
   `esp_wifi/esp32s3/esp_adapter.c`.
3. **Link** `libpp`+`libnet80211`+`libcore` + the 25 stubs (mesh/espnow/regdomain/
   WIFI_EVENT/debug) into the XIP payload (needs a real `.data` section + likely
   more IROM/DROM pages — the MAC blobs are big).
4. Call **`esp_wifi_init`** → first B2 milestone. Then B3 = `esp_wifi_start`/
   `connect` (STA, compile-time creds).
