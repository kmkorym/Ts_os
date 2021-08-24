#include "../lib/print.h"
#include "isr.h"
#include "task.h"
#include "mem.h"
#include "frame.h"
#include "malloc.h"

int main(){
    clear();
    printl("kernel main start");
    //setup_tss();
    init_idt();
    init_devices();
    patch_page_table_k();
    init_heap();
    test_heap();
    return 0;
}