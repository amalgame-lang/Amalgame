ENTRY(_start)
uart_tx_one_char  = 0x40000648;
uart_tx_wait_idle = 0x4000069c;
MEMORY {
  iram_flash (RX) : ORIGIN = 0x42000000, LENGTH = 0x10000   /* entry0 -> flash page 8: .text+.literal */
  drom       (R)  : ORIGIN = 0x3C010000, LENGTH = 0x10000   /* entry1 -> flash page 9: .rodata        */
  dram       (RW) : ORIGIN = 0x3FC88000, LENGTH = 0x50000   /* SRAM: .bss + stack                     */
}
SECTIONS {
  .text 0x42000000 : AT(0x00000) {
    KEEP(*(.text.start))
    *(.literal .text)
    *(.literal.* .text.*)
  } > iram_flash
  .rodata 0x3C010000 : AT(0x10000) {
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)
  } > drom
  .bss (NOLOAD) : ALIGN(16) {
    _sbss = . ;
    *(.bss .bss.*)
    *(.sbss .sbss.*)
    *(COMMON)
    . = ALIGN(16);
    _ebss = . ;
  } > dram
  _stack_top = ORIGIN(dram) + LENGTH(dram);
}
