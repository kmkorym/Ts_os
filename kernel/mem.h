#include "../lib/print.h"
#include "common.h"
#define PAGE_P 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4
#define PAGE_WT 0x8
#define PAGE_DIRTY 0x80
#define FRAME_SIZE 4096
void init_page_settings();
void setup_page_tables();
