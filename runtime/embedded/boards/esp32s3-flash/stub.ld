ENTRY(_start)
uart_tx_one_char            = 0x40000648;
uart_tx_wait_idle           = 0x4000069c;
Cache_MMU_Init              = 0x40001998;
Cache_Ibus_MMU_Set          = 0x400019a4;
Cache_Enable_ICache         = 0x40001878;
rom_config_instruction_cache_mode = 0x40001a1c;
Cache_Invalidate_ICache_All = 0x400016d4;
rom_Cache_Suspend_ICache    = 0x4000189c;
Cache_Resume_ICache         = 0x400018a8;
Cache_Invalidate_DCache_All  = 0x400016e0;
Cache_Set_IDROM_MMU_Size    = 0x40001914;
MEMORY {
  iram (RWX) : ORIGIN = 0x403C8700, LENGTH = 0xA000
  dram (RW)  : ORIGIN = 0x3FCE2700, LENGTH = 0x5000
}
SECTIONS {
  .text : ALIGN(4) {
    *(.literal .text)
    *(.literal.* .text.*)
  } > iram
  .rodata : ALIGN(4) {
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)
  } > dram
  .data : ALIGN(4) {
    _sdata = . ;
    *(.data .data.*)
    *(.sdata .sdata.*)
    _edata = . ;
  } > dram
  .bss (NOLOAD) : ALIGN(16) {
    _sbss = . ;
    *(.bss .bss.*)
    *(.sbss .sbss.*)
    *(COMMON)
    . = ALIGN(16);
    _ebss = . ;
  } > dram
}
