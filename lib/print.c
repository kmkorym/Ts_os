// 1. print hex byte on screen
// 2. print characters and new line (move cursor)
// 3. print by memory address and constant
// 4. clear screen
#include "print.h"
#include "string.h"
#include "../kernel/mem.h"
#include "varg.h"

#define TEXT_COLOR  0xF
#define BG_COLOR 0x0
#ifdef EARLY_INIT
#define VGA_BASE  0XB8000
#else
#define VGA_BASE  0XB8000+KERNEL_V_START
#endif
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

void print_screen(char c,unsigned int x,unsigned int y){
    if(x>=VGA_MAX_COL){
        x = VGA_MAX_COL-1;
    }
    if(y>=VGA_MAX_ROW ){
        y = VGA_MAX_ROW -1;
    }
    char* p = (char*) __vga_addr(x,y);
    *p=c;
}


void back_char(){
    if(current_x>0){
        print_screen(0,--current_x,current_y);
    }
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

void print_chars(char * s,int n){
    int i;
    for(i=0;i<n;++i){
       print_char(s[i]); 
    }
}


void printstr(const char * s){
    while(*s){
        print_char(*s++);
    }
}

void printl(char * s){
    printstr(s);
    print_char('\n');
}


char hb_to_hex(unsigned char * b){ 
    char c;
    if((*b>>4)<=9){
        c = (*b>>4)+'0';
    }else{
        c = (*b>>4)-(unsigned char)10+'A';
    }
    return c;
}

char lb_to_hex(unsigned char * b){
    char c; 
    if((*b&0x0F) <= 9){
        c = (*b&0x0F)+'0';
    }else{
        c = (*b&0x0F)-(unsigned char)10+'A';
    }
    return c;
}


void _print_byte(unsigned char * b){
    print_char(hb_to_hex(b));
    print_char(lb_to_hex(b));
}


void print_byte(unsigned char x){
    print_char('0');
    print_char('x');
    _print_byte(&x);
}

// it acutally interpret x as unsigned though
void print_hex(int x){
    unsigned char * a = (unsigned char *) &x;
    unsigned char * b = a+3;
    print_char('0');
    print_char('x');
    while(b >=a){
        //first 4 byte
        _print_byte(b);
        //little endian
        --b;
    }

}


void sprint_hex(int x,char *buf){
    
    int cnt = 0; 
    unsigned char * a = (unsigned char *) &x;
    unsigned char * b = a+3;
    
    while(b >=a){
        buf[cnt++] = hb_to_hex(b);
        buf[cnt++] = lb_to_hex(b);
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

/*
 * num format has limited padding size <= 32
 * fmt : 02, 11 (number) before d for %02d %5d .. 
 *       0 for padding zero  
 *       digits after 0 represents the max length of output
 *       a single 0 means directly print the number
 */

void _print_num(const char * fmt,char num_type,int num){
    
    char buf[33] = {0};
    int  zero_flag = 0;
    int  num_str_len = -1;
   
    const char * p = fmt;
    
    if(!*p || (*p == '0' && ! *(p+1))){
        goto output_num ;
    }

    while(*p){
        if(!is_num(*p)){
            printl("error print num format");
            goto output_num;
        }
        ++p;
    }

    
    if(*fmt == '0' ){
        zero_flag = 1;
    }

    num_str_len = atoi(fmt+zero_flag,10);

     
    output_num:
    if(num_type == 'x'){ //hex
        print_char('0');
        print_char('x');
        sprint_hex(num,buf);
    }else{ // digiit , not implement , use hex format
        itoa(num,buf,33);
    }

    // ignores leading zeros
    p = buf;
    
    while( *(p+1) && *p == '0'){
        ++p;
    }
    

    if(num_str_len == -1){
        printstr(p);
        return ;
    }
    
    int num_len =  strlen(p);
    int len_diff =  num_str_len  - num_len;

    if(len_diff > 0 ){
        if(zero_flag){
                while(len_diff--){print_char('0');}
        }
    }else{
        p = p + num_len -num_str_len;
    }

    printstr(p);
        
}

char * _print_arg_handler(char* fmt,char **va_list){
   
    char buf[32] = {0};
    char *p = fmt;
    char offset = 0;

    while( *p && !is_alpha(*p) ){ ++p ; }

    if(!*p){
        goto _print_fmt_err;
    }
    
    
    switch(*p){
        case 'c':
            print_char(**va_list);
            *va_list = va_next(*va_list,char);
            return fmt+(p-fmt)+1;
        case 'x':
        case 'd':
            memcpy(fmt,buf,p-fmt);
            buf[p-fmt] = 0; 
            _print_num(buf,*p,*((int*)(*va_list)));
            *va_list = va_next(*va_list,int);
            return fmt+(p-fmt) +1  ;
        case 's':
            printstr( *((char**)(*va_list)));
            *va_list = va_next(*va_list,char*);    
            return  fmt+(p-fmt) +1 ;
        default:
            break;
    }

    _print_fmt_err:
    return NULL;
}



void printf(char *fmt,...){
    
    if(!fmt){
        return ;
    }

    char * va_list = va_start(&fmt,char*);
    char * ptr_fmt = fmt;
    
    while(*ptr_fmt){
        switch(*ptr_fmt){
            case '%' :
                if( (ptr_fmt=_print_arg_handler(++ptr_fmt,&va_list)) == NULL ){
                    printl("printf format error!");
                    return;
                }
                continue;
            default : 
                print_char(*ptr_fmt++);
                continue;

        }
    }

    return ;
}

