ENTRY(_xipmain)
MEMORY {
  irom (RX) : ORIGIN = 0x42000000, LENGTH = 0x10000   /* entry 0 -> flash page 8 */
  drom (R)  : ORIGIN = 0x3C010000, LENGTH = 0x10000   /* entry 1 -> flash page 9 */
}
SECTIONS {
  .text   0x42000000 : AT(0x00000) { *(.text .text.*) } > irom
  .rodata 0x3C010000 : AT(0x10000) { *(.rodata .rodata.*) } > drom
}
