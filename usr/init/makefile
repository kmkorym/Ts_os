CFLAGS = -m32 -ffreestanding  -g -fno-asynchronous-unwind-tables -fno-pie 

HEADERS = $(wildcard ./*.h)
C_SOURCES = $(wildcard ./*.c)
#$(info $$var is [${var}])
OBJS = ${C_SOURCES:.c=.o}
BINS = ${C_SOURCES:.c=.bin}


INCLUDE_PATH = ../../kernel/ ../lib/
INCLUDE_SRC = ../../kernel/sys.c  ../lib/lib.c
INCLUDE_OBJS = ${INCLUDE_SRC:.c=.o}

#CFILE=''


.PRECIOUS:  %.o %.elf

all :  ${BINS}
	mkdir -p tasks
	mv ${BINS} tasks

build:
	make   ${CFILE:.c=.bin}  
	
%.bin: %.elf 
	objcopy   $<  -O binary $@ 
	objcopy --only-keep-debug $<  $(subst .elf,.sym,$<) 
	
%.elf: start.o  %.o ${INCLUDE_OBJS}
	ld -m elf_i386 -nostdlib  -T init.ld  $^ -o $@

%.o: %.c
	gcc ${CFLAGS}  -I../../kernel/ -I../lib/ -c $< -o $@

start.o: ${INCLUDE_OBJS}
	nasm  start.asm  -f elf32  -o start.o 


clean:
	rm -f ./*.o ./*.elf ./*.bin  ../lib/*.o ./*.sym
	rm -rf tasks/
