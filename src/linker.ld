ENTRY(_start)
PROVIDE(_kernel_start = 0x00010000);
SECTIONS {
  . = 0x00010000;

  .text : {
    *(.text*)
  }
  .rodata : {
    *(.rodata*)
  }
  .data : {
    *(.data*)
  }
  .bss : {
    *(COMMON)
    *(.bss*)
  }
}

PROVIDE(_kernel_end = .);
