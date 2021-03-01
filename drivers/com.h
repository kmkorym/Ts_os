#include "../lib/print.h"
#include "../kernel/common.h"


int init_serial();
int is_transmit_empty();
void write_serial(char a);
int serial_received();
char read_serial();
void com1_handler();
uint32_t serial_file_transfer(uint32_t mem_addr);
int read_until(char* buf,uint32_t buf_size,char end_char);
int read_bytes(char* buf,uint32_t buf_size,uint32_t total);
