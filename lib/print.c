#include "print.h"
#include "string.h"
#include "../kernel/mem.h"
#include "../kernel/console.h"
#include "varg.h"

void clear(){
    console_clear();
}


void print_char(char c){
    write_console_one(c);
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

