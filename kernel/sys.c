
#include "sys.h"
#include "common.h"
#include "./task.h"

DEFINE_SYSCALL0(SYS_EXIT,exit)
DEFINE_SYSCALL1(SYS_PRINTL,printl,p1)
DEFINE_SYSCALL2(SYS_FOO,foo,p1,p2)


void* syscall_table[SYSCALL_NUM]={0};


int __foo(int a,int b){
    return a+b;
}


void init_syscall(){
    syscall_table[SYS_PRINTL]=printl;
    syscall_table[SYS_FOO]=__foo;
}


int register_syscall(int nr,void* func){
    syscall_table[nr]=func;
    return 0;
}

