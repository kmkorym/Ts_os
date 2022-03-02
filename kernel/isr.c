#include "../lib/print.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "../drivers/com.h"
#include "./common.h"
#include "./task.h"

#define IRQ_MAX 63
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)


void  init_devices(){
    init_timer();
    init_kb();
    init_serial();

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



extern uint32_t IDT_TABLE_DESC; // note  idt_ptr_struct*  is wrong
struct IDTDesc idt[IRQ_MAX+1];

void IRQ_set_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 40) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 32;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}

void config_pic(){
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
}



void page_fault_handler(uint32_t err_code){
    printl("error code");
    print_hex(err_code);
    printl("");

    uint32_t cr2;

    asm volatile("movl %%cr2, %0": "=r"(cr2));
    printl("address:");
    print_hex(cr2);
    printl("");
    while(1){};
    


}



void  irq_handler_entry( uint32_t irq, uint32_t err_code){
    //print_hex();

    if( irq == 13 ){
        printstr("handler # ");
        print_hex(irq);
        printl("");
        printstr("err # ");
        print_hex(err_code);
        while(1){};
    }

    if (irq!=32 && irq!=33  ){
        printstr("handler #");
        print_hex(irq);
        printl("");

    }
    
    switch(irq){
        case 14:
            page_fault_handler(err_code);
            break;
        case 32:
            timer_handler();
            break;
        case 33:
            kb_handler();
            break;
        case 36:
            com1_handler();
            break;
        case 63:
            terminate_process();
            break;
    }
    // EOI
    if (irq >= 40){
         outb(0xA0, 0x20);
    }
    if(irq>=32){
        // must slave first..
        // https://wiki.osdev.org/User:Johnburger/PIC
        // Send reset signal to master. (As well as slave, if necessary).
        outb(0x20, 0x20);
    }

    cond_schedule();
}

void init_trap_gate(uint8_t irq,uint32_t  handler_offset,uint16_t selector,uint8_t flag){   
    idt[irq].offset_1 = handler_offset & 0xFFFF;
    idt[irq].offset_2 = (handler_offset >> 16) & 0xFFFF;
    idt[irq].selector = selector;
    idt[irq].zero = 0;
    idt[irq].type_attr = flag; 
}

#define INIT_IRQ(n) \
extern void isr##n();\
init_trap_gate((n),(uint32_t)isr##n,0x08,0x8F);

void init_idt(){

    struct idt_ptr_struct* idt_desc =  (struct idt_ptr_struct*) &IDT_TABLE_DESC ;

    config_pic();
    //IRQ_set_mask(32);
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
    // DPL for system call must be 3
    idt[63].type_attr = 0xEF; ; 
    idt_desc->limit = sizeof(struct IDTDesc)*( IRQ_MAX+1)-1;
    idt_desc->base =  (uint32_t)&idt;

    // load idt
    asm volatile (
            "lidt (%%eax)\n\t"
            :
            : "a"(&IDT_TABLE_DESC)
            : 

    );  

    return;
}

