#define SYSCALL_NUM 64
#define SYS_EXIT 0
#define SYS_PRINTL 0x10
#define SYS_FOO 0x20


int sys_exit();
int sys_printl();
int sys_foo();

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



#define DEFINE_SYSCALL2(num,name,p1,p2)\
int sys_##name(int p1,int p2){\
    int ret;\
    asm volatile("int $0x3f" : "=a"(ret): "a" (num),"b" (p1),"c" (p2):); \
    return ret;\
}\


void init_syscall();
int  register_syscall(int nr,void* func);








