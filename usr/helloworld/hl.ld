OUTPUT_FORMAT(elf32-i386)
ENTRY(main)
phys = 0x50000;
SECTIONS
{
  .text phys : AT(phys) {
    *(.text)
   }
   .data :{
    *(.data)
   }
   .rodata :{
    *(.rodata)
   }
}