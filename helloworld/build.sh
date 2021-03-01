nasm start.asm -f elf32  -o start.o 
gcc  -m32 -ffreestanding   -c  helloworld.c 
ld  -m elf_i386  start.o  helloworld.o -nostdlib  -T hl.ld   -o  helloworld.elf
objcopy helloworld.elf -O binary helloworld.bin
cp  helloworld.bin ~/Desktop
