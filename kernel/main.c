#include "../lib/print.h"
#include "../lib/string.h"
#include "common.h"
#include "isr.h"
#include "task.h"
#include "mem.h"
#include "frame.h"
#include "malloc.h"
#include "sys.h"
#include "../drivers/ata.h"
#include "../fs/fat16.h"

void test_ata_driver(){
    uint8_t  buffer[1025]={0};
    IDE_Device *device = get_ide_device(0,0);
    uint32_t i;
    //ata_soft_reset(device);
    //printl("after reset");
    //ata_detect(0,0);
    Disk_IO_Command read_commad = {.device=device,.lba_start=1,.sec_num=1,.buf=buffer,.buf_size=512};
    Disk_IO_Command write_commad = {.device=device,.lba_start=0,.sec_num=2,.buf=buffer,.buf_size=1025};

    /*
    for(i=0;i<512;++i){
        buffer[i] = 0x33;
        buffer[i+512] = 0x87;
    }
    
    ata_write_sector(&write_commad);
    */
    
    ata_read_sector(&read_commad);

    for(i=0;i<3;++i){
        print_byte(buffer[i]);
        printl("");
    }

    printl("after read");
}


void _test_fat16_read(Fat16Volume* volume,char *path){
    
    Fat16File * file  = fat_open(volume,path,0);
    
    if(!file){
        printstr("can't open ");printl(path);
        return;
    }
    
    printstr("read ");printl(path);
    #define _test_fat16_buf_len 251
    char buf[_test_fat16_buf_len];
    int len;
    //int sl;
    while((len = fat_read_file(file,buf,_test_fat16_buf_len))!=EOF){
       // sl = strlen(buf);
        print_chars(buf,len);
    }
    printl("");
        
    fat_close(file);
}

void _test_fat16_find(Fat16Volume* volume,char *path){
    Fat16File *file =  new_fat16_file(path,volume,NULL,0);
    file = fat16_find(file,path);
    ASSERT(file!=NULL,"test fat16 find:  NULL file")

    printstr("test_fat_16: "); printl(path);
    printstr("result is -> ");printl(file->dirent->filename);

    kfree(  (uint32_t*)file);
}

void test_fat16_find(Fat16Volume* volume){
    _test_fat16_find(volume,"/1234");
    _test_fat16_find(volume,"/usr/");
    _test_fat16_find(volume,"/main");
    _test_fat16_find(volume,"/usr/abcd");
    _test_fat16_find(volume,"/usr/");
    char *name = "/usr/8787";
    ASSERT(!fat_open(volume,"/usr/8787",0),"not such file: /usr/8787");
}

void test_fat16_read(Fat16Volume* volume){
    _test_fat16_read(volume,"/1234");
    _test_fat16_read(volume,"/usr/abcd");
    _test_fat16_read(volume,"/main");
    _test_fat16_read(volume,"/usr/tmp/");
    //_test_fat16_read(volume,"/usr/main");
    //_test_fat16_read(volume,"gdb");
    //_test_fat16_read(volume,"/gdb");
    //_test_fat16_read(volume,"/usr/abcd");
    //_test_fat16_read(volume,"/gpio2");
   // _test_fat16_read(volume,"/usr/gpio3");
   // _test_fat16_read(volume,"/usr/");
   // _test_fat16_read(volume,"/usr/8787");
}



void _test_fat16_ls(Fat16Volume* volume,char *path){
    Fat16File *file = fat_open(volume,path,0);
    printl("list directory : "); printl(path);
    if(fat16_dirlist(file) < 0 ){
        printl("ls failed");
    }
}

void test_fat16_ls(Fat16Volume* volume){
    _test_fat16_ls(volume,"/");
    _test_fat16_ls(volume,"/usr");
    _test_fat16_ls(volume,"/usr/");
    _test_fat16_ls(volume,"/usr/gpio3");
    _test_fat16_ls(volume,"/usr/gpio4");
}


