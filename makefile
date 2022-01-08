CFLAGS = -m32 -ffreestanding  -g -fno-asynchronous-unwind-tables -fno-pie
HEADERS = $(wildcard kernel/*.h  drivers/*.h lib/*.h fs/*.h)
C_SOURCES = $(wildcard kernel/*.c drivers/*.c lib/*.c fs/*.c)
OBJS = $(C_SOURCES:.c=.o)
#BUILD_OBJS = $(addprefix build/,$(notdir $(C_SOURCES:.c=.o)))




build_test_mbr: boot.bin
	mv boot.bin  build
	./scripts/bootloader.sh  build -f build/boot.bin  -o  build/test_mbr.bin
	

deploy: kimage boot.bin early_setup.bin early_setup.elf  kernel.bin   kernel/init_mem.o early_setup.o kernel_entry.o process.o  kernel.elf kernel.sym early_setup.sym  ${OBJS}
	mv   $^  build/

%.o: %.c
	gcc ${CFLAGS} -ffreestanding -c $< -o $@

build: kimage

kimage: boot.bin early_setup.bin kernel.bin user_tasks kernel.sym early_setup.sym 
	cat boot.bin  early_setup.bin  kernel.bin  usr/init/tasks/*.bin  > kimage
	truncate -s 1M kimage

user_tasks :
	make -C usr/init/

kernel.bin: kernel.elf
	objcopy kernel.elf -O binary kernel.bin

kernel.sym: kernel.elf
	objcopy --only-keep-debug kernel.elf kernel.sym

kernel.elf: kernel_entry.o process.o  ${OBJS}
	ld -m elf_i386 -nostdlib  -T linker.ld  $^ -o $@

early_setup.bin: early_setup.elf
	objcopy early_setup.elf -O binary early_setup.bin

early_setup.sym: early_setup.elf
	objcopy --only-keep-debug   early_setup.elf   early_setup.sym

early_setup.elf: early_setup.o  kernel/init_mem.o kernel/init_console.o kernel/init_vga.o lib/init_print.o lib/string.o
	ld -m elf_i386 -nostdlib  -T esetup.ld  $^ -o $@

kernel/init_console.o: kernel/init_vga.o
	gcc ${CFLAGS} -ffreestanding  -D EARLY_INIT -c kernel/console.c -o kernel/init_console.o

kernel/init_vga.o:
	gcc ${CFLAGS} -ffreestanding  -D EARLY_INIT -c kernel/vga.c -o kernel/init_vga.o

kernel/init_mem.o:
	gcc ${CFLAGS} -ffreestanding  -D EARLY_INIT -c kernel/mem.c -o kernel/init_mem.o

lib/init_print.o:
	gcc ${CFLAGS} -ffreestanding  -D EARLY_INIT -c lib/print.c  -o lib/init_print.o

kernel_entry.o:
	nasm  boot/kernel_entry.asm  -f elf32  -o kernel_entry.o 

early_setup.o:
	nasm  boot/early_setup.asm  -f elf32  -o early_setup.o 

process.o:
	nasm asm/process.asm -f elf32  -o process.o 

boot.bin:
	nasm  -f bin boot/mbr.asm  -o boot.bin 

clean:
	rm -f build/*  boot.bin early_setup.bin lib/*.o  *.elf  kernel/*.o  drivers/*.o ./*.o
	make -C usr/init/ clean

