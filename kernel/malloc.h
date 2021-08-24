#include "common.h"
#include "mem.h"
#include "../lib/print.h"


#define HEAP_END  KERNEL_V_START+(16*_1MB)
#define REGION_FREE  0
#define REGION_ALLOC 1



struct RegionHeader{            
    unsigned char  type                ; //  0 : free , 1 : allocated
    unsigned char  magic               ; // 0x8787
    uint32_t       size                ; // size of region, not contains header
    struct RegionHeader * link         ; // when type is allocated , link point to the header address of previous region , this is for free(addr) unification
                                         // when type is free , link point to  the next greater size free region header  , this is for alloacted to find the most matched
                                         // region to malloc(size), the  link between free header will form a sorted linked list of free region for smaller size to larger size
};



void init_heap();
void test_heap();
void add_alloc_region(struct RegionHeader* addr);
void del_alloc_region(struct RegionHeader* addr);
void add_free_region(struct RegionHeader * prev,struct RegionHeader * next, struct RegionHeader * new);
void del_free_region(struct RegionHeader * prev,struct RegionHeader *  deleted);
void search_and_add_free_region(struct RegionHeader * new);
uint32_t* kmalloc(uint32_t size);
void  kfree (uint32_t* addr);