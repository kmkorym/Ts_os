#include "../lib/print.h"
#include "../kernel/isr.h"

int main(){
    clear();
    init_idt();
    init_devices();
    
    if (0!=init_serial()){
        printl("serial init error!");
    }
    char c;
    write_serial('Q');
    //c=read_serial();
    //print_char(c);
    printl("GG!");
    
    
    
    
    return 0;
}