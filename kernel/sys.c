
#include "sys.h"
#include "common.h"
#include "./task.h"

DEFINE_SYSCALL0(SYS_EXIT,exit)
DEFINE_SYSCALL1(SYS_PRINTL,printl,p1)


void* syscall_table[SYSCALL_NUM]={0};



void init_syscall(){
    syscall_table[SYS_PRINTL]=printl;
}


int register_syscall(int nr,void* func){
    syscall_table[nr]=func;
    return 0;
}

