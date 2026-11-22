ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);
_Min_Heap_Size = 0x000;
_Min_Stack_Size = 0x400;

MEMORY
{
  RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 20K
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
}

SECTIONS
{
  .isr_vector : { KEEP(*(.isr_vector)) } >FLASH
  .text : { *(.text*) *(.rodata*) KEEP(*(.init)) KEEP(*(.fini)) } >FLASH
  .ARM : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } >FLASH
  .preinit_array : { PROVIDE_HIDDEN(__preinit_array_start = .); KEEP(*(.preinit_array*)) PROVIDE_HIDDEN(__preinit_array_end = .); } >FLASH
  .init_array : { PROVIDE_HIDDEN(__init_array_start = .); KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array*)) PROVIDE_HIDDEN(__init_array_end = .); } >FLASH
  .fini_array : { PROVIDE_HIDDEN(__fini_array_start = .); KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array)) PROVIDE_HIDDEN(__fini_array_end = .); } >FLASH
  _etext = .;
  .data : { _sdata = .; *(.data*) _edata = .; } >RAM AT>FLASH
  _sidata = LOADADDR(.data);
  .bss : { _sbss = .; *(.bss*) *(COMMON) _ebss = .; } >RAM
  ._user_heap_stack : { . = ALIGN(8); PROVIDE(end = .); . = . + _Min_Heap_Size + _Min_Stack_Size; . = ALIGN(8); } >RAM
  /DISCARD/ : { libc.a (*) libm.a (*) libgcc.a (*) }
}
