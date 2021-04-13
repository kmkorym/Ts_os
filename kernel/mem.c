#include "mem.h"

#define PAGE_DIR_INDEX(addr)(addr>>22)
#define PAGE_TABLE_INDEX(addr)((addr>>12)&0x3FF)
#define PAGE_OFFSET(addr)(addr&(~0xFFFFF000))
#define PAGE_TABLE_START(dir_idx)((uint32_t)page_table+4096*dir_idx)
#define PAGE_ALIGNED(addr)(addr% FRAME_SIZE == 0)
#define KERNEL_LOAD_ADDR 0x9030
/*
#define KERNEL_MAX_END   0XB0000
*/
/*
assume our kernel in 1 MB
set up identity paging
*/
uint32_t  aaa  = 0x8787;                   
uint32_t  *page_dir =    (uint32_t*) 0x100000; // max 4KB 1024*4MB = 4G
uint32_t  *page_table =  (uint32_t*) 0x101000; // start from 1MB+4KB


uint32_t get_phy_address(uint32_t va);
int page_dirent_exists(uint32_t va);

uint32_t get_page_directory_pyhsical_address(){
    uint32_t * p = ( uint32_t *) 0xFFFFF000; 
    return (*p & 0xFFFFF000 );
}


void test_page(){
    print_hex(aaa);
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

void fill_zeros_page_dir(){
    uint32_t *p = page_dir;
    while(p<page_table ){
        *p = 0 ;
        ++p ;
    }

}


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

void init_page_settings(){
    fill_zeros_page_dir();
    uint32_t start_p = KERNEL_LOAD_ADDR & (~0x3FF);
    //init
    int i; // set 1 MB identity mapping
    for(i=0;i<(1024*1024/FRAME_SIZE);++i){
        //identity mapping
        allocate_page_tent(start_p,start_p,0);
        start_p+=FRAME_SIZE;
    }
    //reverse mapping for page ditectory
    printl("1234");
    set_reverse_mapping(1023, page_dir );
    enable_paging();
    test_page();
}

