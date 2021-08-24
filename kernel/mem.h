#include "../lib/print.h"
#include "common.h"
#define PAGE_P 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4
#define PAGE_WT 0x8
#define PAGE_DIRTY 0x80
#define FRAME_SIZE 4096

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


#define KERNEL_HEAP_START  0xC0F00000
#define KERNEL_HEAP_END    0xC1000000
#define PRINT_ADDR(addr)\
    print_hex((int)addr);\
    printl("");\



#ifdef EARLY_INIT
void init_page_settings();
void setup_page_tables();
#else
void patch_page_table_k();
#endif


int get_frame_address(uint32_t va,uint32_t  *phy_addr);
void allocate_page_tent(uint32_t va,uint32_t pa,uint8_t user);
