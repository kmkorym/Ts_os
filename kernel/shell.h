#include "../lib/print.h"
#include "../lib/string.h"
#include "../kernel/common.h"
#include "../drivers/com.h"

void halt();
void  parse_command(char*s);
void parse_serial_port_cmd();
void  parse_command(char*s);