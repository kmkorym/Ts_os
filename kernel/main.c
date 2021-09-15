#include "../lib/print.h"
#include "isr.h"
#include "task.h"
#include "mem.h"
#include "frame.h"
#include "malloc.h"

int main(){
    clear();
    printl("kernel main start");
    re_init_pg_dir0();
    test_page();
    //setup_tss();
    init_idt();
    init_devices();
    init_heap();
    test_heap();
    extern uint32_t* pg_dir0;
    start_task0(pg_dir0);
    return 0;
}