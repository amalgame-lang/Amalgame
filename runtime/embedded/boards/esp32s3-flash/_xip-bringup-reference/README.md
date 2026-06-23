# esp32s3 flash-XIP bring-up — WORKING reference (B0c)

Self-contained flash-XIP, **no 2nd-stage bootloader, no ESPHome, no FreeRTOS** —
and **observable** (keeps the ROM UART at 115200). Validated on an
ESP32-S3-DevKitM: the XIP code runs from flash via the I-cache and streams "OK"
over the ROM UART.

## How it works
Two flash regions, ROM-loaded stub does the cache/MMU bring-up:
- `0x0`      — **stub** (`stub.c`/`stub.ld`): a normal RAM-load image (ROM loads
  it to IRAM/DRAM and jumps to `_start`; ROM UART stays usable).
- `0x80000`  — **XIP blob** (`xip.S`/`xipblob.ld`): raw binary, `.text` linked at
  `0x42000000`. Pure I-bus code (no literals / no data loads from flash) so it
  needs only the IROM mapping for this first proof.

Build + flash:
```
xtensa-esp32s3-elf-gcc -mlongcalls -nostartfiles -nostdlib -T xipblob.ld xip.S -o xipblob.elf
xtensa-esp32s3-elf-objcopy -O binary xipblob.elf xipblob.bin
xtensa-esp32s3-elf-gcc -mlongcalls -Os -ffreestanding -nostartfiles -Wl,--gc-sections -T stub.ld stub.c -o stub.elf
esptool --chip esp32s3 elf2image --flash-size 4MB -o stub.bin stub.elf
esptool --chip esp32s3 -p /dev/ttyUSB0 write_flash 0x0 stub.bin 0x80000 xipblob.bin
```

## The winning sequence (what the stub does)
The crux took real on-silicon debugging. Key discovery (via a DBUS data read of
the mapped page): after a RAM-load boot the ROM **leaves the I-cache disabled**
(`EXTMEM_ICACHE_CTRL` @ `0x600C4060` == 0) while the D-cache is on — so flash
*data* reads work but instruction fetches from `0x42000000` give
`IllegalInstruction`. The fix is to configure + enable the I-cache ourselves:

```c
rom_config_instruction_cache_mode(16384, 8, 32);   // ROM 0x40001a1c — set I-cache size/ways/line
                                                   // + allocate its SRAM banks (ROM left it off)
*(uint32_t*)(0x600C5000 + entry*4) = page;         // MMU table: entry = (vaddr & 0x1FFFFFF)>>16,
                                                   //   page = paddr>>16, value = page|VALID(0)|FLASH(0)
*(uint32_t*)0x600C4064 &= ~1u;                     // EXTMEM_ICACHE_CTRL1: clear ICACHE_SHUT_CORE0_BUS
Cache_Invalidate_ICache_All();                     // ROM 0x400016d4
Cache_Enable_ICache(0);                            // ROM 0x40001878
// jump:  __asm__("jx %0" :: "r"(0x42000000));
```
For `vaddr=0x42000000, paddr(flash)=0x80000` → entry 0, page 8.

ROM function addresses (from `esp_rom/esp32s3/ld/esp32s3.rom.ld`): see `stub.ld`.
MMU: `DR_REG_MMU_TABLE=0x600C5000`, `SOC_MMU_VADDR_MASK=0x1FFFFFF`, page size
64 KB, `SOC_MMU_VALID=0`, `SOC_MMU_ACCESS_FLASH=0`, `SOC_MMU_INVALID=BIT(14)`.

## Next (turn this into the real board)
- Map a **DROM** entry too (for `.rodata`): same scheme at vaddr `0x3C000000`
  (entry 0 also serves the DBUS — use a vaddr offset for a distinct entry when
  IROM and DROM live at different flash pages). D-cache is already on.
- Replace this reference with `crt0.S`/`startup.c`/`board.ld` that put the *real*
  amc runtime in the XIP blob, fold back B0a/B0b-1/2/3, and wire into the amc
  target selection. Then merge.

## ✅ Update — IROM + DROM both work (`xip2.S`/`xip2.ld`)

`xip2.S`/`xip2.ld` extend the proof to **both flash buses**: `.text`→IROM (entry 0,
`0x42000000`) and a `.rodata` string→**DROM** (entry 1, `0x3C010000`). The XIP code
reads the string from DROM and prints it — confirmed on silicon
("hello from flash XIP (.text=IROM, this string=DROM) ok" streams). `stub.c` here
is the IROM+DROM version (maps `MMU[0]=8` for .text@flash-page-8 and `MMU[1]=9`
for .rodata@flash-page-9; D-cache was already enabled by the ROM).

**MMU entry collision gotcha:** IROM `0x42000000` and DROM `0x3C000000` both yield
entry 0 (`(vaddr & 0x1FFFFFF)>>16`). To map .text and .rodata to *different* flash
pages, give them distinct entries via vaddr offset — here DROM lives at
`0x3C010000` (entry 1). Real apps (and the IDF) do the same with per-segment vaddr
offsets.

So the full cache/MMU foundation is done. The remaining B0c work is pure
integration: a payload linker that lays the real amc runtime as
`.text`→IROM / `.rodata`→DROM / `.data`+`.bss`→DRAM-SRAM (with `.data` init copied
from its flash LMA by the payload crt0), the stub mapping enough IROM/DROM pages,
fold B0a/b1/b2/b3, wire into amc, merge.
