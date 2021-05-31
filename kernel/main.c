#include "../lib/print.h"
#include "isr.h"
#include "task.h"

int main(){
    clear();
    printl("kernel main start");
    setup_tss();
    init_idt();
    init_devices();
    return 0;
}