/*
    test case: 
    1. root create file
    2. root create file cross sector boundary
    3. subdirectory create file
    4. subdirectory create file cross cluster boundary

    1 2 3 4 for directory

    create directory/file under created subdirectory 

*/


void _test_fat_create(Fat16Volume *volume,char *path,uint32_t flag){
    Fat16File * file ;
    printstr("test fat create: "); printl(path);
    file = fat_open(volume,path,FAT16_CTL_CREATE|flag);
    ASSERT(file!=NULL,"test_fat_create : open failed");
}

void _test_fat_write1(Fat16Volume *volume,char *path,char *content){
    Fat16File * file ;
    printstr("test fat write: "); printl(path);
    file = fat_open(volume,path,FAT16_CTL_WRITE);
    ASSERT(file!=NULL,"test_fat_write : open failed");
    fat_write_file(file,content,strlen(content));
}


// cp test for write
void _test_fat_write2(Fat16Volume *volume,char *src_path,char *dest_path,int dest_flag){
    Fat16File * src_file,*dest_file ;
    #define FAT_WR_TEST2_BUF_SIZE 123
    char buf[123] = {0};
    printl("fat write test2 (copy): "); 
    printstr("from: " );  printstr(src_path);  printstr(" to:  " ); printl(dest_path);
    src_file  = fat_open(volume,src_path,0);
    dest_file = fat_open(volume,dest_path,FAT16_CTL_WRITE|FAT16_CTL_CREATE|dest_flag);
    ASSERT(src_file!=NULL && dest_file!=NULL ,"test_fat_write2 : open failed");
    while(fat_read_file(src_file,buf,FAT_WR_TEST2_BUF_SIZE)!= FAT_FILE_EOF){
        fat_write_file(dest_file,buf,FAT_WR_TEST2_BUF_SIZE);
    }
    printl("end of write test 2");
}

void test_fat_write(Fat16Volume *volume){
    _test_fat_write1(volume,"/file1","i am file1.txt haha");
    _test_fat_write1(volume,"/1234","1234 2234 3234 4234 \n");
    _test_fat_write1(volume,"/usr/abcd","abcdefh hijklmn\n");
    _test_fat_write1(volume,"/usr/log1","lllllllllllllllllllllllllllllllllllloggggggggggggggggggggggg\n");
    _test_fat_write1(volume,"/usr/desktop/config","aaa=bbb\nggg=ccc\nuser=12345\n");
    _test_fat_write2(volume,"/file1","/file2",0);
    _test_fat_write2(volume,"/1234","/file2",FAT16_CTL_APPEND);
    _test_fat_write2(volume,"/main","/usr/desktop/main2",0);
}



void test_fat_create(Fat16Volume* volume){
    _test_fat_create(volume,"/file1",0);
    _test_fat_create(volume,"/var",FAT16_CTL_DIR);
    _test_fat_create(volume,"/usr/log1",0);
    _test_fat_create(volume,"/usr/desktop",FAT16_CTL_DIR);
    _test_fat_create(volume,"/usr/desktop/config",0);
    _test_fat_create(volume,"/var/tmp",FAT16_CTL_DIR);
    _test_fat_create(volume,"/var/tmp2",FAT16_CTL_DIR);
    //_test_fat16_ls(volume,"/usr");
    printl("test fat create done");
}

void _test_fat_delete(Fat16Volume* volume,char *path){
    printstr("delete: "); printl(path);
    ASSERT(fat_delete_file(volume,path)>=0,"test_fat_delete : delete failed");
    ASSERT(fat_open(volume,path,0) == NULL, "test_fat_delete : delete failed2" );
}


void test_fat_delete(Fat16Volume* volume){
   // _test_fat_delete(volume,"/file1");
   // _test_fat_delete(volume,"/usr/main");
    //_test_fat_delete(volume,"/var/tmp2/..");
    _test_fat_delete(volume,"/usr/");
    ASSERT(fat_delete_file( volume,"/gg") == -1,"test_fat_delete : delete NULL file");
}


