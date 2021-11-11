#include "../lib/print.h"
#include "isr.h"
#include "task.h"
#include "mem.h"
#include "frame.h"
#include "malloc.h"
#include "sys.h"
#include "../drivers/ata.h"




/*
struct Disk_IO_Command{
   IDE_Device * device;
   uint32_t lba_start;
   uint32_t sec_num;
   uint32_t mem_addr;
   uint8_t *buf;
   uint8_t buf_size;
};
*/
void test_ata_driver(){
    char  buffer[1025]={0};
    IDE_Device *device = get_ide_device(0,0);
    uint32_t i;
    //ata_soft_reset(device);
    //printl("after reset");
    //ata_detect(0,0);
    Disk_IO_Command read_commad = {.device=device,.lba_start=1,.sec_num=1,.buf=buffer,.buf_size=512};
    Disk_IO_Command write_commad = {.device=device,.lba_start=0,.sec_num=2,.buf=buffer,.buf_size=1025};

    


    
    for(i=0;i<512;++i){
        buffer[i] = 0x33;
        buffer[i+512] = 0x87;
    }
    
    ata_write_sector(&write_commad);
    

    
    ata_read_sector(&read_commad);
    for(i=0;i<3;++i){
        print_byte(buffer[i]);
        printl("");
    }
    
    

    
    printl("after read");
    while(1){}
}



void parse_kargs(){
    extern uint32_t* pg_dir0;
    extern uint32_t karg_phy;
    struct Task *task;
    task = load_task(pg_dir0,karg_phy+KERNEL_V_START);
    uint32_t header2 = next_task_pos(karg_phy+KERNEL_V_START);
    task = load_task(pg_dir0,header2);
    uint32_t header3 = next_task_pos(header2);
    task = load_task(pg_dir0,header3);
    //schedule();
    //printl("return from task2");  
    //switch_task(task);
    //printl("return from task1");
    //task = load_task(pg_dir0,karg_phy+KERNEL_V_START);
    //switch_task(task);
    //printl("return from task2");
}




int main(){
    clear();
    printl("kernel main start");
    re_init_pg_dir0();
    test_page();
    test_ata_driver();
    while(1){}
    //setup_tss();
    /*
    init_idt();
    init_devices();
    init_heap();
    test_heap();
    init_task0();
    init_syscall();
    */
    //parse_kargs();
    //printl("return parse kargs");
    //while(1){};
    return 0;
}