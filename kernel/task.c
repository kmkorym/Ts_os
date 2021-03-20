#include "task.h"
#define TASK_MAX 128


//TODO fix hardcore TSS address
gdt_entry_t* TSS_SEGMENT = ( gdt_entry_t*) 0x9028;
tss_entry_t tss_entry;

static struct simple_task simple_task_list[TASK_MAX];
static  int task_head =  0 ; 
static  int task_tail =  0 ; 

static void gdt_set_gate(gdt_entry_t* entry, uint32_t base, uint32_t limit, uint8_t access,  uint8_t gran)
{
   entry->base_low    = (base & 0xFFFF);
   entry->base_middle = (base >> 16) & 0xFF;
   entry->base_high   = (base >> 24) & 0xFF;

   entry->limit_low   = (limit & 0xFFFF);
   entry->granularity = (limit >> 16) & 0x0F;

   entry->granularity |= gran & 0xF0;
   entry->access      = access;
}


void switch_to_user(uint32_t new_eip){
    // mov eax,0x8000000 ; default qemu memory size is 128MB = 0x8000000, if greater than 128MB it will be zeros when reference  ....
    // it seems like VM's behavior, not x86 real hardware
    uint32_t x = new_eip;
    asm volatile ( 
                    "movl  $0x23,%%eax \n\t"
                    "pushl %%eax \n\t"

                    "movl  $0x8000000, %%eax \n\t"
                    "pushl %%eax\n\t"

                    "pushf \n\t"
                    "popl %%eax; \n\t"
                    "orl $0x200,%%eax \n\t"
                    "pushl %%eax \n\t"

                    "movl $0x1B,%%eax \n\t"
                    "pushl %%eax \n\t"

                    "movl %0,%%eax \n\t"
                    "pushl %%eax \n\t"             
                  
                    "movl $0x23,%%eax \n\t"
                    "mov %%ax,%%ds \n\t"
                    "mov %%ax,%%es \n\t"   
                    "mov %%ax,%%fs \n\t"   
                    "mov %%ax,%%gs \n\t"   
                    "iret \n\t"            
                    : 
                    : "r"(x)
                    :"eax"
    );
}




void setup_tss(){
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(tss_entry);   
    gdt_set_gate(TSS_SEGMENT,base,limit,0xE9,0x00);
    tss_entry.ss0  = 0x10;  // Set the kernel stack segment.
    tss_entry.esp0 = 0x90000; // Set the kernel stack pointer.
}


void user_loop(){
    while(1){
        execute_task();
    }
}


void execute_task(){

    if(task_head == task_tail){
        return ;
    }

    struct simple_task current = simple_task_list[task_tail] ; 
    printstr("execute task");
    print_hex(current.task_id);
    printl("");
    uint32_t old_esp = 0 ;
    asm volatile ( 
                    "pushal \n\t"
                    "movl %%esp,%0 \n\t"
                    "movl %1,%%eax \n\t"
                    "movl %2,%%eax \n\t"
                    "movl %3,%%eax \n\t"
                    "call *%%eax \n\t"
                    "popal"
                    : "=r" (old_esp)
                    : "r"(current.esp),"r" (current.ebp),"r"(current.eip)
                    : "eax"
    );
    pop_task();
    printl("finish task");
}


void kill_and_reschedule(){
    printl("kill and reschedule");
    pop_task();
    switch_to_user((uint32_t)user_loop);
}


void add_task( struct simple_task task){
    int next_head = (task_head  + 1) % TASK_MAX;
    if( next_head == task_tail){
        printl("task is full"); return;
    }
    simple_task_list[task_head] = task;
    task_head = next_head;
}

void pop_task(){
    if(task_head == task_tail){
        return ;
    }
    ++task_tail;
}
