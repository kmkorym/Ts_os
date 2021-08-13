#include "com.h"
#include "../lib/string.h"
#include "../kernel/task.h"

#define PORT 0x3f8          // COM1
 

 //current implementation using polling

static char*  __buffer=0;
static uint32_t __cnt=0;
static uint32_t __buf_size=0;
static uint8_t __read_done=0;




static uint8_t put_char_to_buffer(char c){

}

void com1_handler() {
   char c = read_serial();
   if(0>put_char_to_buffer(c)){
      
   }
}


int init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x01);    // baud rate ??
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
  // outb(PORT + 1, 0x01);    // interrupt when data available
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)
 
   // Check if serial is faulty (i.e: not same byte as sent)
  if(inb(PORT + 0) != 0xAE) {
      printl("gg serial");
      return 1;
  }
 
   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   outb(PORT + 4, 0x0F);
   return 0;
}


//proctol file transfer
// 1. <total bytes to transfer>
// 2. file content

uint32_t serial_file_transfer(uint32_t mem_addr){
   printl("start to transfer file");
   #define BUF_SIZE 256
   char buffer[BUF_SIZE];
   uint32_t rx_total = 0;
   uint32_t rx_cnt = 0;

   read_until(buffer,BUF_SIZE,0);
   uint32_t expected_bytes = atoi(buffer,10);

   printstr("expected ");
   print_hex(expected_bytes);
   printl(" bytes");

   while(rx_total < expected_bytes){
      rx_cnt = read_bytes(buffer,BUF_SIZE,expected_bytes-rx_total);
      // don't use strncpy because address can be zero (null) and strncpy won't copy null
      memcpy(buffer,(char*)(mem_addr+rx_total),rx_cnt);
      rx_total += rx_cnt;
   }
   // printl((char*)mem_addr);
   printstr("recieved ");
   print_hex(rx_total);
   printl(" bytes");
   return rx_total;
}

int read_until(char* buf,uint32_t buf_size,char end_char){
   char c;
   uint32_t rx_cnt = 0;
   while(rx_cnt != buf_size-1){
      c = read_serial();
      buf[rx_cnt++] = c;
      if(c == end_char){
         break;
      }
   }
   buf[rx_cnt] = 0;
   return rx_cnt; 
}

int read_bytes(char* buf,uint32_t buf_size,uint32_t total){
   char c;
   uint32_t rx_cnt = 0;
   while( rx_cnt != total && rx_cnt != buf_size-1){
      c = read_serial();
      buf[rx_cnt++] = c;
   }
   buf[rx_cnt] = 0;
   return rx_cnt; 
}


int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}
 
void write_serial(char a) {
   while (is_transmit_empty() == 0);
 
   outb(PORT,a);
}

int serial_received() {
   return inb(PORT + 5) & 1;
}
 
char read_serial() {
   while (serial_received() == 0);
   return inb(PORT);
}