#include "task.h"
#include "mem.h"
#include "frame.h"
#include "common.h"
#include "../lib/string.h"

#define TASK_MAX 32
#define TASK_STACK_V_BASE   ( 0xC0000000 - 0x1000 )


extern uint32_t read_eip();

struct Task  task_arr[MAX_TASK_NUM];
struct Task*  current ;
struct Task*  old_task ;

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


void create_task_address_space(uint32_t *dir, uint32_t base){

    struct TsExHeader   *header =  (struct TsExHeader   *) base;

    if(header->magic != 0xABCD1234){
        ASSERT(header->magic == 0xABCD1234,"load_task_from_ram");
    }
    
    if(header->vma_data % FRAME_SIZE){
        ASSERT(header->vma_data % FRAME_SIZE == 0 ,"load_task_from_ram");
    }

  
    uint32_t  img_text_base  =  base + header->lma_text;
    uint32_t  img_data_base  =  base + header->lma_data;
    uint32_t* page;
    uint32_t page_phy;
    uint32_t vma_img;
    uint32_t vma ;

    // load text section
    request_region_vmap(dir,header->vma_text,header->text_size,PAGE_FLG_KERNEL);

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

    request_region_vmap(dir,header->vma_data,header->data_size,PAGE_FLG_KERNEL);
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


struct Task * load_task(uint32_t* parent_dir,uint32_t task_hd_addr){
    
    uint32_t  stack_base_phy,new_pg_dir_phy,rev_tb_phy,sys_pg_phy ;
    uint32_t *new_stack = NULL;

    uint32_t *new_pg_dir        =  alloc_page(USER_P_START,MAX_MEM_SIZE,&new_pg_dir_phy);  
    uint32_t *rev_tb            =  alloc_page(USER_P_START,MAX_MEM_SIZE,&rev_tb_phy); 
    uint32_t *sys_pg            =  alloc_page(USER_P_START,MAX_MEM_SIZE,&sys_pg_phy); 
    uint32_t new_tid;
    struct Task *task = NULL;
    

    uint32_t i;
    // link kernel space to new process  (except rev_tb,sys_tb)
    for(i=PAGE_DIR_INDEX(KERNEL_V_START);i<=1021;++i){
        new_pg_dir [i] = parent_dir[i];
    }


    //init page dir and rev dir , set some system mappings
    pg_dir_add(new_pg_dir ,rev_tb,1023,new_pg_dir_phy,PAGE_FLG_KERNEL);

    sys_pg[1023] = new_pg_dir_phy | PAGE_FLG_KERNEL; 
    pg_dir_add(new_pg_dir ,rev_tb,1022,sys_pg_phy,PAGE_FLG_KERNEL);

    //allocate Task struct and initial stack
    //0xC0000000 

    request_region_vmap(new_pg_dir,TASK_STACK_V_BASE,FRAME_SIZE,PAGE_FLG_KERNEL);
    stack_base_phy = get_phy_from_dir(new_pg_dir,TASK_STACK_V_BASE);
    new_stack      = create_temp_va(stack_base_phy);

    task = (struct Task *)((uint32_t)new_stack+FRAME_SIZE-sizeof(struct Task));
    task->phy_dir = new_pg_dir_phy;
    task->esp0 = TASK_STACK_V_BASE+FRAME_SIZE-sizeof(struct Task);
    task->ebp0 = task->esp0;
    task->eip = 0;
    task->ptid = current->tid;
    task->state = TASK_ALLOCATED;
    task->ttl = 10;
    new_tid = task->tid = get_next_tid();

   
    // copy task to task_arr, redundant data 
    // TODO to use linked list to save space and reduce redundant data
    task_arr[task->tid] = *task;

    create_task_address_space(new_pg_dir,task_hd_addr);
    delete_temp_va((uint32_t)new_pg_dir);
    delete_temp_va((uint32_t)rev_tb);
    delete_temp_va((uint32_t)sys_pg );
    delete_temp_va((uint32_t) task & 0xFFFFF000 );
    return  &task_arr[new_tid];
}

//https://csiflabs.cs.ucdavis.edu/~ssdavis/50/att-syntax.htm
//https://stackoverflow.com/questions/54386736/what-is-jmpl-instruction-in-x86
// TODO -->  don't call with stack version

/*
    init task 0 --> first task is the code after enter kernel and create task0, must fill task structure by
                    itself




*/

extern void  save_context();
extern void  context_switch(struct Task *new);
extern void  __switch_task();

void init_task0(){
    uint32_t i ;
    for(i=0;i<MAX_TASK_NUM;++i){
        task_arr[i].state = 0;
    }
    task_arr[0].tid = 0;
    task_arr[0].ptid = 0;
    task_arr[0].state = TASK_ALLOCATED ;
    task_arr[0].ttl =  250 ;
    //_save_context(&task_arr[0]);
    current = &task_arr[0];
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
    if(current->state& TASK_NEED_SCHED){
        schedule();
    }
}

void schedule(){
    uint32_t i;
    struct Task* next_task = NULL;
    //uint32_t parent_id = current->ptid;
    for(i=1;i<MAX_TASK_NUM;++i){
        if(task_arr[(current->tid+i)%MAX_TASK_NUM].state){
            next_task=&task_arr[(current->tid+i)%MAX_TASK_NUM];
            break;
        }
    }
    if(current->state){
        current->state &=~(TASK_NEED_SCHED);
        save_context();
        current->ttl = 10;
        context_switch(next_task);
    }else{
        __switch_task();
    }
}


/*
    current implementation only remove tcb from global task array
    but task resources not really release
*/

int terminate_process(){
    printl("terminate process");
    current->state = 0;
    ///task_arr[current->tid].state = 0;
    schedule();
    return 0;
}



/*
    iret stack
    w/o priviledge switch
    [esp + 12] eflags
    [esp + 8]  cs
    [esp + 4]  eip
    [esp]      error code?
*/


// task switch cooperative path + timer path
// in kernel mode
//        call schedule 
//             --> save registers
//             --> context switch
//                     will be used for isr and normal function call
//                                        timer --> iret for target EIP




// two questiob : 
//      how to switch to right eip?
//      is the saved old eip right?          
//      gcc version for x86 don't support naked .... 



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


//;0xe05b


void setup_tss(){
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(tss_entry);   
    gdt_set_gate(TSS_SEGMENT,base,limit,0xE9,0x00);
    tss_entry.ss0  = 0x10;  // Set the kernel stack segment.
    tss_entry.esp0 = 0x15F000; // Set the kernel stack pointer.
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


