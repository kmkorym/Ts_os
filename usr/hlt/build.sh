nasm start.asm -f elf32  -o start.o 
gcc  -m32 -ffreestanding   -c  hlt.c 
ld  -m elf_i386  start.o  hlt.o -nostdlib  -T hlt.ld   -o  hlt.elf
objcopy hlt.elf -O binary hlt.bin
cp  hlt.bin ~/Desktop
