#define SYSCALL_NUM 64
#define SYS_EXIT 0
#define SYS_PRINTL 0x10


int sys_exit();
int sys_printl();


#define DEFINE_SYSCALL0(num,name)\
int sys_##name(){\
    int ret;\
    asm volatile("int $0x3f" : "=a"(ret): "a" (num):); \
    return ret;\
}\

#define DEFINE_SYSCALL1(num,name,p1)\
int sys_##name(int p1){\
    int ret;\
    asm volatile("int $0x3f" : "=a"(ret): "a" (num),"b" (p1):); \
    return ret;\
}\



#define SYSCALL2(num,p1,p2)\
    asm volatile("int $0x3f" : : "a" (num),"b" (p1),"c" (p2):); \
}

#define SYSCALL3(num,p1,p2,p3)\
    asm volatile("int $0x3f" : : "a" (num),"b" (p1),"c" (p2),"d" (p3):); \
}








