
#include "malloc.h"
#include "mem.h"
#include <stdint.h>
#define CREATE_HEADER(addr,__type,__size,__link)\
    addr->magic = 0x87;\
    addr->type  = __type;\
    addr->size = __size;\
    addr->link = __link;\

/*
#define  LOOP_FREE_LIST(prev,cur,cond)\
    struct RegionHeader  * prev=NULL; \
    struct RegionHeader  * cur  =  index_head; \
    while(cur){  \
        if((cond)){ \
            break; \
        }          \
        prev = cur;\
        cur = cur->link;\    
    }\
*/

#define R_HEADER_SIZE  (sizeof(struct RegionHeader))
#define MEM_ADD(addr,val)((uint32_t )(addr)+val)
#define GET_DATA(hd_addr)((uint32_t )(hd_addr)+R_HEADER_SIZE)
#define GET_NEXT_HEADER(hd_addr)(  (struct RegionHeader*)   ( (uint32_t) (hd_addr) + (hd_addr->size) + R_HEADER_SIZE ) )


#define REACH_HEAP_END(addr)((uint32_t) addr >= HEAP_END)


/*
 *        don't check sanify for free function (speed consideration)
 *        kmalloc(size) -->  search free continous size space from heap range 
 *                      -->  a list of  heap region
 *                      -->  at start of allocated heap region
 *
 *        index_free_list (by size)
 *        
 *        empty_chunk
 *          status = free
 * 
 * 
 *        chunk header at start of allocated region
 *             status  busy/free
 *             size :  size
 *             magic 
 *             header_previous :  for unification ,
 *             snext    -->   for sorted size free list ,
 *             
 *         
 *        ignore unification first ..
 *        goal is save index as list (not as array because it requires a fucking predefined size)
 *  
 *        [size,magic,free_flag] [size,magic,free_flag]                
 *                              
 *        by look at next, we can know how much free space 
 *        if next = NULL -->
 * 
 *        allocate 
 *            search  best fitting  (least waste) !!!!
 *            search index to get best fitting so it's need to put index  header at free space ...
 *            because need free, so allocated frame  must also need a header
 *        
 *        S = start of head
 *        M = total heap size      
 *        [SIZE, [status], P, S]
 * 
 * 
 *        1.   head_s = S   
 *             [  M , free, 0, 0  ]
 * 
 *        2.    malloc(30) --> A  
 *             [M=30,free,0,0][30, busy,S]  ( allocate at end of empty space, so don't need to remove header) 
 * 
 *                   
 *           
 *              
 *         0                                             heap_max-30-header
 *         [ size = max-30 ,sorted_next=NULL] [size=30      ][30]
 *        3.  index =  [size=40] , 0
 *              malloc (40) ; malloc(50)
 *        [size = max-120 , free][size=50][size=40][40] [size=30][30]
 *        4.  index =
 *               free(addr=[size=40])
 *        [size = max-120 , free][size=50][size=40,free][30]
 * 
 * 
*/






uint32_t heap_start = 0;
struct RegionHeader  * index_head = NULL ;  // the start pointer of free region list , it has the smallest size region

