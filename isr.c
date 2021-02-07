#include "print.h"

#define IRQ_MAX 63

typedef unsigned int uint32_t;
typedef unsigned short  uint16_t;
typedef unsigned char uint8_t ;


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



struct idt_ptr_struct
{
    uint16_t limit;
    uint32_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));


struct IDTDesc {
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
}__attribute__((packed));



extern struct idt_ptr_struct IDT_TABLE_DESC; // note  idt_ptr_struct*  is wrong
struct IDTDesc idt[IRQ_MAX+1];
int cnt=1;


void  irq_handler_entry( uint32_t irq, uint32_t err_code){
    printl("Int");
    print_hex(irq);
    printl("");
    printl("Err code");
    print_hex(err_code);
    printl("");
    cnt+=1;
    if(cnt>=10){
        while(1);
    }

}

void init_trap_gate(uint8_t irq,uint32_t  handler_offset,uint16_t selector){   
    idt[irq].offset_1 = handler_offset & 0xFFFF;
    idt[irq].offset_2 = (handler_offset >> 16) & 0xFFFF;
    idt[irq].selector = selector;
    idt[irq].zero = 0;
    idt[irq].type_attr = 0x8E;
    
}

#define INIT_IRQ(n) \
extern void isr##n();\
init_trap_gate((n),(uint32_t)isr##n,0x08);

void init_idt(){

    //remap interrupt
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);


    INIT_IRQ(0);
    INIT_IRQ(1);
    INIT_IRQ(2);
    INIT_IRQ(3);
    INIT_IRQ(4);
    INIT_IRQ(5);
    INIT_IRQ(6);
    INIT_IRQ(7);
    INIT_IRQ(8);
    INIT_IRQ(9);
    INIT_IRQ(10);
    INIT_IRQ(11);
    INIT_IRQ(12);
    INIT_IRQ(13);
    INIT_IRQ(14);
    INIT_IRQ(15);
    INIT_IRQ(16);
    INIT_IRQ(17);
    INIT_IRQ(18);
    INIT_IRQ(19);
    INIT_IRQ(20);
    INIT_IRQ(21);
    INIT_IRQ(22);
    INIT_IRQ(23);
    INIT_IRQ(24);
    INIT_IRQ(25);
    INIT_IRQ(26);
    INIT_IRQ(27);
    INIT_IRQ(28);
    INIT_IRQ(29);
    INIT_IRQ(30);
    INIT_IRQ(31);
    
    INIT_IRQ(32);
    INIT_IRQ(33);
    INIT_IRQ(34);
    INIT_IRQ(35);
    INIT_IRQ(36);
    INIT_IRQ(37);
    INIT_IRQ(38);
    INIT_IRQ(39);
    INIT_IRQ(40);
    INIT_IRQ(41);
    INIT_IRQ(42);
    INIT_IRQ(43);
    INIT_IRQ(44);
    INIT_IRQ(45);
    INIT_IRQ(46);
    INIT_IRQ(47);
    INIT_IRQ(48);
    INIT_IRQ(49);
    INIT_IRQ(50);
    INIT_IRQ(51);
    INIT_IRQ(52);
    INIT_IRQ(53);
    INIT_IRQ(54);
    INIT_IRQ(55);
    INIT_IRQ(56);
    INIT_IRQ(57);
    INIT_IRQ(58);
    INIT_IRQ(59);
    INIT_IRQ(60);
    INIT_IRQ(61);
    INIT_IRQ(62);
    INIT_IRQ(63); 
    IDT_TABLE_DESC.limit = sizeof(struct IDTDesc)*( IRQ_MAX+1)-1;
    IDT_TABLE_DESC.base =  (uint32_t)&idt;
    return;
}



    // INIT_IRQ(0);
 


    // IDT_TABLE_DESC.limit = sizeof(struct IDTDesc)*( IRQ_MAX+1)-1;
    // IDT_TABLE_DESC.base =  (uint32_t)&idt;
    //return;