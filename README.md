# Ts_OS

An experimental tiny x86 kernel inspired by [xv6](https://github.com/mit-pdos/xv6-public) and [JamesM's kernel development](http://www.jamesmolloy.co.uk/tutorial_html/).

The goal of this project is to learn operating system concepts by actually design and implement a kernel from scratch. This project is still under development and has some limitation which may be improved in the future depending on the priotriy.

## Features 
Features and facilities are implemented on the current kernel are :

- Bootloader in real mode loads kernel image on floppy disk
- Protected mode 
- Interrupt and interrupt handler
- Device driver for keyboard/serial port (COM)/timer/IDE disk 
- Paging
- Higher half kernel
- Kernel heap, virtual memory allocator
- Multi-tasking and round-robin scheduler
- FAT16 filesystem (read/write/search/create/delete) file
- Console can browse output history (user control the console by up/down key to show specific part of history)
- User mode (debugging now)

## Limitation

The above features have some limitations includes: 

- Only supports protected mode, 64-bit mode is not available
- Can't boot from GRUB, the kernel must boot from **its own bootlader**  
- The bootloader loads the whole kernel in real mode and there is not a second -stage bootloader can load kernel on disk image - which means the kernel image size must smaller than **1MB**
- Testing environment only on QEMU now, may not be able to boot on real hardware
- Thread is not implemented
- Locking/Synchronization mechanism are not supported
- Sleep/Blocking IO operation is not supported
- SMP is not supported, only use 1 core
- IDE hard disk driver only supports PIO mode, speed is slow 
- Not supports exeutable format like a.out/ELF
- VFS is not support

## Future Goals

The following list are features I will implement in the future, features ranked by priority (the highest one first)

- System call (reference to xv6)
- Support ELF/a.out executable
- Shell implements subset of shell syntax
- Stress test scenarios for mult-tasking
- Design documentation
- VFS
- Userspace tool (reference to xv6)
- Second-stage bootlader, load kernel from hard disk 
- Blocking IO
- Basic synchronization mechanism (spinlock,mutex)
- IDE disk driver with DMA
- Network driver
- Network protocol stack 
