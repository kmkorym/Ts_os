// 1. print hex byte on screen
// 2. print characters and new line (move cursor)
// 3. print by memory address and constant
// 4. clear screen
#include "print.h"

#define TEXT_COLOR  0xF
#define BG_COLOR 0x0
#define VGA_BASE  0XB8000
#define VGA_MAX_COL 80
#define VGA_MAX_ROW 25

static unsigned int current_x=0;
static unsigned int current_y=0;


void set_cursor(unsigned int x,unsigned int y){
    current_x = x;
    current_y =y;
}

unsigned int  __vga_addr(unsigned int x,unsigned int y){
    return  VGA_BASE+(y * VGA_MAX_COL + x) * 2;
}

unsigned int   __current_vga_addr(){
    return  __vga_addr(current_x,current_y);
}

void __move_cursor_next_line(){
    current_x=0;
     ++current_y;
    if(current_y==VGA_MAX_ROW){
        current_y=0;
    }
}

void __move_cursor_next(){
    ++current_x;
    if(current_x==VGA_MAX_COL){
        current_x=0;
         __move_cursor_next_line();
    }
}

void show_prompt(){
}

void print_char(char c){
    if(c=='\n'){
        __move_cursor_next_line();
        if(current_y==0){
            clear();
        }
        return;
    }
    char* p = (char*) __current_vga_addr();
    *p=c;
    __move_cursor_next();
       
    if(current_x ==0 && current_y==0){
        clear();
    }

    show_prompt();
}


void printstr(char * s){
    while(*s){
        print_char(*s++);
    }
}

void printl(char * s){
    printstr(s);
    print_char('\n');
}


// it acutally interpret x as unsigned though
void print_hex(int x){
    unsigned char * a = (unsigned char *) &x;
    unsigned char * b = a+3;
    print_char('0');
    print_char('x');
    while(b >=a){
        //first 4 byte
        if((*b>>4)<=9){
            print_char((*b>>4)+'0');
        }else{
            print_char((*b>>4)-(unsigned char)10+'A');
        }
        //print_char((*b&0x0F)+(char)0x30);
        if((*b&0x0F) <= 9){
            print_char((*b&0x0F)+'0');
        }else{
            print_char((*b&0x0F)-(unsigned char)10+'A');
        }
        
        //little endian
        --b;
    }

}
void clear(){
    int i,j;
    char* p;
    for(i=0;i<VGA_MAX_ROW;++i){
        for(j=0;j<VGA_MAX_COL;++j){
            p=(char*) __vga_addr(j,i);
            *p=0;
        }
    }
    set_cursor(0,0);
}




