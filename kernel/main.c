#include "../lib/print.h"
#include "isr.h"
#include "task.h"
#include "mem.h"

int main(){
    clear();
    printl("kernel main start");
    //setup_tss();
    init_idt();
    init_devices();
    patch_page_table_k();
    return 0;
}