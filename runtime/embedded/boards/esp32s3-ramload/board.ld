/* Linker script — ESP32-S3 RAM-loaded image (bundled board for
 * `amc --target=esp32s3`).
 *
 * The ESP32-S3 first-stage ROM loader reads the image at flash offset 0x0,
 * copies each segment into internal SRAM at its load address, and jumps to
 * the entry point with a valid ROM stack and the ROM exception vectors
 * (windowed-ABI window over/underflow handlers) active. We therefore run
 * entirely from internal SRAM — no flash XIP, no cache setup, no 2nd-stage
 * bootloader. UART0 is already configured by the ROM (it printed the boot
 * log), so the runtime's Console (ROM uart_tx_one_char) works immediately.
 *
 * Memory regions mirror the ESP-IDF 2nd-stage bootloader layout for the
 * ESP32-S3 (components/bootloader/.../esp32s3/bootloader.ld), so we land in
 * the same SRAM window the ROM keeps free and never clobber ROM .data/.bss
 * or the ROM stack (which lives above 0x3fce9700):
 *   iram_seg + iram_loader_seg  → 0x403C8700, len 0xA000   (text)
 *   dram_seg                    → 0x3FCE2700, len 0x5000   (rodata/data/bss)
 * (IRAM addr = DRAM addr + 0x6f0000; the two windows above are contiguous
 *  and non-overlapping.)
 */

ENTRY(_start)

/* ROM UART helpers — addresses from esp32s3.rom.ld. Lets the runtime's
 * Console reach UART0 without re-initialising the peripheral. */
uart_tx_one_char  = 0x40000648;
uart_tx_wait_idle = 0x4000069c;

MEMORY
{
  iram (RWX) : ORIGIN = 0x403C8700, LENGTH = 0xA000   /* instruction SRAM — .text/.literal */
  dram (RW)  : ORIGIN = 0x3FCE2700, LENGTH = 0x5000   /* data SRAM — .rodata/.data/.bss     */
  /* Free low internal SRAM for the runtime heap (B0b-3). The ESP32-S3 DRAM bank
   * is 0x3FC88000..0x3FCF0000; our image (text+data, DIRAM-aliased) sits at
   * 0x3FCD8700..0x3FCE7700, the ROM stack near 0x3FCE9700, ROM .bss/.data above
   * 0x3FCED710. So 0x3FC88000..0x3FCD8000 is free — ~320 KB. (Real sizing comes
   * with B0c flash-XIP; this is room enough to bring up malloc/free now.) */
  heap (RW)  : ORIGIN = 0x3FC88000, LENGTH = 0x50000   /* 320 KB runtime heap   */
}

SECTIONS
{
  /* Exception/interrupt vector table (B0b-2). VECBASE must be 1 KB-aligned, so
   * place this first in IRAM on a 0x400 boundary; interrupts.c points VECBASE
   * at _amc_vecbase. The .org offsets inside vectors.S lay out the fixed Xtensa
   * vector slots relative to that base. */
  .exc_vectors : ALIGN(0x400)
  {
    KEEP(*(.exc_vectors.text))
  } > iram

  /* Xtensa literals must sit with the code. */
  .text : ALIGN(4)
  {
    *(.literal .text)
    *(.literal.* .text.*)
  } > iram

  .rodata : ALIGN(4)
  {
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)
  } > dram

  .data : ALIGN(4)
  {
    _sdata = .;
    *(.data .data.*)
    *(.sdata .sdata.*)
    _edata = .;
  } > dram

  /* NOBITS — not loaded by the ROM; startup.c zeroes it. */
  .bss (NOLOAD) : ALIGN(16)
  {
    _sbss = .;
    *(.bss .bss.*)
    *(.sbss .sbss.*)
    *(COMMON)
    . = ALIGN(16);
    _ebss = .;
  } > dram

  /* Runtime heap (B0b-3) — own free SRAM bank, not loaded, not zeroed by
   * startup (the allocator initialises its own free list lazily). */
  .heap (NOLOAD) : ALIGN(16)
  {
    _heap_start = .;
    . = . + LENGTH(heap);
    _heap_end = .;
  } > heap
}
