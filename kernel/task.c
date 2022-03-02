#include "task.h"
#include "mem.h"
#include "frame.h"
#include "common.h"
#include "../lib/string.h"
#include "sys.h"

#define TASK_MAX 32
//#define TASK_STACK_V_BASE   ( KERNEL_V_START - FRAME_SIZE )


extern uint32_t read_eip();
extern void  __context_switch(struct Task *new);
extern void  __spawn(struct Task *new);
extern void  __ret_syscall();
extern uint32_t* pg_dir0;

struct Task  task_arr[MAX_TASK_NUM];
struct Task*  current ;
struct Task*  old_task ;

//const uint32_t KSTACK_TOP = (KERNEL_V_START - FRAME_SIZE +FRAME_SIZE-sizeof(struct Task)-sizeof(int));

//TODO fix hardcore TSS address
gdt_entry_t* TSS_SEGMENT = ( gdt_entry_t*) 0x9028;
tss_entry_t tss_entry;



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


/*
    img mem layout
    [header]
    [text]
    [data]
*/

// base = kargs address

uint32_t next_task_pos(uint32_t header_addr){
    struct TsExHeader   *header =  (struct TsExHeader   *)  header_addr;
    return header_addr+header->lma_data+header->data_size;
}


void create_task_address_space(struct Task * task, uint32_t base){

    uint32_t *dir; 
    struct TsExHeader   *header =  (struct TsExHeader   *) base;

    if(header->magic != 0xABCD1234){
        ASSERT(header->magic == 0xABCD1234,"load_task_from_ram");
    }
    
    if(header->vma_data % FRAME_SIZE){
        ASSERT(header->vma_data % FRAME_SIZE == 0 ,"load_task_from_ram");
    }

    dir = get_temp_va(task->phy_dir);

    uint32_t  img_text_base  =  base + header->lma_text;
    uint32_t  img_data_base  =  base + header->lma_data;
    uint32_t* page;
    uint32_t page_phy;
    uint32_t vma_img;
    uint32_t vma ;
    //uint32_t *user_stack;

    // load text section
    request_region_vmap(dir,header->vma_text,header->text_size,PAGE_FLG_USR);

    vma_img = img_text_base;
    vma     = header->vma_text & 0xFFFFF000;

    while(vma_img<img_text_base+header->text_size){
        page_phy  =    get_phy_from_dir(dir,vma);
        page = create_temp_va(page_phy);
        memcpy((uint8_t*)vma_img,(uint8_t*)page,FRAME_SIZE);
        delete_temp_va((uint32_t)page);
        vma_img+=FRAME_SIZE;
        vma+=FRAME_SIZE;
    }

    // load data section
    // assume data section aligned at page boundary

    request_region_vmap(dir,header->vma_data,header->data_size,PAGE_FLG_USR);
    vma     = header->vma_data;
    vma_img = img_data_base ;
 
    while(vma_img<img_data_base+header->data_size){
        page_phy  =    get_phy_from_dir(dir,vma);
        page = create_temp_va(page_phy);
        memcpy((uint8_t*)vma_img,(uint8_t*)page,FRAME_SIZE);
        delete_temp_va((uint32_t)page);
        vma_img+=FRAME_SIZE;
        vma+=FRAME_SIZE;
    }

    // create user stack
    request_region_vmap(dir, USTACK_FRAME,FRAME_SIZE,PAGE_FLG_USR);
    
    delete_temp_va((uint32_t)dir);
}

/*
void exec_ram(uint32_t base){
    create_task_address_space((uint32_t*)PAGE_DIR_VA,base);
    jmp_eip(0);
}
*/

void spawn_ram(uint32_t base){
    struct Task * task ;
    task = load_task((uint32_t*)pg_dir0,base);
    __context_switch(task);
}


/*
    Goal: to  create 2 kernel thread and swtich between them w/o errors
*/

/*
    create kernel thread liked task
    share data code heap, but with private stack and regs
*/

// steps
// 1. link kernel space  data/code
// 2. allocate page directory and tables
// 3. allocate stack page
// 4. start from physical 16MB ?? 
// 5. (TODO) allocate self exetuable code memory /data (acutally it's a process so skip)
// 6. save current process state
// 7. switch code using stack/iret/jmp ...
// 8. can accept interrput

/*
    address space = page table + phy address
    kill a process = iterate through page table then free frame + free self page table
    because kernel space is linked, dont need to free, free frame is just makrk bitmap  as unused
    after all marking done, kernel space free code swtich cr0 and because previous page table frame is marked as free so 
    can use by other process


    switch address space is create page ditectory (if not exists, if exists just retrieve it)   in Process control block to cr0??
    
    how to build a page 

    create_mapping --> va pa flag 


*/

