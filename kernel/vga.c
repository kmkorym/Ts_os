#include "vga.h"

static VGA_PARAM vga_param;


/*
    TODO : init vga parameter by probe BIOS/hardware setting 
*/
const VGA_PARAM*  vga_init(){
    vga_param.max_col = VGA_MAX_COL;
    vga_param.max_row = VGA_MAX_ROW;
    return &vga_param;
}


unsigned int  __vga_addr(unsigned int x,unsigned int y){
    return  VGA_BASE+(y * vga_param.max_col + x) * 2;
}


void vga_print_row(unsigned int y, const char * row){
    
    int x  = VGA_MAX_COL-1;

    while( x >= 0 ){
        vga_write_char(row[x],x,y);
        --x;
    }

}


int  vga_write_char(char c,unsigned int x,unsigned int y){
    
    if( x< 0  || x >= vga_param.max_col){
        return -1;
    }

    if( y< 0  || y >= vga_param.max_row){
        return -1;
    }

    char* p = (char*) __vga_addr(x,y);
    *p = c;

    return 0;
}



