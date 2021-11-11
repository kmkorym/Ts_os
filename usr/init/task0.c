#include "lib.h"
int i = 0 ;
void main(){
    int num=3;
    while(i<num){
        ++i;
        if(i<0){
            i = 0;
        }
        printl("I am task0 ");
        //printstr("I am task0 "); print_hex(i); printl(" ");
    }
    return;
}