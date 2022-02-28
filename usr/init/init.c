#include "lib.h"
char * s = "start init hello world";
int main(){

    // ring 3 level test  
    //asm volatile("cli"); // this privileged instruction will trigger #GP
    // this will access kernel address space from user space, must trigger #GP
    //char * p = (char*)0xC0000000;
    //*p = 3;
    printl(s);
    //while(1){}
    return 0;
}