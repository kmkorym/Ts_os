#ifndef _MEMH_
#define _MEMH_

#include "../lib/print.h"
#include "common.h"
#define PAGE_P 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4
#define PAGE_WT 0x8
#define PAGE_DIRTY 0x80
#define PAGE_FLG_USR  (PAGE_P | PAGE_RW | PAGE_WT | PAGE_USER)
#define PAGE_FLG_KERNEL  (PAGE_P | PAGE_RW | PAGE_WT)
#define PAGE_TABLE_PTR(n)( (uint32_t*)(0xFFC<<20|(n<<12)) )
#define PAGE_ENTRY_SET(entry)(entry & PAGE_P)
#define IX_TO_VA(di,ti)( ((di)<<22 | (ti)<<12))
#define REV_TB_VA (0XFFFFFFFC)
#define FRAME_SIZE 4096
#define PG_DIR0_ADDR    (_1MB) 


#define PAGE_DIR_INDEX(addr)(addr>>22)
#define PAGE_TABLE_INDEX(addr)((addr>>12)&0x3FF)
#define PAGE_OFFSET(addr)(addr&(~0xFFFFF000))
#define PAGE_ALIGNED(addr)(addr% FRAME_SIZE == 0)
#define KERNEL_LOAD_ADDR 0x9030
#define KERNEL_V_START   0xC0000000
#define KERNEL_P_START  (8*_1MB)
#define KERNEL_VMAP_SIZE 0x800000
#define MAX_MEM_SIZE  (128*_1MB)

#define USER_P_START  (16*_1MB)
#define USER_V_STACK  (0xC0000000-4)

#define KERNEL_HEAP_END    0xC1000000
#define PRINT_ADDR(addr)\
    print_hex((int)addr);\
    printl("");\



#ifdef EARLY_INIT

void init_page_settings();
void setup_page_tables();
#else
void patch_page_table_k();
void test_page();
#endif

#define VA_MAP_NULL 0
#define VA_MAP_TEMP 1
#define VA_MAP_NEW  2
#define VA_MAP_UNK  3




void re_init_pg_dir0();
int init_kernel_dir();
void memzero(void*,uint32_t);
uint32_t   pt_phy_from_revmap(uint32_t* dir,uint32_t nb);
uint32_t*  create_temp_va(uint32_t pa);
uint32_t* create_page_table(uint32_t* dir,uint32_t* rev_tb,uint32_t va,uint32_t flag);
int pg_dir_no_entry(uint32_t* dir,uint32_t va);
void delete_temp_va(uint32_t va);
uint32_t*  alloc_page(uint32_t start_phy,uint32_t end_phy,uint32_t* phy);
uint32_t*  get_temp_va(uint32_t pa);
uint32_t * lookup_pt_temp_va(uint32_t* dir,uint32_t di);
uint32_t get_phy_from_dir(uint32_t *dir,uint32_t va);
int  pg_dir_add(uint32_t* dir,uint32_t *rev_tb,uint32_t di,uint32_t pt_phy,uint32_t flag);
int request_region_vmap(uint32_t *dir,uint32_t va_start,uint32_t size,uint32_t flag);
#endif
