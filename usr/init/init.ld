OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    OVERLAY : {
	    .hd  {
        SHORT(0x1234)
        SHORT(0xABCD)
        LONG(LOADADDR(.text))
		    LONG(ADDR(.text))
		    LONG(SIZEOF(.text))
        LONG(LOADADDR(.data))
		    LONG(ADDR(.data))
		    LONG(SIZEOF(.data))
	    }
	    .text  {
	     *(.text);
	     *(.rodata);
	   }
    }

   .data ALIGN(4096) :  AT(LOADADDR(.text)+SIZEOF(.text)) {
    *(.data);
    *(.bss);
   }
  
  /DISCARD/ : {

    *(.note.gnu.property);

  }




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