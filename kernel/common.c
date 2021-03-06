#include "common.h"
#include "../lib/print.h"




void panic(char*s){
    printstr("Panic: ");
    printstr(s);
    printl("");
    while(1){}
}


int max(int a,int b){
    return a>b?a:b;
}

int min(int a,int b){
    return a<b?a:b;
}


void outw(uint16_t port, uint16_t value)
{
    asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

void outb(uint16_t port, uint8_t value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port)
{
   uint8_t ret;
   asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

uint16_t inw(uint16_t port)
{
   uint16_t  ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}


int first_set_bit(uint8_t* addr){
    int i = 0 ;
    int pat = 1;
    while(i<8){
        if(pat & *addr){
            return i;
        }
        pat = pat << 1;
        ++i;
    }
    return -1;
}

int first_clr_bit(uint8_t* addr){
    int i = 0 ;
    int pat = 1;
    while(i<8){
        if(!(pat & *addr)){
            return i;
        }
        pat = pat << 1;
        ++i;
    }
    return -1;
}


