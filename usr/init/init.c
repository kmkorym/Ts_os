#include "lib.h"
char * s = "start init hello world";
int main(){

    // ring 3 level test  
    //asm volatile("cli"); // this privileged instruction will trigger #GP
    // this will access kernel address space from user space, must trigger #GP
    //char * p = (char*)0xC0000000;
    //*p = 3;
    //sleep
    int i = 0 ;
    int ret = foo(4, 5);
    while(1){
        i = 0;
        
        while(i<0x8FFFFFF){++i;};
        printl(s);
    }
    //while(1){}
    return 0;
}