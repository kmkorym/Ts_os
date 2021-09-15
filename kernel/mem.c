
#include "mem.h"
#include "common.h"
#include "frame.h"

#define DIR0_PT_PHY(dir_idx)(PG_DIR0_ADDR +FRAME_SIZE+4096*dir_idx)

uint32_t  pg_dir0_phy =  PG_DIR0_ADDR;
#ifdef EARLY_INIT
uint32_t* pg_dir0 = (uint32_t*) PG_DIR0_ADDR ;
#else
uint32_t* pg_dir0 = (uint32_t*)( PG_DIR0_ADDR + KERNEL_V_START);
#endif

uint32_t karg_phy;




static inline void __native_flush_tlb_single(uint32_t addr) {
   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}


//hexdump   -e '"%_ax" "\t" 16/1 "%02X " "\n"'  build/kimage

//   TODO page dir address dir[1022][1023]
//   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void memzero(void*_p,uint32_t size){
    uint8_t* p = (uint8_t*) _p ;
    for(uint32_t i=0;i<size;++i){
        *p = 0;
        ++p;
    }
}

int pg_dir_no_entry(uint32_t* dir,uint32_t va){
    return PAGE_ENTRY_SET(dir[PAGE_DIR_INDEX(va)]) ? 0:1;
}

uint32_t pt_phy_from_dir(uint32_t* dir,uint32_t  dir_index){
    ASSERT(PAGE_ENTRY_SET(dir[dir_index]),"error pt_phy_from_dir");
    return dir[dir_index] & 0XFFFFF000;
}


int  pg_dir_add(uint32_t* dir,uint32_t *rev_tb,uint32_t di,uint32_t pt_phy,uint32_t flag){
    uint32_t  *ent = &dir[di];
    if(PAGE_ENTRY_SET(*ent)){
        return 0;
    }
    *ent  = (pt_phy & 0XFFFFF000 ) | flag;
    rev_tb[di] = (pt_phy & 0XFFFFF000 ) | PAGE_P;
    return 1;
}



uint32_t * lookup_temp_va(uint32_t pa){
    uint32_t i;
    uint32_t* page_table  = PAGE_TABLE_PTR(1022);
    for(i=0;i<1024;++i){
       if( PAGE_ENTRY_SET(page_table[i]) && (page_table[i]&0xFFFFF000) == pa){
            return (uint32_t*) IX_TO_VA(1022,i);
       }
    }
    return NULL;   
}


uint32_t * lookup_pt_temp_va(uint32_t* dir,uint32_t di){
    uint32_t phy = pt_phy_from_dir(dir,di);
    ASSERT(phy!=NULL ,"lookup_pt_temp_va");
    return lookup_temp_va(phy);
}


uint32_t *  get_pt_va(uint32_t* dir,uint32_t di){
    uint32_t phy = pt_phy_from_dir(dir,di);
    ASSERT(phy!=NULL ,"get_exist_pt_va");
    return get_temp_va(phy);
}



uint32_t*  get_temp_va(uint32_t pa){ 
    uint32_t*  p = lookup_temp_va(pa);
    if(p){
        return p;
    }
    return create_temp_va(pa);
}



uint32_t* create_page_table(uint32_t* dir,uint32_t* rev_tb,uint32_t va,uint32_t flag){
    
    if( ! pg_dir_no_entry(dir,va)){
        return NULL;
    }   

    uint32_t dir_index = PAGE_DIR_INDEX(va);

    #ifdef EARLY_INIT
    uint32_t  pt_phy      =  DIR0_PT_PHY(dir_index);
    uint32_t  *pt_base    = (uint32_t*) pt_phy;
    memzero((void*)pt_base,FRAME_SIZE);
    #else
    uint32_t  pt_phy      =  alloc_frame(USER_P_START,MAX_MEM_SIZE);
    uint32_t *pt_base     =  create_temp_va(pt_phy);
    if(!pt_phy || !pt_base){
        return NULL;
    }
    memzero((void*)pt_base,FRAME_SIZE);   
    #endif
    dir[dir_index] = pt_phy | flag;
    if(rev_tb){
        rev_tb[dir_index] = pt_phy | PAGE_P;
    }
    return  pt_base;
}





uint32_t * get_page_table(uint32_t *dir,uint32_t* rev_tb,uint32_t va,uint32_t flag){
    uint32_t * pt = create_page_table(dir,rev_tb,va,flag);
    if(!pt){
        uint32_t pt_phy = pt_phy_from_dir(dir,PAGE_DIR_INDEX(va));
        pt = lookup_temp_va(pt_phy);
        if(!pt){
            pt =  create_temp_va(pt_phy_from_dir(dir,PAGE_DIR_INDEX(va)));
            ASSERT(pt!=NULL,"get pg table");
        }
    }
    return pt;
}

