#include "shell.h"
#include "../lib/print.h"
#include "../lib/string.h"
#include "../kernel/common.h"
#include "../drivers/com.h"
#include "task.h"
#include "console.h"
#define DONWCASE   0
#define UPCASE 1

static uint32_t __case  = UPCASE; 

// define commnads can execute when user type enter
// halt -> quit computer
// hello --> print hello from kernel
// clear --> clear console and keyboard buffer
// sp  load mem_addr
// sp print
//  execute mem_addr
void halt(){
    /*
    asm volatile (  "movb %%al, 0xFE\n\t"
                    "outb %%al,$0x64\n\t"
                    : 
                    : 
                    );
    */
    asm volatile (  "hlt\n\t"
                : 
                : 
                );
}

void parse_serial_command(char* s){
    char buffer[256];
    s = strtok(s," ",buffer); 
    if(string_equal("print",buffer)){
        printl("execute sp print");
    }else if(string_equal("load",buffer)){
        /*
        uint32_t addr=0;
        uint32_t cnt=0;
        int res=0;
        s = strtok(s," ",buffer);
        printl(buffer);
        addr = atoi(buffer,16);
        printstr("load com1: ");
        print_hex(addr);
        printl("");
        serial_file_transfer(addr);
        struct simple_task task;
        task.task_id = 33;
        task.esp = 0x50000;
        task.ebp = 0x50000;
        task.eip = addr;
        add_task(task);
        */

    }else if(string_equal("test",buffer)){
        printl("com1 test");
        uint32_t total = serial_file_transfer(0x50000);
        uint32_t i = 0 ;
        uint8_t* p = (uint8_t*) 0x50000;
        for(i=0;i<total&& i<32;++i){
            print_hex(*p);
            print_char(' ');
            ++p;
        }
    }
    
}



void  parse_command(char*s){
    char buffer[256];
    char *p;
    if(string_equal("hello",s)){
        printl("hello from kernel");
    }else if(string_equal("halt",s)){
        printl("halt ...");
        halt();
    }else if(string_equal("clear",s)){
        clear();
    }else if(string_equal("nt",s)){
        printl("switch to next task");
        extern struct Task *current;
        current->state|=TASK_NEED_SCHED;
    }else{
        // parse command
        p = s;
        p = strtok(p," ",buffer); 
        if(string_equal("sp",buffer)){
            parse_serial_command(p);
        }
    }

}


void shell_get_key_code(KEY_CODE keycode){

    char c = (char)keycode;

    switch (keycode){
        case EMPTY:
            break;
        case UP_RELEASE:
            window_move_up();
            break;
        case DOWN_RELEASE:
            window_move_down();
            break;  
        case A ... Z: // pass through
            if(__case  == DONWCASE){
                c = c - 'A' + 'a';
            }
             //... AT   :  // pass through
        case ZERO ... NINE: 
        case HYPHEN   : case           EQU: case LEFT_BRACKET: case RIGHT_BRACKET:
        case SEMICOLON: case  SINGLE_QUOTE: case ACUTE       : case BACK_SLASH:
        case COMMA:     case  DOT:          case SLASH:        case WILDCARD: case SPACE:  
            get_input_char(c);
            break;
        case ENTER:
            flush_input_buffer();
            break;
        case BACKSPACE:
            modify_input_cnt(-1);
            break;
        case PRINTSCR:
            printf("printscr detected\n");
            break;
        case PRINTSCR_RELEASE:
             printf("printscr_re detected\n");
            break;
        case PAUSE:
             printf("pause detected\n");
            break;
    }

    

}