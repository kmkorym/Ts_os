CFLAGS = -m32 -ffreestanding  -g 
HEADERS = $(wildcard kernel/*.h  drivers/*.h lib/*.h)
C_SOURCES = $(wildcard kernel/*.c drivers/*.c lib/*.c)
OBJS = ${C_SOURCES:.c=.o}
RUN_OPTIONS = -boot order=a -fda build/kimage -chardev pipe,mux=on,id=sp0,path=/home/uabharuhi/Desktop/com1 \
-mon chardev=sp0,mode=readline  -serial chardev:sp0


run: 
	qemu-system-i386  ${RUN_OPTIONS}

debug: 
	qemu-system-i386 ${RUN_OPTIONS} -S -s &
	gdb -ex 'target remote localhost:1234' \
		-ex 'set architecture i386' \
		-ex 'symbol-file build/kernel.sym'\  
		-ex 'b main'\
		-ex 'c'

deploy: kimage boot.bin kernel.bin kernel_entry.o  kernel.elf kernel.sym  ${OBJS}
	mv   $^  build/

build: kimage


kimage: boot.bin kernel.bin kernel.sym 
	cat boot.bin kernel.bin > kimage
	truncate -s 1MB kimage

kernel.bin: kernel.elf
	objcopy kernel.elf -O binary kernel.bin

kernel.sym: kernel.elf
	objcopy --only-keep-debug kernel.elf kernel.sym

kernel.elf: kernel_entry.o  ${OBJS}
	ld -m elf_i386 -nostdlib  -T linker.ld  $^ -o $@

%.o: %.c
	gcc ${CFLAGS} -ffreestanding -c $< -o $@


kernel_entry.o:
	nasm boot/kernel_entry.asm -f elf32  -o kernel_entry.o 

boot.bin:
	nasm  -f bin boot/mbr.asm  -o boot.bin 


clean:
	rm  build/*
  
	