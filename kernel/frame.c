#include "frame.h"
#include "common.h"
#define FRAME_SIZE 4096   // how many bytes in a frame
#define MAX_FRAME_NUM  (MAX_MEM_SIZE/ FRAME_SIZE)
#define FRAME_NB(addr)(addr/FRAME_SIZE)
#define FRAME_BBASE(bm,addr)(&bm[FRAME_NB(addr)/8])
#define FRAME_BSET(bm,addr)(bm[FRAME_NB(addr)/8] |= 1<<(FRAME_NB(addr)%8))
#define FRAME_BCLR(bm,addr)(bm[FRAME_NB(addr)/8] &= ~(1<<(FRAME_NB(addr)%8)))

#define FRAME_BGET(bm,addr)(!! (*FRAME_BBASE(bm,addr) & 1<<(FRAME_NB(addr)%8)))

// TODO: check bss really exists?
uint8_t  bitmap[MAX_FRAME_NUM] = {0};


void test_frame(){

    uint32_t addr = 0 ;
    addr = alloc_frame(8*_1MB,16*_1MB);
    if(addr != 8*_1MB){
        printl("test frame error 1:");
        print_hex(addr);printl("");
    }
    addr = alloc_frame(8*_1MB,16*_1MB);
    if(addr != 8*_1MB+FRAME_SIZE){
        printl("test frame error 2:");
        print_hex(addr);printl("");
        print_hex(bitmap[( 8*_1MB)/FRAME_SIZE]);;printl("");
    }
    addr = alloc_frame(8*_1MB,16*_1MB);
    if(addr != 8*_1MB+2*FRAME_SIZE){
        printl("test frame error3:");
        print_hex(addr);printl("");
    }
    addr = alloc_frame(7*_1MB,8*_1MB);

    if(addr != 7*_1MB){
        printl("test frame error 4:");
        print_hex(addr);printl("");
    }

    uint8_t   status1 = frame_status(8*_1MB);
    uint8_t   status2 = frame_status(8*_1MB+FRAME_SIZE);
    uint8_t   status3 = frame_status(8*_1MB+2*FRAME_SIZE);
    uint8_t   status4 = frame_status(7*_1MB);


    if( (status1 & status2 & status3 & status4) !=1){
        printl("test frame error 5:");
        print_hex(status1);  printl("");
        print_hex(status2);  printl("");
        print_hex(status3);  printl("");
        print_hex(status4);  printl("");
    }

    clear_frame(8*_1MB+FRAME_SIZE);
    status2 = frame_status(8*_1MB+FRAME_SIZE);
    if(status2 !=0){
        printl("test frame error 6:");
        print_hex(status2);printl("");     
    }


    addr = alloc_frame(8*_1MB,16*_1MB);
    if(addr != 8*_1MB+FRAME_SIZE){
        printl("test frame error 7:");
        print_hex(addr);printl("");
    }

    clear_frame(8*_1MB);
    addr = alloc_frame(8*_1MB,16*_1MB);
    if(addr != 8*_1MB){
        printl("test frame error 8:");
        print_hex(addr);printl("");
    }

    clear_frame(8*_1MB);
    clear_frame(8*_1MB+FRAME_SIZE);
    addr = alloc_frame(8*_1MB,16*_1MB);
    if(addr != 8*_1MB){
        printl("test frame error 9:");
        print_hex(addr);printl("");
    }

    addr = alloc_frame(8*_1MB,16*_1MB);
     if(addr != 8*_1MB+FRAME_SIZE){
        printl("test frame error 10:");
        print_hex(addr);printl("");
    }
}


/*
1 --> busy
0 --> idle
*/

uint8_t  frame_status(uint32_t addr){
     uint8_t tmp = FRAME_BGET(bitmap,addr);
     return tmp;
}



uint32_t alloc_frame(uint32_t min_addr,uint32_t max_addr){

    uint32_t base =  get_idle_frame_addr(min_addr,max_addr);

    if(!base){
        return 0;
    }
    
    FRAME_BSET(bitmap,base);
    return base;
}


void  clear_frame(uint32_t addr){
    FRAME_BCLR(bitmap,addr);
}

/*
    offset : frame addr must > offset
*/
uint32_t get_idle_frame_addr(uint32_t min_addr,uint32_t max_addr){

    if( !min_addr  ){
        printl("warning: get idle frame  min_addr  = 0 ");
    }
    
    // next page aligned addr

    if( min_addr%FRAME_SIZE){
         min_addr =  min_addr- ( min_addr%FRAME_SIZE)+FRAME_SIZE;
    }

    uint32_t  frame_idx  = FRAME_NB(min_addr);
    uint32_t  frame_addr = frame_idx*FRAME_SIZE;

    while(frame_addr<max_addr){
        if(bitmap[frame_idx] != 0xFFFF){
            int bit_offset = first_clr_bit(&bitmap[frame_idx/8]);
            frame_addr +=   bit_offset*FRAME_SIZE ;
            if(frame_addr+FRAME_SIZE > max_addr){
                return 0;
            }
            return frame_addr;
        }
        frame_addr+= 8 * FRAME_SIZE;
        ++frame_idx;
    }

    return 0;
}


/*

 for heap

 */

int  acquire_frame(uint32_t va,int is_user,uint32_t  *phy_addr){

    uint32_t addr;

    *phy_addr = 0 ;

    // if frame address for va already exist, return it

    if( get_frame_address(va,&addr) ){
        *phy_addr = addr;
        return 1;
    }

    // else allocate a frame address to va

    if(is_user){
        printl("heap for user not support now!");
        return 0;
    }else{
        addr = alloc_frame(KERNEL_HEAP_START,KERNEL_HEAP_END);
    }

    if(!addr){
        return 0;
    }


    *phy_addr = addr;
    allocate_page_tent(va,addr,is_user);

    return 1;

}