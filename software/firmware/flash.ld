
SECTIONS
{
  .ivt 0x00000 : { *(.ivt) }
  .opt 0x000c0 : { *(.opt) }
  .text 0x000ce : { __btext = ABSOLUTE(.); *(.text) __etext = ABSOLUTE(.); }
  .plt : { *(.plt); }
  .init ALIGN(2) : {
    __binit = ABSOLUTE(.);
    . += SIZEOF(.data);
    __broinit = ABSOLUTE(.);
    . += SIZEOF(.rodata);
    __einit = ABSOLUTE(.);
  }
  . = 0xfbf00;
  .bss : { __bbss = ABSOLUTE(.); *(.bss) __ebss = ABSOLUTE(.); }
  .data : AT(__binit) {
    __bdata = ABSOLUTE(.); *(.data)
  }
  .rodata : AT(__broinit) {
    __brodata = ABSOLUTE(.);
    *(.rodata)
    __edata = ABSOLUTE(.);
  }
}
