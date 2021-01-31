run:
	qemu-system-i386 -boot order=a -fda kimage

build: kimage

kimage: boot.bin kernel.bin
	cat boot.bin kernel.bin > kimage

kernel.bin: kernel_entry.o util.o
	ld -m elf_i386 -o kernel.bin -Ttext 0x9527  kernel_entry.o util.o --oformat binary

kernel_entry.o:
	nasm kernel_entry.asm -f elf -o kernel_entry.o

util.o:
	gcc -m32 -ffreestanding -c uitl.c  -o util.o

boot.bin:
	nasm  -f bin mbr.asm  -o boot.bin 

clean:
	rm boot.bin kernel.bin kernel_entry.o util.o kernel_entry.o
  
	