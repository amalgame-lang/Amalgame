/* Linker script — ESP32-S3 flash-XIP image (B0c), booted by a reused ESP-IDF
 * 2nd-stage bootloader (no FreeRTOS in it; it only sets up the flash cache/MMU
 * and loads our app). Unlike the esp32s3-ramload board, code and constants live
 * in flash and execute/read through the cache (XIP) — this is what gives room
 * for the WiFi blobs + lwIP later.
 *
 * Flash layout (matches the ESPHome esp32s3 bootloader we reuse):
 *   0x0000   bootloader.bin
 *   0x8000   partition table (factory app @ 0x10000)
 *   0x10000  this app image (esptool elf2image)
 *
 * Memory regions (from ESP-IDF components/esp_system/ld/esp32s3/memory.ld.in):
 *   IROM (flash .text)    @ 0x42000020   — cache-mapped instructions
 *   DROM (flash .rodata)  @ 0x3C000020   — cache-mapped constants
 *   DRAM (SRAM .data/.bss) @ 0x3FC88000  — internal SRAM, loaded by bootloader
 * The 0x20 offset on the flash regions satisfies the MMU's
 * (paddr % 64KB == vaddr % 64KB) rule once esptool adds the image/segment
 * headers; esptool elf2image classifies segments as flash-mapped by their
 * 0x42.. / 0x3C.. load address and pads the file accordingly.
 *
 * We run on the stack the bootloader leaves set (high DRAM, ~0x3FCD_xxxx); our
 * DRAM data sits low (0x3FC88000+), so there is no clobber — same trick the
 * ramload board uses with the ROM stack.
 */

ENTRY(_start)

/* ROM UART helpers (esp32s3.rom.ld) — Console without re-init. */
uart_tx_one_char  = 0x40000648;
uart_tx_wait_idle = 0x4000069c;

MEMORY
{
  iram_flash (RX) : ORIGIN = 0x42000020, LENGTH = 0x400000 - 0x20  /* IROM: flash .text  */
  drom_seg   (R)  : ORIGIN = 0x3C000020, LENGTH = 0x400000 - 0x20  /* DROM: flash .rodata */
  dram_seg   (RW) : ORIGIN = 0x3FC88000, LENGTH = 0x50000          /* SRAM: .data/.bss    */
}

SECTIONS
{
  /* Constants in flash (DROM). Must be the first flash-mapped data segment. */
  .flash.rodata : ALIGN(16)
  {
    _rodata_start = ABSOLUTE(.);
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)
    *(.rodata_wrapper)
    . = ALIGN(16);
    _rodata_end = ABSOLUTE(.);
  } > drom_seg

  /* Code + literals in flash (IROM). Literal pools first so PC-relative l32r
   * (e.g. crt0's _stack_top) always finds its literal at a lower address. */
  .flash.text : ALIGN(16)
  {
    _text_start = ABSOLUTE(.);
    *(.flash.literal .flash.literal.*)
    *(.literal .text)
    *(.literal.* .text.*)
    . = ALIGN(16);
    _text_end = ABSOLUTE(.);
  } > iram_flash

  /* Initialised data in SRAM — loaded from flash by the bootloader. */
  .data : ALIGN(16)
  {
    _sdata = ABSOLUTE(.);
    *(.data .data.*)
    *(.sdata .sdata.*)
    . = ALIGN(16);
    _edata = ABSOLUTE(.);
  } > dram_seg

  /* NOBITS — not in the image; startup.c zeroes it. */
  .bss (NOLOAD) : ALIGN(16)
  {
    _sbss = ABSOLUTE(.);
    *(.bss .bss.*)
    *(.sbss .sbss.*)
    *(COMMON)
    . = ALIGN(16);
    _ebss = ABSOLUTE(.);
  } > dram_seg

  /* Stack grows down from the top of the DRAM bank (crt0.S sets SP here, since
   * the bootloader's hand-off SP is not reliable). .data/.bss live at the
   * bottom of the same bank — 320 KB apart, no collision. */
  _stack_top = ORIGIN(dram_seg) + LENGTH(dram_seg);
}
