#include "../lib/print.h"
#include "../kernel/isr.h"
#include "../test/test_string.c"
int main(){
    clear();
    init_idt();
    init_devices();
    
    // test_string_main();
    
    
    
    return 0;
}