// TODO how to use a algorithm to auto test
void test_heap(){


    #define PRINT_ADDR_LABEL_OFST_HDR(addr,n)\
        printstr(#addr " = ");print_hex((int)(addr));printstr(" " #addr " " #n "HDR = ");print_hex((int)(addr)+(n*R_HEADER_SIZE));printl("");\
    

    #define PRINT_ADDR_LABEL(addr)\
        printstr(#addr " = ");print_hex((int)(addr));printl(""); \
       


    uint32_t  *addr1, *addr2, *addr3,*addr4,*addr5,*addr6,*addr7,*addr8,*addr9;
    printl("heap header size");
    print_hex(R_HEADER_SIZE);

    printl("");
    printl("");

    printl("test heap 1");

    // test malloc then free will get same address
    // addr1 == addr2

    addr1 = kmalloc(8);
    kfree(addr1);
    PRINT_ADDR_LABEL(addr1);

    addr2 =  kmalloc(8);
    kfree(addr2);
    PRINT_ADDR_LABEL(addr2);


    ASSERT(addr1==addr2 ,"test1");


    // test two unifcation 
    // addr2 + R_HEADER_SIZE == addr3

    printl("test heap 2");

    addr1 = kmalloc(8);
    addr2 = kmalloc(8);
    PRINT_ADDR_LABEL(addr2);
                       // hdr2 <12> addr2  [8]  hdr1 <12>  addr1 [8]
                       //heap_start...   addr2 [28]
    kfree(addr1);
    kfree(addr2);
    addr3 = kmalloc(16);
                        //    addr2  hdr3 <12> addr3 [16]

    kfree(addr3);
    PRINT_ADDR_LABEL(addr3);
    
    ASSERT( (uint32_t) addr3 == ((uint32_t)addr2+R_HEADER_SIZE) ,"test2");

    //test three unification

    printl("test heap 3");
    addr1 = kmalloc(8);
    addr2 = kmalloc(8);
    addr3 = kmalloc(8);
    //addr4 = kmalloc(8);
    PRINT_ADDR_LABEL(addr3);
    
    kfree(addr1);
    kfree(addr3);
    kfree(addr2);
    addr4 = kmalloc(24);
    kfree(addr4);

    ASSERT( (uint32_t) addr4 == ((uint32_t)addr3+2*R_HEADER_SIZE) ,"test3");

    printl("test heap 4");
    addr1 = kmalloc(18);
    addr2 = kmalloc(16);
    addr3 = kmalloc(32);
    addr4 = kmalloc(24);

    PRINT_ADDR_LABEL(addr1);
    PRINT_ADDR_LABEL(addr2);
    PRINT_ADDR_LABEL(addr3);
    PRINT_ADDR_LABEL(addr4);
                            // hd4    addr4   hd3    addr3    hd2    addr2    hd1    addr1
                            //    [12]     [24]   [12]     [32]   [12]     [16]   [12]      [18]
    kfree(addr2);
    kfree(addr3);
                            // hd4    addr4    hd3    addr3     hd1     addr1
                            //    <12>     <24>   <12>     [60]     <12>      <18>
    addr5 = kmalloc(40);   
                            // hd4     addr4     hd3     addr3   hd5      addr5      hd1     addr1
                            //    <12>      <24>    <12>      [8]   <12>       <40>      <12>      <18>

    kfree(addr4);
                            // hd4     addr4        hd5      addr5      hd1     addr1
                            //    <12>      [44]       <12>       <40>      <12>      <18>

    kfree(addr1);
                            // hd4     addr4        hd5      addr5      hd1     addr1
                            //    <12>      [44]       <12>       <40>      <12>      [18]
    addr6 = kmalloc(4);  
            
                            // hd4     addr4        hd5      addr5      hd1     addr1    hd6    addr6
                            //    <12>      [44]       <12>       <40>      <12>      [2]   <12>      <4>

          
    addr7 = kmalloc(20); 
                            // hd4     addr4    hd7    addr7       hd5      addr5      hd1     addr1    hd6    addr6
                            //    <12>      [12]   <12>     <20>      <12>       <40>      <12>      [2]   <12>      <4> 
    PRINT_ADDR_LABEL(addr5);
    PRINT_ADDR_LABEL(addr6);
    PRINT_ADDR_LABEL(addr7);


    ASSERT( (uint32_t) addr5 == ((uint32_t)addr3+20) ,"test4-1");
    ASSERT( (uint32_t) addr6 == ((uint32_t)addr1+14) ,"test4-2");
    ASSERT( (uint32_t) addr7 == ((uint32_t)addr4+24) ,"test4-3");
    
  
    kfree(addr5);
    
                            // hd4     addr4    hd7    addr7       hd5      addr5    hd6    addr6
                            //    <12>      [12]   <12>     <20>      <12>       [54]   <12>      <4> 

    addr8 = kmalloc(30);

                         //... hd4     addr4    hd7    addr7       hd5      addr5    hd8     addr8     hd6    addr6
                            //    <12>      [12]   <12>     <20>      <12>       [12]    <12>     [30]    <12>      <4> 

    ASSERT( ((uint32_t) addr8 == ((uint32_t)addr7+56)&& ((struct RegionHeader  *) ((uint32_t)addr8-R_HEADER_SIZE))->size == 30 ) ,"test4-4");
    addr9 = kmalloc(100);
                            // hd9    addr9   addr4    hd7    addr7       hd5      addr5    hd8     addr8     hd6    addr6
                            //    <12>     [88]    [12]   <12>     <20>      <12>       [12]    <12>     [30]    <12>      <4> 

    ASSERT( ((uint32_t) addr9+112) == (uint32_t) addr7  ,"test4-5");

    kfree(addr6); kfree(addr7); kfree(addr8); kfree(addr9);

}

void init_heap(){
    //extern uint32_t _heap_start;
    //heap_start = (uint32_t)(&_heap_start);
    // for current system design
    // some user program is concat right after the end of data section
    // which is also  &_heap_start linker symbol defined
    
    // because of this reason, can't use &_heap_start as start adress of heap 
    heap_start = HEAP_START;
    index_head = (struct RegionHeader  *) heap_start;
    //printf("head start %x\n",heap_start); 
    CREATE_HEADER( index_head,REGION_FREE,HEAP_END-heap_start-R_HEADER_SIZE,NULL);
   
}


/*
static inline int is_free_region(struct RegionHeader* addr){
    if(addr->type == REGION_FREE ){
        return 0;
    }
    return 1;
}
*/

void add_alloc_region(struct RegionHeader* addr){

    struct RegionHeader* next = GET_NEXT_HEADER(addr);

    if(REACH_HEAP_END(next) ){
        return;
    }

    if(next->type == REGION_FREE){
        return ;
    }

    next->link = addr;
}

void del_alloc_region(struct RegionHeader* addr){

     struct RegionHeader* next =  GET_NEXT_HEADER(addr);

    if(REACH_HEAP_END(next)){
        return;
    }

    if(next->type == REGION_FREE){
        return ;
    }

    next->link = addr->link;

}

void add_free_region(struct RegionHeader * prev,struct RegionHeader * next, struct RegionHeader * new){



    if(!prev){   // add as head 
        index_head = new;
    }else{
        prev->link = new;
    }

    new->link = next;

}

void del_free_region(struct RegionHeader * prev,struct RegionHeader *  deleted){

    if(!deleted){
        return ;
    }

    struct RegionHeader * next = deleted->link;

    if(prev){
        prev->link = next;
    }else{
        index_head = next;
    }

}

void search_and_add_free_region(struct RegionHeader * new){

    struct RegionHeader * prev = NULL;
    struct RegionHeader * cur  =  index_head;

    while(cur){
        if( cur->size >= new->size ){
            break;
        }
        prev = cur;
        cur = cur->link;
    }  
    add_free_region(prev,cur,new);

}

/*
    return address is offset by header size

*/

uint32_t* kmalloc(uint32_t size){

    struct RegionHeader *  alloc_hd = NULL;
    struct RegionHeader * prev = NULL;
    struct RegionHeader * cur  =  index_head;

    // LOOP_FREE_LIST
    while(cur){
        if(cur->size >= size + R_HEADER_SIZE){
            break;
        }
        prev = cur;
        cur = cur->link;
    }

    if(!cur){
        return NULL;
    }

    del_free_region(prev,cur);
    cur->size = cur->size - size - R_HEADER_SIZE;
    search_and_add_free_region(cur);

    alloc_hd = GET_NEXT_HEADER(cur);

    CREATE_HEADER(alloc_hd,REGION_ALLOC,size,cur)
    add_alloc_region(alloc_hd);

    return  (uint32_t*) MEM_ADD(alloc_hd,R_HEADER_SIZE);
}


/*
 addr pass by user is the real data address, so must substarct   header size to addr
*/



void  kfree (uint32_t* addr){

    addr = (uint32_t*) ((uint32_t)addr - R_HEADER_SIZE);

    struct RegionHeader *  header  = ( struct RegionHeader *) addr;
    struct RegionHeader *  prev  = ( struct RegionHeader *) header->link;
    struct RegionHeader *  next   = GET_NEXT_HEADER(header);
    struct RegionHeader *  it1   = 0;
    struct RegionHeader *  it2   = 0;

    if(REACH_HEAP_END(next) ||  next->type == REGION_ALLOC){
        next =  NULL;
    }

    if(prev->type == REGION_ALLOC){
        prev = NULL;
    }

   
    header->type = REGION_FREE;

    if(prev){
        it1 = 0;
        it2 = index_head;
        while(it2!=prev){
            it1 = it2;
            it2 = it2->link;
        }

        del_free_region(it1,prev);
    }

    if(next){
        it1 = 0;
        it2 = index_head;
        while(it2!=next){
            it1 = it2;
            it2 = it2->link;
        }
        del_free_region(it1,next);
    }


    struct RegionHeader *  unified_hd  =  header;
    
    if(prev){
        unified_hd = prev;
        unified_hd->size+= R_HEADER_SIZE+ header->size;
    }

    if(next){
        unified_hd->size+= R_HEADER_SIZE+ next->size;
    }

    search_and_add_free_region(unified_hd);

    next = GET_NEXT_HEADER(unified_hd);

    if( !(REACH_HEAP_END(next))){
        next->link = unified_hd;
    }
}
