// directory, file  shrink test ...

void test_fat_stress_create(Fat16Volume* volume){

    int i;
    Fat16File * file ;
    char *folder = "/test1/";
    char s[5] = {0};
    char path_n[128]={0};    
    printstr("stress test create: "); printl(folder);
    file = fat_open(volume,folder,FAT16_CTL_CREATE|FAT16_CTL_DIR);
    ASSERT(file!=NULL,"fat stress test create : create topest level directory failed");
    printl("");
    for(i=0;i<128;++i){
        itoa(i,s,4);
        strcat(folder,s,path_n);
        file = fat_open(volume,path_n,FAT16_CTL_CREATE);
        ASSERT(file!=NULL,"fat stress open 2 failed");      
    }

    
}

void test_fat_stress_delete(Fat16Volume * volume){

    int i;
    Fat16File * file ;
    char *folder = "/trash/";
    char s[5] = {0};
    char path_n[128]={0};    
    printstr("stress test delete: "); printl(folder);
    printl("");
    for(i=0;i<128;++i){
        itoa(i,s,4);
        strcat(folder,s,path_n);
        fat_delete_file(volume,path_n);
    }

    printl("create and write to file /trash/aaa");
    file = fat_open(volume,"/trash/aaa",FAT16_CTL_CREATE|FAT16_CTL_APPEND);
    ASSERT(file!=NULL,"fat test delete 2 failed");
    char *pattern = "1234567890\n";

    for(i=0;i<1024;++i){
        fat_write_file(file,pattern,strlen(pattern));
    }
    
    file = fat_open(volume,"/trash/aaa",0);

    ASSERT(file!=NULL,"fat test delete 3 failed");
   
    
    printl("read file /trash/aaa");
    char read_buf[124] = {0};
    while(EOF != fat_read_file(file,read_buf,123)){
        printl(read_buf);
    }

}



void test_fat16(){
    IDE_Device *device = get_ide_device(0,0);
    Fat16Volume volume;
    
    fat_get_volume(&volume);
    //_test_fat16_read(&volume,"/file1");
    //fat16_display_super_block(&volume.sb);
    //dump_fat(&volume);
    test_fat16_find(&volume);
    test_fat16_read(&volume);
    //test_fat16_ls(&volume);
   // test_fat_create(&volume);
    //test_fat_write(&volume);
    //test_fat_delete(&volume);
    //test_fat_stress_create(&volume);
    test_fat_stress_delete(&volume);
    /*
    char *path = "/usr/gpio3";
    char buf[10000];
    //char buf[volume.cluster_bytes];
    printstr("read file ");printl(path);
    fat_read_file(&volume,path,buf,10000);
    printstr(path);printl(":");
    printl(buf);
    */

    //fat_create_file(&volume,"/","111");
    
   /*
    printl("find /usr/abcd.txt");
    FatDirEntry *dirent =  fat_find(&volume,"/usr/abcd");
    printl("Found");
    printl(dirent->filename);
    */
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


void test_printf(){
    char *s1="1234\n";
    printf("%d",1);
    printf("[%d]\n");
    printf("%s",s1);
    printf("%s %d %c%c\n","1234",5,'g','d');

    printf("%05x %x %02x %3x %5x %08x\n",0x5,0x5,0x123,0x1234,0x12342234,0x12345);
    printf("%x %0x %0x %0x\n",0,0x123,0,3);

    printf("%05d %d %02d %3d %5d %08d\n",5,5,123,1234,12342234,12345);
    printf("%d %0d %0d %0d\n",0,123,0,3);
}


int main(){
    clear();
    printl("kernel main start");
    re_init_pg_dir0();
    test_page();
    init_heap();
    //test_printf();
    //test_ata_driver();

    //test_fat16();
    
    setup_tss();
    
    init_idt();
    init_devices();
    //while(1){};
   // test_heap();
    //init_task0();
    //init_syscall();
    //parse_kargs();
    //printl("return parse kargs");
    //while(1){};
    return 0;
}
