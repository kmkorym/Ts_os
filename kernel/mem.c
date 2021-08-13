#include "mem.h"
#define _1MB  0x100000
#define PAGE_DIR_INDEX(addr)(addr>>22)
#define PAGE_TABLE_INDEX(addr)((addr>>12)&0x3FF)
#define PAGE_OFFSET(addr)(addr&(~0xFFFFF000))
#define PAGE_TABLE_START(dir_idx)((uint32_t)page_table+4096*dir_idx)
#define PAGE_ALIGNED(addr)(addr% FRAME_SIZE == 0)
#define PAGE_DIR_ADDR    (_1MB) 
#define KERNEL_LOAD_ADDR 0x9030
#define KERNEL_V_START   0xC0000000
#define KERNEL_P_START  (8*_1MB)
#define USER_P_START  (16*_1MB)
#define KERNEL_VMAP_SIZE 0x800000
#define MAX_MEM_SIZE (128*_1MB)


// the address for page dir,page table before enable page is physical but after enable page table
// the address for them are virtual 

#ifdef EARLY_INIT
uint32_t  page_dir_addr =  PAGE_DIR_ADDR;                     
uint32_t  *page_dir =    (uint32_t*)  PAGE_DIR_ADDR; // max 4KB 1024*4MB = 4G
uint32_t  *page_table =  (uint32_t*) 0x101000; // start from 1MB+4KB
#else
uint32_t  page_dir_addr =  PAGE_DIR_ADDR + KERNEL_V_START;                     
uint32_t  *page_dir =    (uint32_t*)  (PAGE_DIR_ADDR + KERNEL_V_START);     // max 4KB 1024*4MB = 4G
uint32_t  *page_table =  (uint32_t*) (0x101000 + KERNEL_V_START); // start from 1MB+4KB
#endif

uint32_t get_phy_address(uint32_t va);
int page_dirent_exists(uint32_t va);

// use reverse mapping to get pade directory address
uint32_t get_page_directory_pyhsical_address(){
    uint32_t * p = ( uint32_t *) 0xFFFFF000; 
    return (*p & 0xFFFFF000 );
}


#ifdef EARLY_INIT
void test_page(){
    //print_hex(aaa);
    printl("");
    uint32_t * p = ( uint32_t *)(0x9000+0x100000-4);
    //this will get page fault becuase an integer is 4 byte 
    //the last address will be  (0x9000+0x100000+2) > 0x9000+0x100000-1
    
    *p = 0x1234;
    print_hex(*p);

    //test reverse mapping
    uint32_t p1,p2,p3,p4,p5;
    p1 = get_phy_address(0x9030);
    p2 = get_phy_address(0x9000+0x100000-4);
    p3 = get_phy_address(0x90000+0x50);
    printl("");
    print_hex(p1);
    printl("");
    print_hex(p2);
    printl("");
    print_hex(p3);
    printl("");

    p4 = get_page_directory_pyhsical_address();
    printl("page dir address");
    print_hex(p4);
    printl("");
    //printl("page table address");

}
#endif

uint32_t get_phy_address(uint32_t va){
    uint32_t dir_idx = PAGE_DIR_INDEX(va);
    uint32_t pt_idx = PAGE_TABLE_INDEX(va);
    uint32_t offset = PAGE_OFFSET(va);
    uint32_t * pt = (uint32_t*) ( 0xFFC00000 + (dir_idx<<12) );

    return ( pt[pt_idx]&0xFFFFF000 ) + offset ;
}

void set_reverse_mapping(uint32_t dir_index,uint32_t table_address){

    uint32_t reverse_mapping_table = PAGE_TABLE_START(1023);

    if( ! (page_dir[1023] & PAGE_P)){
        page_dir[1023] =  reverse_mapping_table  | PAGE_P;
    }
    uint32_t *p = (uint32_t*) reverse_mapping_table;
    p[dir_index] = table_address | PAGE_P;  //read onlty and (kernel mode only ??)
}

int page_dirent_exists(uint32_t va){
    uint32_t dir_index  = PAGE_DIR_INDEX(va);
    return page_dir[dir_index] & PAGE_P;
}

// return : start address of page_table of va
uint32_t set_page_dirent(uint32_t va,uint8_t user){
    uint32_t dir_index  = PAGE_DIR_INDEX(va);
    uint32_t pt_base    = PAGE_TABLE_START(dir_index);
    page_dir[dir_index] = pt_base  | PAGE_P | PAGE_RW | PAGE_WT;
    if(user){
        page_dir[dir_index] = page_dir[dir_index] | PAGE_USER;
    }
    return pt_base;
}