uint32_t*  alloc_kstack_and_task(uint32_t* parent_dir,uint32_t new_cr3){

    uint32_t * stack ;
    uint32_t   stack_phy;
    uint32_t * new_pg_dir;
    struct Task * task;

    new_pg_dir =  create_temp_va(new_cr3);

    request_region_vmap(new_pg_dir, KSTACK_FRAME ,FRAME_SIZE,PAGE_FLG_KERNEL);
    stack_phy  =  get_phy_from_dir(new_pg_dir,KSTACK_FRAME);
    stack      =  create_temp_va(stack_phy);
    task =   KSTACK_TO_TASK(stack);
    task->phy_dir = new_cr3;
    task->esp0 =  KSTACK_FRAME+ FRAME_SIZE  - sizeof(struct Task) - sizeof(int);
    //task->esp =   KSTACK_FRAME;
    task->ptid = current->tid;
    task->state = TASK_PRESENT;
    task->ttl = 10;
    task->tid = get_next_tid();

    // copy task to task_arr, duplicatd data
    // TODO to use linked list to save space and reduce redundant data
    task_arr[task->tid] = *task;

    delete_temp_va((uint32_t)new_pg_dir);

    return stack;
}


    
struct Task * load_task(uint32_t* parent_dir,uint32_t task_hd_addr){
    uint32_t  stack_base_phy,new_pg_dir_phy,rev_tb_phy,sys_pg_phy ;
    uint32_t *kernel_stack,*user_stack,*pt_va;
    uint32_t *new_pg_dir        =  alloc_page(USER_P_START,MAX_MEM_SIZE,&new_pg_dir_phy);  
    uint32_t *rev_tb            =  alloc_page(USER_P_START,MAX_MEM_SIZE,&rev_tb_phy); 
    uint32_t *sys_pg            =  alloc_page(USER_P_START,MAX_MEM_SIZE,&sys_pg_phy); 
    uint32_t new_tid ;
    uint32_t phy_pt,va;
    struct Task *task = NULL;

    uint32_t i;

    user_stack = kernel_stack = NULL;

    //init page dir and rev dir , set some system mappings
    pg_dir_add(new_pg_dir ,rev_tb,1023,rev_tb_phy,PAGE_FLG_KERNEL);

    sys_pg[1023] = new_pg_dir_phy | PAGE_FLG_KERNEL; 
    pg_dir_add(new_pg_dir ,rev_tb,1022,sys_pg_phy,PAGE_FLG_KERNEL);


    // link kernel space to new process  (except rev_tb,sys_tb)
    for(i=PAGE_DIR_INDEX(KERNEL_V_START);i<=1021;++i){
        new_pg_dir [i] = parent_dir[i];
    }

    //allocate Task struct and initial stack
    //0xC0000000 

    kernel_stack = alloc_kstack_and_task(parent_dir,new_pg_dir_phy);
    task      = KSTACK_TO_TASK(kernel_stack);
    new_tid   = task->tid;
    // user stack , text , data 
    
    create_task_address_space(task,task_hd_addr);

    struct TrapFrame * tf ;
    tf = (struct TrapFrame *)((char*)task - sizeof(*tf));

    /*
        H
            stack layout :
            stack_frame_base+ frame size
            struct Task
            iret registers
            segments registers (ds,es ...) 
            struct Context Register  <-- trap_frame   <-- esp
            
           
        L
    
    */
    task->esp = KSTACK_FRAME + FRAME_SIZE - sizeof(*task)-sizeof(*tf); 

    memset((char*)tf,0,sizeof(*tf));
   
    tf->cs = 0x1b;
    tf->context_reg.eip = (uint32_t)&__ret_syscall;
    
    tf->ds = tf->es = tf->gs = tf->fs = tf->ss = 0x23;
    tf->iret_eip = 0 ;
    tf->iret_esp = USTACK_FRAME+FRAME_SIZE-sizeof(int);
    //enable interrupt when switch to user mode 
    tf->eflags   = 1<<9; // #########
    // #############################

    task_arr[new_tid] = *task;

    delete_temp_va((uint32_t)new_pg_dir);
    delete_temp_va((uint32_t)rev_tb);
    delete_temp_va((uint32_t)sys_pg );
    delete_temp_va((uint32_t) kernel_stack );
    return  &task_arr[new_tid];
}


void  context_switch(struct Task* new){
   
    if( current == new ){
        return ;
    }

    tss_entry.esp0 = new->esp0;
    tss_entry.ss0  = 0x10; 
    __context_switch(new);
}

//https://csiflabs.cs.ucdavis.edu/~ssdavis/50/att-syntax.htm
//https://stackoverflow.com/questions/54386736/what-is-jmpl-instruction-in-x86
// TODO -->  don't call with stack version

