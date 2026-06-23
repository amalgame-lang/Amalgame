ENTRY(_xip)
MEMORY { irom (RX) : ORIGIN = 0x42000000, LENGTH = 0x10000 }
SECTIONS { .text : { *(.text .text.*) } > irom }