uint32_t get_phy_address(uint32_t va){
    uint32_t dir_idx = PAGE_DIR_INDEX(va);
    uint32_t pt_idx = PAGE_TABLE_INDEX(va);
    uint32_t offset = PAGE_OFFSET(va);
    uint32_t * pt = (uint32_t*) ( 0xFFC00000 + (dir_idx<<12) );
    return ( pt[pt_idx]&0xFFFFF000 ) + offset ;
}



uint32_t get_phy_from_dir(uint32_t *dir,uint32_t va){

    if( ! PAGE_ENTRY_SET(dir[PAGE_DIR_INDEX(va)] )  ){
        ASSERT(1==0," get_phy_from_dir page dir not set");
        return NULL;
    }

    uint32_t phy_tb = dir[PAGE_DIR_INDEX(va)]  & 0xFFFFF000;
    uint32_t   *tb  = get_temp_va(phy_tb);
    uint32_t phy = tb[PAGE_TABLE_INDEX(va)] & 0xFFFFF000;

    delete_temp_va((uint32_t)tb);

    return phy;
}

uint32_t pt_phy_from_revmap(uint32_t* dir,uint32_t nb){
    if( ! PAGE_ENTRY_SET(dir[nb])  ){
        ASSERT(1==0,"error pt_phy_from_revma");
        return NULL;
    }
    uint32_t* pt = (uint32_t*)(0xFFFFF000);
    return pt[nb]&0xFFFFF000;
}


uint32_t*  create_temp_va(uint32_t pa){
    uint32_t i;
    uint32_t* page_table  = PAGE_TABLE_PTR(1022);
    for(i=0;i<1024;++i){
       if(! PAGE_ENTRY_SET(page_table[i])){
            page_table[i] = pa | PAGE_FLG_KERNEL;
            return (uint32_t*) IX_TO_VA(1022,i);
       }
    }
    return NULL;
}


void delete_temp_va(uint32_t va){
    uint32_t* page_table  = PAGE_TABLE_PTR(1022);
    page_table[PAGE_TABLE_INDEX(va)] = 0;
   
    // if not add this line, page entry change might not seen by the cpu
    // and still use old page content ...
    __native_flush_tlb_single(va);
}




void switch_page_dir(uint32_t dir_addr){
    asm volatile ( 
                    "movl %0,%%eax \n\t"
                    "movl %%eax,%%cr3 \n\t"    
                    : 
                    : "r"(dir_addr)
                    :"eax"
    );

}

void enable_paging(uint32_t dir_addr){
    switch_page_dir(dir_addr);
    asm volatile ( 
                    "movl %%cr0,%%eax \n\t"  
                    "orl $0x80000001,%%eax \n\t"  
                    "movl %%eax,%%cr0 \n\t"           
                    : 
                    :
                    :"eax"
    );
}




/*
    1.identity mapping first 1MB code (boot up code)
    2. 1MB ~ 8MB  is for page tables (kernel space) virtual address start from 3G
    3. 8MB ~ 16MB for kernel sections virtual address starts from 3G+8MB
    4. 16NB ~ MAX_MEM_SIZE for user  (user space so virtual address starts from 0)
*/
// higher kernel setting settings before enable page table
#ifdef EARLY_INIT

int init_pg_dir0(){
    memzero((void*)pg_dir0,FRAME_SIZE);
    uint32_t * rev_tb  = create_page_table(pg_dir0,NULL,0xFFC00000,PAGE_FLG_KERNEL);
    rev_tb[1023] = pt_phy_from_dir(pg_dir0,1023)  | PAGE_P;
    pg_dir_add(pg_dir0 ,rev_tb,1022,DIR0_PT_PHY(1022),PAGE_FLG_KERNEL);
   
    //TODO create system mapping table ...
}

/*
    because only early setup can access reverse page table by physical address
    this function can only call in early setup phase
*/
void set_pg_dir0_vmap(uint32_t va,uint32_t pa ,uint32_t flag){
    uint32_t *rev_tb = (uint32_t*) pt_phy_from_dir(pg_dir0,1023); 
    uint32_t *pt     =  create_page_table(pg_dir0,rev_tb,va,PAGE_FLG_KERNEL);
    if(!pt){
        pt = (uint32_t*) pt_phy_from_dir(pg_dir0,PAGE_DIR_INDEX(va));
    }
    pt[PAGE_TABLE_INDEX(pa)] = pa | PAGE_P;
}


void init_pg_dir0_vmap(){
    uint32_t physical_address = 0;
    clear();
    init_pg_dir0();
    //identity mapping first 1MB code (boot up code)
    while( physical_address <  _1MB  ){
        set_pg_dir0_vmap(physical_address,physical_address,PAGE_FLG_KERNEL);
        physical_address+= FRAME_SIZE;
    }

    physical_address = 0 ;
    // 0MB - 1MB ~ *MB is for GDT  + setup code
    // 1MB ~ 8MB  is for page tables (kernel space) virtual address start from 3G
    // 8MB ~ 16MB for kernel code + data
    while( physical_address <  USER_P_START ){
        set_pg_dir0_vmap(physical_address+KERNEL_V_START,physical_address,PAGE_FLG_KERNEL);
        physical_address+= FRAME_SIZE;
    }

    extern int _init_end;  
    printl("init_end");
    print_hex((int)&_init_end);
}
void init_page_settings(){
    init_pg_dir0_vmap();
    enable_paging(pg_dir0_phy);
}
#else


