#include "lib.h"
#include <string.h>


int  exit(){
    return sys_exit();

}

int  printl(char*s ){
    return sys_printl(s);
}

int  foo(int a,int b){
    return sys_foo(a,b);
}