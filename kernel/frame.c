#include "frame.h"
#define FRAME_SIZE 4096   // how many bytes in a frame
#define FRAMESET_SIZE 8   // how many bits a frameset element represents
#define OUT_ADDR_RANGE(addr) (addr < start_heap || addr >= end_heap)
#define FRAMESET_INDEX(addr) (addr/(FRAME_SIZE*FRAMESET_SIZE))
#define FRAMESET_OFFSET(addr) ((addr/FRAME_SIZE)%FRAMESET_SIZE)
/*
extern uint32_t _start_heap;
uint32_t start_heap = (uint32_t) &_start_heap;
uint32_t end_heap  = 0 ;
uint8_t* _heap_top = (uint8_t*) start_heap;

uint8_t*  _frame_set;
uint32_t   _n_frame;

void memset(uint32_t start_addr,uint8_t val,uint32_t size){
    uint8_t* p = (uint8_t*) start_addr;
    while(size--){
        *p = val;
        ++p;
    }
}

void test_frame(){
    printl("test frame");
    print_hex( (int) start_heap);
}
*/

/*
strategy : 
this function don't handle fragment in phsical adress space
must call another function to check and handle physical memory problem then call this function
*/
/*
int  phy_malloc(uint32_t size,uint8_t* mem_start){
    uint32_t new_head = _heap_top + size;
    if(OUT_ADDR_RANGE(new_head-1)){
        *mem_start = 0;
        // TODO : when there is no memory availabe, handle memory fragments then allocate again
        return -1;
    }
    set_frameset();
    *mem_start = _heap_top;
    _heap_top = new_head;
    return 0;
}


void initialze_frame_set(uint32_t total_bytes){
    _n_frame =  total_bytes/FRAME_SIZE;
    end_heap = start_heap + total_bytes;
    if(total_bytes % FRAME_SIZE){
        ++_n_frame;
    }

    _frame_set = _heap_top;
    if (_n_frame % FRAMESET_SIZE){
        memset(_frame_set,0,_n_frame/FRAMESET_SIZE+1);
         _heap_top =  _heap_top + _n_frame/FRAMESET_SIZE+1
    }else{
        memset(_frame_set,0,_n_frame/FRAMESET_SIZE);
         _heap_top =  _heap_top + _n_frame/FRAMESET_SIZE;
    }
}

uint8_t get_frameset_value(uint32_t addr){
    uint32_t idx =  FRAMESET_INDEX(addr);
    uint32_t offset = FRAMESET_OFFSET(addr);
    return _frame_set[idx] & (1>>offset);
}

uint8_t frame_available(uint32_t addr){
    if(OUT_ADDR_RANGE(addr)){
        return 0;
    }
    return get_frameset_value()?0:1;
}


void _set_value_on_frameset(uint32_t addr,uint8_t val){
    uint32_t idx;  
    uint32_t offset; 
    if(!OUT_ADDR_RANGE(addr)){
        idx  =  FRAMESET_INDEX(addr);
        offset = FRAMESET_OFFSET(addr);
        if(!val)){
            _frame_set[idx] = _frame_set[idx] & ~(1 >> offset);
        }else{
             _frame_set[idx] = _frame_set[idx] |( 1 >> offset);
        }
    }
}

void set_frameset(uint32_t start,uint32_t end){
    _set_value_on_frameset(start,1);
    _set_value_on_frameset(end,1);
}

void clear_frameset(uint32_t start,uint32_t end){
    _set_value_on_frameset(start,0);
    _set_value_on_frameset(end,0);
}
*/
