CFLAGS = -m32 -ffreestanding  -g -fno-asynchronous-unwind-tables -fno-pie
HEADERS = $(wildcard ./*.h)
C_SOURCES = $(wildcard ./*.c)
OBJS = ${C_SOURCES:.c=.o}
BINS = ${OBJS:.o=.bin}

.PRECIOUS:  %.o %.elf

all : build

build: ${BINS}
	
%.bin: %.elf 
	objcopy   $<  -O binary $@ 
	
%.elf: %.o
	ld -m elf_i386 -nostdlib  -T init.ld  $^ -o $@

%.o: %.c
	gcc ${CFLAGS}  -c $< -o $@
clean:
	rm -f ./*.o ./*.elf ./*.bin
