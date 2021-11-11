#ifndef _TASKH_
#define _TASKH_


#include "common.h"
#include "../lib/print.h"
#define MAX_TASK_NUM 32
#define TASK_PRESENT 0x1
#define TASK_NEED_SCHED 0x2
/*
Multi-tasking

task loading mechanism ver 0.1
1. embedded task code/data in kimage after kernel.bin
2. each task has a    
	- header
		- has text lma/vma/size
		- generated by linker script
	- raw binary
3. read the task headers to load code/data in kimage

4. task control block 
    - kernel space array for each task
    - address space (page dir pyhiscal address) 
    - registers state
    - allocate stack in 3GB (top most user address space ) when create task
      and PCB will allocate to there and stack address is below it
    - setup init value of register and save register value of current ??? 

5. create 3 or more task and kernel can switch between them
    - by timer 
    - by keyboard interrupt command
*/



struct TsExHeader{
   uint32_t   magic; // 0xABCD1234
   uint32_t   lma_text;
   uint32_t   vma_text;
   uint32_t   text_size;
   uint32_t   lma_data;
   uint32_t   vma_data;
   uint32_t   data_size;
}__attribute__((packed));



struct ContextRegister{
   uint32_t esp;
   uint32_t ebp;
   uint32_t eip;
   uint32_t edi;
   uint32_t esi;
   uint32_t eax;
   uint32_t ebx;
   uint32_t ecx;
   uint32_t edx;
   uint32_t flags;
};



struct tss_entry_struct
{
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // Unused...
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         // The value to load into ES when we change to kernel mode.
   uint32_t cs;         // The value to load into CS when we change to kernel mode.
   uint32_t ss;         // The value to load into SS when we change to kernel mode.
   uint32_t ds;         // The value to load into DS when we change to kernel mode.
   uint32_t fs;         // The value to load into FS when we change to kernel mode.
   uint32_t gs;         // The value to load into GS when we change to kernel mode.
   uint32_t ldt;        // Unused...
   uint16_t trap;
   uint16_t iomap_base;
} __attribute__((packed));
typedef struct tss_entry_struct tss_entry_t;


struct gdt_entry_struct
{
   uint16_t limit_low;           // The lower 16 bits of the limit.
   uint16_t base_low;            // The lower 16 bits of the base.
   uint8_t  base_middle;         // The next 8 bits of the base.
   uint8_t  access;              // Access flags, determine what ring this segment can be used in.
   uint8_t  granularity;
   uint8_t base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

struct simple_task
{
    uint32_t task_id;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
};


#define TASK_ALLOCATED 1

struct Task{
   // field of registers
   uint32_t esp0;
   uint32_t eip;
   uint32_t phy_dir;
   uint32_t ebp0;
   uint32_t ptid;   
   uint32_t tid;
   uint32_t state;   /* 0: unallocated */
   uint32_t ttl;
   // struct ContextRegister regs;
}__attribute__((packed));



uint32_t next_task_pos(uint32_t header_addr);
uint32_t get_next_tid();
void schedule();
void cond_schedule();
void init_task0();
void start_task0(uint32_t* kernel_dir);
void execute_task();
void setup_tss();
void user_loop();
void kill_and_reschedule();
void pop_task();
void add_task( struct simple_task task);
void switch_to_user(uint32_t new_eip);
void switch_task(struct Task *task);
struct Task * load_task(uint32_t* parent_dir,uint32_t task_hd_addr);
int terminate_process();
#endif