#ifdef EARLY_INIT
#define VGA_BASE  0XB8000
#else
#include "mem.h"
#define VGA_BASE  0XB8000+KERNEL_V_START
#endif

#include "common.h"

#define VGA_MAX_COL 80
#define VGA_MAX_ROW 25


struct VGA_PARAM{
    uint32_t max_col;
    uint32_t max_row;
};
typedef struct VGA_PARAM VGA_PARAM;


const VGA_PARAM*  vga_init();
int  vga_write_char(char c,unsigned int x,unsigned int y);
void vga_print_row(unsigned int y, const char * row);