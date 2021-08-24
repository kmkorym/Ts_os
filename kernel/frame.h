#include "../lib/print.h"
#include "common.h"
#include "mem.h"

void test_frame();
uint8_t  frame_status(uint32_t addr);
uint32_t alloc_frame(uint32_t min_addr,uint32_t max_addr);
void  clear_frame(uint32_t addr);
uint32_t get_idle_frame_addr(uint32_t min_addr,uint32_t max_addr);