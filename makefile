# run_debug:
#	qemu-system-i386 -boot order=a -fda kdgimage


# build_debug: boot.bin dbg_kernel.bin
#	cat boot.bin dbg_kernel.bin >kdgimage

# dbg_kernel.bin:

run:
	qemu-system-i386 -boot order=a -fda kimage

build: boot.bin kernel.bin kernel.sym 
	cat boot.bin kernel.bin > kimage
	truncate -s 1MB kimage

kernel.bin: kernel.elf
	objcopy kernel.elf -O binary kernel.bin

kernel.sym: kernel.elf
	objcopy --only-keep-debug kernel.elf kernel.sym

kernel.elf: kernel_entry.o   main.o  util.o print.o isr.o
	ld -m elf_i386 -nostdlib  -T linker.ld  $^ -o kernel.elf


kernel_entry.o:
	nasm kernel_entry.asm -f elf32  -o kernel_entry.o

isr.o:
	gcc -m32 -ffreestanding -c   isr.c -g -o isr.o

print.o:
	gcc -m32 -ffreestanding -c   print.c -g -o print.o

util.o:
	gcc -m32 -ffreestanding -c   uitl.c -g -o util.o

main.o:
	gcc -m32 -ffreestanding -c  main.c -g -o main.o

boot.bin:
	nasm  -f bin mbr.asm  -o boot.bin 



clean:
	rm *.bin  *.o
  
	