// can't request same region when it's request once
// if want to request again, must delete the region first
int request_region_vmap(uint32_t *dir,uint32_t va_start,uint32_t size,uint32_t flag){
    uint32_t  va;
    uint32_t  va_end;
    uint32_t *new_pt,*pt_now;
    uint32_t * rev_tb  =  0;
    uint32_t  new_rev_va_flg = 0;
    
    va_end =   (va_start+size) & 0xFFFFF000;
    va_start = va_start & 0xFFFFF000;
   
    rev_tb = lookup_pt_temp_va(dir,1023);
    if(!rev_tb){
        rev_tb =   get_pt_va(dir,1023);
        new_rev_va_flg = 1;
    }
    
    new_pt = pt_now = NULL;

    for(va=va_start;va<=va_end;va+=FRAME_SIZE){
        new_pt = create_page_table(dir,rev_tb,va,PAGE_FLG_KERNEL);
        if(new_pt){  
            if(pt_now){
                delete_temp_va((uint32_t)pt_now);
            }
            pt_now = new_pt;
        }else{ 
            // the case of va_start has created pg dir entry  but w/o mapping to va
           if(!pt_now){  
             pt_now = get_pt_va(dir,PAGE_DIR_INDEX(va));
           }
           // otherwise previous va is in the same page dir range  , just use pt_now
       }

        uint32_t *pt_entry = &pt_now[PAGE_TABLE_INDEX(va)];

        if(!PAGE_ENTRY_SET( *pt_entry)){
            uint32_t frame_phy;
            uint32_t * page;
            page = alloc_page(USER_P_START,MAX_MEM_SIZE,&frame_phy);
            *pt_entry =  frame_phy | flag;
            delete_temp_va((uint32_t)page);
        }

    }

    if(new_rev_va_flg){
        delete_temp_va((uint32_t)rev_tb);
    }

    return 1;
}



uint32_t*  alloc_page(uint32_t start_phy,uint32_t end_phy,uint32_t* phy){
    uint32_t  _phy =  alloc_frame(start_phy,end_phy);
    uint32_t  *ptr = create_temp_va(_phy);
    memzero((void*)ptr,FRAME_SIZE);
    *phy = _phy;
    ASSERT(_phy && ptr ,"allocate page");
    return ptr;
}


void re_init_pg_dir0(){
    uint32_t physical_address = 0;
    // clear indentiy mapping
    while( physical_address <  _1MB  ){
        pg_dir0[PAGE_DIR_INDEX(physical_address)] = 0 ;
        physical_address+=FRAME_SIZE;
    }
}


void test_page(){
    uint32_t i,phy1,phy2;
    uint32_t * p = ( uint32_t *)(0x9000+_1MB*15-4+KERNEL_V_START);
    *p=0x1234;
    printl("");
    ASSERT(*p == 0x1234 ,"test page 1");
    p = (uint32_t * )0x9008;
    ASSERT(*p == 0xFFFF ,"test page 2");
    
    // test reverse mapping table
    /*
    for(i=0;i<1023;++i){
        phy1 = KERNEL_PAGE_TABLE_TO_BASE(kernel_dir.base,i);
        pyh2 = *((uint32_t*)0xFFC00000 | i<<12) & (0xFFF);

        
    }
    */
    
    //test reverse mapping
    uint32_t p1,p2,p3,p4,p5;
    p1 = get_phy_address(0x9030);
    p2 = get_phy_address(0x9000+0x100000-4+KERNEL_V_START );
    p3 = get_phy_address(0x90000+0x50+KERNEL_V_START);
    printl("");
    print_hex(p1);
    ASSERT(p1 == 0x9030 ,"test page 3");
    printl("");
    print_hex(p2);
    ASSERT(p2 == (0x9000+0x100000-4) ,"test page 4");
    printl("");
    print_hex(p3);
    printl("");
    ASSERT(p3 == (0x90000+0x50) ,"test page 5");
    printl("");
    
    p4 = pt_phy_from_revmap(pg_dir0,PAGE_DIR_INDEX(KERNEL_V_START));
    
    print_hex(p4);
    printl("");
    print_hex(DIR0_PT_PHY(3));
    printl("");

    uint32_t  *rev_tb = (uint32_t*) REV_TB_VA ;

    ASSERT(  5*_1MB   == (*rev_tb&0xFFFFF000), "test page 6"  );
    
    ASSERT(p4 == DIR0_PT_PHY(PAGE_DIR_INDEX(KERNEL_V_START)) ,"test page 7");


}
#endif



// frame allication





