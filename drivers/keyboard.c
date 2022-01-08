#include "../lib/print.h"
#include "../lib/string.h"
#include "../kernel/common.h"
#include "../kernel/shell.h"
#include "keyboard.h"
#define KB_OUT_STATUS_OK 0x1

uint8_t scancode_history[6] = {0};
uint32_t scancode_depth  =   0 ;



/*
    it seems like qemu default setting is to translate scancode set 2 to scan code set 1 
    so construct the table of scan code set 1
*/

/*
    if scan code is key board position of 0-9 A-Z a-z , use ascii code as key code
*/

// scan code set 1 to key code
KEY_CODE keycode_tbl[] = { EMPTY,EMPTY,ONE,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE,ZERO,HYPHEN, 
    EQU,BACKSPACE ,TAB , Q,W, E,R, T, Y, U,I,O, P,LEFT_BRACKET,RIGHT_BRACKET,ENTER,CTRL,A,S,D,F,G,H,J,
    K,L,SEMICOLON, SINGLE_QUOTE, ACUTE,SHIFT,BACK_SLASH, Z, X,C, V, B, N, M, COMMA,DOT,
    SLASH, SHIFT, WILDCARD, ALT,SPACE,CAPS,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLOCK,SCROLL,SEVEN,EIGHT,
    NINE,HYPHEN,FOUR,FIVE,SIX,ADD,ONE,TWO,THREE,ZERO,DOT,F11,F12};


//static char kb_buffer[MAX_BUFFER+1]={0};
//static uint8_t bf_len = 0;
//static uint8_t shift_pressed = 0;

/*
char char_with_case(uint8_t keycode){
    char c;
    c = keycode_tbl[keycode];
    if(shift_pressed){
        return c;
    }
    if(is_alpha(c)){
        c = c - 'A'+'a';
    }
    return c;
}
*/

void init_kb(){

}

void kb_handler(){

    uint8_t status ;
    uint8_t scan_code ; 
    KEY_CODE key_code = EMPTY ; 

    // polling check , it shoud alwayys pass ! 
    while( ! ( ( status=inb(0x64) ) & KB_OUT_STATUS_OK ) );
    scan_code = inb(0x60);

    switch(scancode_depth){
        case 0 :
            switch(scan_code){
                case 0 ... sizeof(keycode_tbl)/sizeof(KEY_CODE):          
                    key_code = keycode_tbl[scan_code];
                break;
                case 0xE0:
                case 0xE1:
                    goto _put_scan_history;
            }
            break;

        case 1:
            
            if(scancode_history[0] == 0xE1){
                goto _put_scan_history ; 
            }

            switch (scan_code){
                case 0xC8:
                    key_code = UP_RELEASE;
                    break;
                case 0xD0:
                    key_code = DOWN_RELEASE;
                    break;
                case 0x2A:
                case 0xB7:
                    goto _put_scan_history ;
            }
            break;

        case 2:
            goto _put_scan_history ;
        case 3:
            
            if(scancode_history[0] == 0xE0 && scancode_history[1] == 0x2A ){
                key_code = PRINTSCR;
                break;    
            }

            if(scancode_history[0] == 0xE0 && scancode_history[1] == 0xB7 ){
                key_code = PRINTSCR_RELEASE;
                break;    
            }
            // for pause
            goto _put_scan_history ;
        case 4:
            goto _put_scan_history ;
        case 5:
            key_code = PAUSE;
            break;  

    }

    
    scancode_depth = 0;
    shell_get_key_code(key_code);
    return ;

    _put_scan_history:
    scancode_history[scancode_depth++] = scan_code;
    return ;
}



