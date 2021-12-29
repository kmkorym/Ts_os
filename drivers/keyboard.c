#include "../lib/print.h"
#include "../lib/string.h"
#include "../kernel/common.h"
#include "../kernel/shell.h"

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define LEFT_SHIFT 0x2A
#define MAX_BUFFER 256
#define SC_MAX 0x39


const char sc_ascii[] = { 0,0, '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '=', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 
        'U', 'I', 'O', 'P', '[', ']', 0, 0, 'A', 'S', 'D', 'F', 'G', 
        'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

static char kb_buffer[MAX_BUFFER+1]={0};
static uint8_t bf_len = 0;
static uint8_t shift_pressed = 0;




void clear_buffer(){
    bf_len = 0;
    kb_buffer[bf_len] = 0;
}

char char_with_case(uint8_t keycode){
    char c;
    c = sc_ascii[keycode];
    if(shift_pressed){
        return c;
    }
    if(is_alpha(c)){
        c = c - 'A'+'a';
    }
    return c;
}

void  put_buffer(char c){
    if(bf_len==MAX_BUFFER-1){
        printl("");
        printl("Too many user input");
        clear_buffer();
        return;
    }
    kb_buffer[bf_len++] = c;
    kb_buffer[bf_len] = 0;
}

char pop_buffer(){
    if(!bf_len){
        kb_buffer[0]=0;
        return 0;
    }
    bf_len--;
    kb_buffer[bf_len]=0;
    return kb_buffer[bf_len];
}


void init_kb(){

}

void kb_handler(){
    uint8_t status = inb(0x64);
    uint8_t keycode = inb(0x60);
    char c;
    if(keycode >SC_MAX){
        return;
    }else if(keycode ==  LEFT_SHIFT){
        
        if(shift_pressed){
            shift_pressed = 0 ;
            //print_screen(0,79,0);
        }else{
            shift_pressed = 1;
           // print_screen('I',79,0);
        }
        
    }else if(keycode ==  BACKSPACE){
        pop_buffer();
       // back_char();
    }else if(keycode ==  ENTER){
        print_char('\n');
        parse_command(kb_buffer);
        clear_buffer();
    }else{
        c = char_with_case(keycode);
        if(c){
            print_char(c);
            put_buffer(c);
        }
    }
}