// address is 4kb aligned
void allocate_page_tent(uint32_t va,uint32_t pa,uint8_t user){

    if(!PAGE_ALIGNED(pa)){
        printl("allocate_page_tent : not aligned");
        print_hex(pa);
        printl("");
        return;
    }
    
    uint32_t pt_base;

    if(!page_dirent_exists(va)){
        pt_base = set_page_dirent(va,user);
    }else{
        pt_base = PAGE_TABLE_START(PAGE_DIR_INDEX(va));
    }

    uint32_t pt_idx  = PAGE_TABLE_INDEX(va);
    uint32_t *pt_ent = (uint32_t*) pt_base;

    pt_ent+=pt_idx;
    *pt_ent = pa | PAGE_P | PAGE_RW | PAGE_WT;
    if(user){
        *pt_ent = *pt_ent | PAGE_USER;
    }

    set_reverse_mapping(PAGE_DIR_INDEX(va),pt_base);
}

#ifdef EARLY_INIT
void fill_zeros_page_dir(){
    uint32_t *p = page_dir;
    while(p < (uint32_t *)KERNEL_P_START ){
        *p = 0 ;
        ++p ;
    }
}
#endif


void switch_page_dir(uint32_t dir_addr){
    asm volatile ( 
                    "movl %0,%%eax \n\t"
                    "movl %%eax,%%cr3 \n\t"    
                    : 
                    : "r"(dir_addr)
                    :"eax"
    );

}


void enable_paging(){
    switch_page_dir((uint32_t)page_dir);
    asm volatile ( 
                    "movl %%cr0,%%eax \n\t"  
                    "orl $0x80000001,%%eax \n\t"  
                    "movl %%eax,%%cr0 \n\t"           
                    : 
                    :
                    :"eax"
    );
}


/*
    1.identity mapping first 1MB code (boot up code)
    2. 1MB ~ 8MB  is for page tables (kernel space) virtual address start from 3G
    3. 8MB ~ 16MB for kernel sections virtual address starts from 3G+8MB
    4. 16NB ~ MAX_MEM_SIZE for user  (user space so virtual address starts from 0)
*/
// higher kernel setting settings before enable page table
#ifdef EARLY_INIT
void setup_page_tables(){
    fill_zeros_page_dir();
    uint32_t physical_address = 0;

    //identity mapping first 1MB code (boot up code)
    while( physical_address <  _1MB  ){
        allocate_page_tent(physical_address,physical_address,0);
        physical_address+= FRAME_SIZE;
    }
    



    // 1MB ~ 8MB  is for page tables (kernel space) virtual address start from 3G
    //first 1MB is for GDT  + setup code
    while( physical_address <  KERNEL_P_START-_1MB ){
        // use identiy mapping first
        //allocate_page_tent(physical_address,physical_address,0);
        allocate_page_tent(physical_address+KERNEL_V_START,physical_address,0);
        physical_address+= FRAME_SIZE;
    }

     //  because higher kernel needs gdt and some other data strcuture in boot sector
    //  after patch table still needs a mapping to gdt so set first mapping in kernel space 
    print_hex(physical_address+KERNEL_V_START);

    int offset=0;
    while( physical_address <  KERNEL_P_START){
        // use identiy mapping first
        //allocate_page_tent(physical_address,physical_address,0);
        allocate_page_tent(physical_address+KERNEL_V_START,offset,0);
        offset+=FRAME_SIZE;
        physical_address+= FRAME_SIZE;
    }



    
   // 8MB ~ 16MB for kernel sections virtual address starts from 3G+8MB
    while( physical_address < USER_P_START ){
        // use identiy mapping first
        //allocate_page_tent(physical_address,physical_address,0);
        allocate_page_tent(physical_address+KERNEL_V_START,physical_address,0);
        physical_address+= FRAME_SIZE;
    }

    extern int _init_end;  
    printl("init_end");
    print_hex((int)&_init_end);
}

void init_page_settings(){
    setup_page_tables();
    // set_reverse_mapping(1023, (uint32_t)page_dir );
    enable_paging();
    
    test_page();
}
#else
void   patch_page_table_k(){
    int user_v_addr=0;
    uint32_t physical_address =  USER_P_START;
    // map first 1M 

    while( physical_address < MAX_MEM_SIZE ){
        allocate_page_tent(user_v_addr,physical_address,1);
        physical_address+= FRAME_SIZE;
        user_v_addr+=FRAME_SIZE;
    }
}
#endif

