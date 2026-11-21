ENTRY(_start)
uart_tx_one_char  = 0x40000648;
uart_tx_wait_idle = 0x4000069c;

/* Flash-XIP layout with MULTI-PAGE mapping (B3): the WiFi blobs make .text ~345 KB
 * and add ~15 KB .data, so one 64 KB MMU page each is no longer enough. The stub
 * maps 8 IROM pages (entries 0..7) for .text and 8 DROM pages (entries 8..15) for
 * .rodata + .data initialisers. Unified MMU: entry = (vaddr & 0x1FFFFFF) >> 16, so
 * IROM lives at 0x42000000.. (entries 0..7) and DROM at 0x3C080000.. (entry 8..).
 * Flash (blob @ 0x80000): .text at blob offset 0 (pages 8..15); .rodata+.data-init
 * +vectors at blob offset 0x80000 (pages 16..23). .data + the vector table run in
 * DRAM SRAM (crt0 copies them from their DROM-readable flash LMA).
 *
 * Interrupt vectors must live in INTERNAL SRAM, not flash: a window over/underflow
 * exception can fire on any windowed call and VECBASE-in-flash faults (and flash is
 * also dead during cache-disable windows). We reserve the first 4 KB of the
 * dual-ported DIRAM for them — DRAM view 0x3FC88000 (crt0 copy dst) aliases
 * instruction view 0x40378000 (VECBASE). SOC_I_D_OFFSET = 0x6F0000. The level-1
 * ISR (.iram.text) is copied alongside so the dispatch's PC-relative call4 reaches
 * it; -mtext-section-literals keeps its constants inline-and-reachable. */
MEMORY {
  iram_flash (RX) : ORIGIN = 0x42000000, LENGTH = 0x80000   /* IROM: 8 pages, .text/.literal     */
  drom       (R)  : ORIGIN = 0x3C080000, LENGTH = 0x80000   /* DROM: entry 8.., .rodata+.data+vec */
  vec_iram   (RWX): ORIGIN = 0x40378000, LENGTH = 0x1000    /* DIRAM I-view: vectors + level1 ISR */
  dram       (RW) : ORIGIN = 0x3FC89000, LENGTH = 0x4F000   /* SRAM (after vec): .data/.bss/heap  */
}
SECTIONS {
  .text 0x42000000 : AT(0x00000) {
    KEEP(*(.text.start))            /* _start must sit at 0x42000000 (stub jx target) */
    *(.literal .text)
    *(.literal.* .text.*)
    /* WiFi blobs mark their hot paths IRAM_ATTR (RX/TX/sleep). In IDF that is
     * because the cache is disabled during flash writes; we never write flash
     * while WiFi runs, so they execute fine from flash IROM via the I-cache.
     * Fold all the blob IRAM sections (+ their literal pools) in here. */
    *(.iram1 .iram1.*)
    *(.wifi0iram .wifi0iram.*)
    *(.wifirxiram .wifirxiram.*)
    *(.wifiextrairam .wifiextrairam.*)
    *(.wifislpiram .wifislpiram.*)
    *(.wifislprxiram .wifislprxiram.*)
    *(.wifiorslpiram .wifiorslpiram.*)
  } > iram_flash

  .rodata 0x3C080000 : AT(0x80000) {  /* blob offset 0x80000 -> flash page 16 -> DROM entry 8 */
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)
    *(.rodata_wlog_*)               /* blob WiFi-log strings (.rodata_wlog_error/info/...) */
    *(.sdata2 .sdata2.* .srodata2 .srodata2.*)
    . = ALIGN(16);
  } > drom

  /* Initialised data: runs in SRAM (VMA dram); image bytes live in flash right
   * after .rodata so objcopy stays compact (LMA = small flash offset), while the
   * crt0 reads them through DROM (_data_lma = the DROM vaddr after .rodata). */
  .data : AT(LOADADDR(.rodata) + SIZEOF(.rodata)) {
    _data_vma = ABSOLUTE(.);
    *(.data .data.*)
    *(.sdata .sdata.*)
    *(.dram1 .dram1.*)              /* blob initialised DRAM data */
    . = ALIGN(16);
    _data_end = ABSOLUTE(.);
  } > dram
  _data_lma = ADDR(.rodata) + SIZEOF(.rodata);   /* DROM vaddr crt0 copies from */

  /* Vector table + level-1 ISR: VMA = DIRAM instruction view (VECBASE target),
   * image in flash right after .data, copied by crt0 to the DRAM-view alias. */
  .vectors 0x40378000 : AT(LOADADDR(.rodata) + SIZEOF(.rodata) + SIZEOF(.data)) {
    _vec_vma = ABSOLUTE(.);
    KEEP(*(.exc_vectors.text))
    *(.iram.literal .iram.text .iram.text.*)
    . = ALIGN(16);
    _vec_end = ABSOLUTE(.);
  } > vec_iram
  _vec_lma      = ADDR(.rodata) + SIZEOF(.rodata) + SIZEOF(.data);  /* DROM vaddr to copy from   */
  _vec_dram_dst = 0x3FC88000;                                       /* DRAM-view alias (copy dst) */
  _vec_size     = _vec_end - _vec_vma;

  .bss (NOLOAD) : ALIGN(16) {
    _sbss = . ;
    *(.bss .bss.*)
    *(.sbss .sbss.*)
    *(COMMON)
    . = ALIGN(16);
    _ebss = . ;
  } > dram

  . = ALIGN(16);
  _heap_start = . ;
  _heap_end   = ORIGIN(dram) + LENGTH(dram) - 0x10000 ;
  _stack_top  = ORIGIN(dram) + LENGTH(dram);
}
