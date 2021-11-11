#include "lib.h"

int i = 0 ;
void main(){
    unsigned int i=0;
    while(1){
        //sleep
        while(i<0x2FFFFFF){++i;};
        printl("I am task2");
        i=0;
        //printstr("I am task1 ~~~~~ "); print_hex(i); printl(" ");
    }
    return;
}