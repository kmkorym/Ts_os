OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
SECTIONS
{
   . = 0;
   .kernel_setup :{
        /* all code in kernel setup must be position independant*/
        __kernel_size = _kernel_end - _kernel_start;
        *(.kernel_setup)
    }
     _kernel_setup_end = . ;
    . = 0xC0800000;
     _kernel_start =  . ;
   .text :  AT ( _kernel_setup_end) {
     *(.text)
     *(.rodata)
   }

   .data :{
    *(.data)
    *(.bss)
   }
   _kernel_end=.;

/* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  
}