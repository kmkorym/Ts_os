typedef unsigned int uint32_t;
typedef unsigned short  uint16_t;
typedef unsigned char uint8_t ;

#define EOF -1
#define NULL 0
#define _1MB  0x100000
#define MAX_MEM_SIZE (128*_1MB)

#define ASSERT(cond,name)\
    if(!(cond)){\
        printl( #name " failed!" );\
        printl( #cond " is False ") ;\
        while(1){}\
    }\

int max(int a,int b);
int min(int a,int b);
int first_set_bit(uint8_t* addr);
int first_clr_bit(uint8_t* addr);
void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void panic(char*s);