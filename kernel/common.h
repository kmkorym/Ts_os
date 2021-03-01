typedef unsigned int uint32_t;
typedef unsigned short  uint16_t;
typedef unsigned char uint8_t ;

#define EOF -1
#define NULL 0


void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);