/*
    init task 0 --> first task is the code after enter kernel and create task0, must fill task structure by
                    itself




*/

void init_task0(){
    extern uint32_t karg_phy;
    struct Task *task;
    
    uint32_t * kstack;
    uint32_t i ;

    for(i=0;i<MAX_TASK_NUM;++i){
        task_arr[i].state = 0;
    }

    // task 0 comes from start up code and data
    // task 1 load init.c and execute
    kstack = alloc_kstack_and_task(pg_dir0,PG_DIR0_ADDR);
    task   = KSTACK_TO_TASK(kstack);
    current = &task_arr[task->tid];
    setup_tss();
    register_syscall(SYS_EXIT,terminate_process);
    spawn_ram(PHY_TO_KVM(karg_phy));
    asm volatile("sti");
    printf("init task 0 end\n");
    while(1){
        i = 0;
        while(i<0x8FFFFFF){++i;};
        printf("I am kernel\n");
    };
}

uint32_t get_next_tid(){
    uint32_t i;
    for(i=0;i<MAX_TASK_NUM;++i){
        if(!task_arr[i].state){
            return i;
        }
    }
    ASSERT(1==0,"can't get next tid");
    return 0;
}

void cond_schedule(){
    if(current->state & TASK_NEED_SCHED){
        schedule();
    }
}

void schedule(){
    uint32_t i;
    struct Task* next_task = NULL;
    //uint32_t parent_id = current->ptid;
    for(i=1;i<=MAX_TASK_NUM;++i){
        if(task_arr[(current->tid+i)%MAX_TASK_NUM].state){
            next_task=&task_arr[(current->tid+i)%MAX_TASK_NUM];
            break;
        }
    }
    if(current->state){
        current->state &=~(TASK_NEED_SCHED);
        current->ttl = 10;
        context_switch(next_task);
    }else{
        context_switch(next_task);
    }
}


// this piece of code mainly for debug and test, should never be called

void switch_to_user(uint32_t new_eip){
    // mov eax,0x8000000 ; default qemu memory size is 128MB = 0x8000000, if greater than 128MB it will be zeros when reference  ....
    // it seems like VM's behavior, not x86 real hardware
    uint32_t x = new_eip;
    //asm volatile ("jmp $0x1b,$0");
    /////////////// 0x200 for enable interrupt
    asm volatile ( 
                    "movl  $0x23,%%eax \n\t"
                    "pushl %%eax \n\t" 
                    "movl  $0xBFFFFFF0,%%eax \n\t"
                    "pushl %%eax\n\t"

                    "pushf \n\t"
                    "popl %%eax; \n\t"
                    "orl $0x000,%%eax \n\t"
                    "pushl %%eax \n\t"

                    "movl $0x1B,%%eax \n\t"
                    "pushl %%eax \n\t"            
                    "movl $0,%%eax \n\t"
                    "pushl %%eax \n\t"
                    
                    "movl $0x23,%%eax \n\t"
                    "mov %%ax,%%ds \n\t"
                    "mov %%ax,%%es \n\t"   
                    "mov %%ax,%%fs \n\t"   
                    "mov %%ax,%%gs \n\t" 
                    "iret \n\t"            
                    : 
                    : 
                    :"eax"
    );
}


void setup_tss(){
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(tss_entry);   
    gdt_set_gate(TSS_SEGMENT,base,limit,0xE9,0x00);
    tss_entry.esp0 = 0xFFFFFF00;
    tss_entry.ss0  = 0x10;
    //why 2B?
    asm volatile( "ltr %%ax" ::"a"(0x28):);
}

/*
void user_loop(){
    while(1){
        execute_task();
    }
}
*/

/*
void execute_task(){

    if(task_head == task_tail){
        return ;
    }

    printstr("execute task");
    print_hex(simple_task_list[task_tail].task_id);
    printl("");
    uint32_t old_esp = 0 ;
    // can't set ebp in inline asm ...
    asm volatile ( 
                    "movl %%esp,%0 \n\t"
                    "movl %1,%%esp \n\t"
                    "movl %2,%%eax \n\t"
                    : "=&r" (old_esp)
                    : "r"( simple_task_list[task_tail].esp),"r"( simple_task_list[task_tail].eip)
                    : "eax"
    );

    asm volatile ( 
                    "movl %0,%%esp \n\t"
                    "call *%%eax \n\t"
                    :
                    : "r"( old_esp)
                    : "eax"
    );

    pop_task();
    printl("finish task");
}
*/


int terminate_process(){
    printf("terminate process\n");
    current->state = 0;
    schedule();
    return 0;
}

// template code 
void kill_and_reschedule(){
    printl("kill and reschedule");
    //pop_task();
    //switch_to_user((uint32_t)user_loop);
}





