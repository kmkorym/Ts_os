/*
 * current implemation has 1MB limted size for kernel image because must load at real mode on floppy disk
 * and bss section is in data section so using kmalloc for screen buffer
 */

/*
    console:
    [input area]  
    [output area] (it's user view of screen buffer and can lift up or down)
    [last line for console status and its error message] (not implement yet)

    Principal:
    1. a history buffer content most N bytes of console history
    2. what the user see on the console is a window buffer, the content of window from
        - history content select by user 
        - the latest output to console
    3. output the window contetn from left up corner of console and output as more as possible
    4. user can view history content by press UP/DOWN key
       - UP/DOWN key show previous/next one row
    5. user can clear up window
    6. a row is terminating by MAX_COL or \n, and start of row is next to terminating 
       of previous row
    7. show the lateset output of console 
       if window is not following curret output
          0,0
       else
          last->window_offset  (cursor)

    Implementation:
    1. a window buffer represents current console output window content
        - the content is determine by two endpoint first (0,0) and last (open interval end)
          first->history             is the byte offset of history
          first->window_offset       always 0
          last->history              is the history offset of window (open interval)
          last->window_offset        the next character postion of window
    2. if history_cnt == last->window_offset  --> is following current
    3. first->history  will determines the window content 
        - condition to change first->history
          1. move up/down
          2. when last->window_offset >= MAX_WINDOW_SIZE
        
        when change first->history --> redraw screen

    4. last->history change for 
        1. if following current = history_cnt
        2. redraw screen
    
    5. move down/up_window
       --> start_offset = next_line/previous start
       if start_offset change --> redraw


*/

#include "vga.h"
#include "console.h"
#include "../lib/print.h"
#include "../lib/string.h"
#include "common.h"


uint32_t console_max_col = 0 ;
uint32_t console_max_row  = 0 ;
uint32_t cosole_ctl_flag = 0;

#ifdef EARLY_INIT
char history[CONSOLE_HISTORY_SIZE] ;
char window[VGA_MAX_COL*VGA_MAX_ROW] ;
#else
#include "malloc.h"
char *history;
char *window ;
#endif

uint32_t history_cnt           = 0 ;
uint32_t max_window_size       = 0 ;
uint32_t window_first_history  = 0 ;  //  represent "how many bytes before first byte in the window"
uint32_t window_last_history   = 0 ;  //  represent "next coming history number"
uint32_t window_size           = 0 ;

char *input_buf;
uint32_t input_cnt = 0;

uint32_t prompt_x = 0 ;
uint32_t status_bar_lines = 1 ;
uint8_t  console_init_flag = 0;

void init_console(){

    const VGA_PARAM* vga = vga_init();

    #ifdef CONSOLE_COL_SIZE
    console_max_col = CONSOLE_COL_SIZE;
    console_max_row = CONSOLE_ROW_SIZE;
    #else
    console_max_col = vga->max_col;
    console_max_row = vga->max_row;
    #endif
  
    max_window_size = (console_max_row-1-status_bar_lines)*console_max_col ;
   
    #ifndef EARLY_INIT
        history = (char*) kmalloc(CONSOLE_HISTORY_SIZE);
        memzero(history,CONSOLE_HISTORY_SIZE);
        window    = (char*) kmalloc(max_window_size);
        input_buf = (char*) kmalloc(INPUT_BUF_SIZE);
    #endif

    console_init_flag = 1;

}


uint32_t prev_line_start(uint32_t next_start){
    
    uint32_t offset      = next_start ;
    uint32_t histroy_min = history_cnt > CONSOLE_HISTORY_SIZE? history_cnt - CONSOLE_HISTORY_SIZE :0 ;
    uint32_t cnt         = 0;

    while( offset > histroy_min   &&  cnt < console_max_col  &&
          !('\n' == history[ (offset-1) % CONSOLE_HISTORY_SIZE ] && offset != next_start )   ){
           -- offset;  
           ++cnt ; 
    }

    return offset;
}


uint32_t next_line_start(uint32_t line_start){
    
    uint32_t offset = line_start;
    uint32_t cnt    = 0 ;
    
    while(  offset < history_cnt   &&  cnt < console_max_col ){

            if( '\n'  ==  history[ offset % CONSOLE_HISTORY_SIZE ] ){
                ++offset;
                break;
            }
            
            ++offset;
            ++cnt;
    }
    
    return offset;
}

static uint32_t inpws_to_rn(uint32_t inp_ws){

    if( inp_ws <= console_max_col){
        return 0;
    }

    return (inp_ws-1)/console_max_col;
}


int  output_window_one_to_vga( uint32_t widx  ,char c){

    uint32_t row =  (widx) / console_max_col + inpws_to_rn(input_cnt) + 1 ;
    uint32_t col =   widx %  console_max_col ;

    if( row  >=  console_max_row - status_bar_lines){
        return -1;
    }

    switch(c){
        case '\n':
            vga_write_char( 0, col , row );
            break;
        default:
            vga_write_char( c, col , row );
    }  

    return 0;
}


// return window size inc amount , ws --> window_size
uint32_t  calc_window_delta( uint32_t ws ,char c ){

    uint32_t  delta = 0;
    uint32_t  new_ws  = ws ;

    switch(c){
        case '\n':
            new_ws = new_ws + console_max_col - 1 + 1;  // plus self /n chracter
            new_ws -=  new_ws % console_max_col;
            delta  = new_ws - ws;
            break;
        default:
            delta = 1;
    }

    return delta;

}


void draw_window(){

    uint32_t widx = 0  ;

    while( widx < max_window_size ){

        if( output_window_one_to_vga(widx,window[widx]) < 0 ) break;
        ++widx;
    }

}


/*
    TODO: optimize this function
*/

void update_window_buffer(){

    uint32_t size = 0 ;
    uint32_t history_index = window_first_history; 

    while( size < max_window_size && history_index < history_cnt  ){
        
        window[size] = history[ history_index % CONSOLE_HISTORY_SIZE];     
        ++size;
        ++history_index;

        if( window[size-1] == '\n'){
            while( size % console_max_col && size < max_window_size){
                window[size] = 0;
                ++size;
            }
        }  
    }

    window_size         = size;
    window_last_history = history_index;

    // filling remain unused position 0
    while( size < max_window_size){
        window[size++] = 0;
    }

}


int  move_down_window_start_position(){
    
    uint32_t next_start = next_line_start(window_first_history);
    
    if( next_start <= window_first_history  ){
        return -1;
    }

    window_first_history = next_start ;

    return 0;
}

int  move_up_window_start_position(){
    
    uint32_t new_start;

    new_start = prev_line_start(window_first_history);

    if(new_start >= window_first_history){
        return -1;
    }

    window_first_history = new_start;

    return 0;
}

void window_move_up(){

    if( move_up_window_start_position() < 0 ){
        return;
    }

    update_window_buffer();
    draw_window();
}



void window_move_down(){

    if( move_down_window_start_position() < 0 ){
        return;
    }

    update_window_buffer();
    draw_window();
}


/*
    1. put new coming character into buffer
    2. check need to update window.last.offset
    3. check whether to update winodw.start.history
*/

void write_console_one(char c){

    uint32_t old_window_size = 0;
    uint32_t i;
    int follow_current = 0;

    if(!console_init_flag){
        init_console();
    }

    if( history_cnt == window_last_history ){  // window is following the latest output
        follow_current = 1 ;    
    }else{
        window_size = 0;
        window_first_history = history_cnt ;
    }

    old_window_size  = window_size;
    window_size += calc_window_delta(window_size,c);

    history[ (history_cnt++) % CONSOLE_HISTORY_SIZE ] = c;
    window_last_history = history_cnt;
    
    if( window_size > max_window_size ){   
        move_down_window_start_position();
        update_window_buffer();
        draw_window();
    }else{

        for( i = old_window_size ; i < window_size -1 ; ++i){
            window[i] =  0 ;
        }

        window[ window_size-1 ] = c;
    }

    if(follow_current){
        output_window_one_to_vga(window_size-1,c);
    }else{
        update_window_buffer();
        draw_window();
    }

}


/*
    current behavior : after clear the output window
    it will follow the latest output
*/

void console_clear(){

    if(!console_init_flag){
        init_console();
    }
    
    window_first_history = window_last_history = history_cnt;
    update_window_buffer();
    draw_window();

}


#ifndef EARLY_INIT

int valid_input_cnt( uint32_t new_input_cnt){

    if( inpws_to_rn(new_input_cnt)  >=  INPUT_AREA_MAX_ROW ){
        return -1;
    }

    return 1;
}

void draw_input_if_need(uint32_t old_input_cnt){

    uint32_t i;

    if( input_cnt >= old_input_cnt){
        
        for( i =  input_cnt - input_cnt %console_max_col ; \
             i < input_cnt - input_cnt %console_max_col + console_max_col ; ++i ){
              vga_write_char( 0 , i %  console_max_col , i / console_max_col );     
        }

        vga_write_char( input_buf[input_cnt-1] , (input_cnt-1) %  console_max_col \
                        , (input_cnt-1) / console_max_col );
        
        return ;
    }

    for( i = 0 ;  i < old_input_cnt ; ++i ){

        if( i< input_cnt ){
            vga_write_char( input_buf[i] , i %  console_max_col , i / console_max_col );
        }
        else{
           vga_write_char( 0 , i %  console_max_col , i / console_max_col ); 
        }
    }
}

int check_resize_layout(uint32_t old_input_cnt, uint32_t new_input_cnt){
    return inpws_to_rn(old_input_cnt) != inpws_to_rn(new_input_cnt);
}

void console_clear_all(){

    if(!console_init_flag){
        init_console();
    }
    
    console_clear_input();
    console_clear();

}


void console_clear_input(){

    uint32_t tmp = input_cnt;
    
    input_cnt = 0;
    draw_input_if_need(tmp);
    draw_window();

}

void flush_input_buffer(){

    // TODO , change to printf with format 
    // %ns not need to be null terminate string !!

    uint32_t i;
    
    for( i = 0 ; i < input_cnt ; ++i ){
        write_console_one(input_buf[i]);
    }
    write_console_one('\n');
    console_clear_input();
}

void get_input_char(char c){

    uint32_t old_cnt = input_cnt;
    uint32_t new_cnt = input_cnt+1;

    if( valid_input_cnt(new_cnt) < 0 ){
        return;
    }

    input_buf[input_cnt] = c;
    input_cnt = new_cnt ;

    if ( check_resize_layout(old_cnt,new_cnt) ){
        draw_input_if_need(old_cnt);
        draw_window();
    }else{
        vga_write_char( c , old_cnt %  console_max_col , old_cnt / console_max_col );
    }
}

void modify_input_cnt(int delta){

    uint32_t new_cnt = 0;
    uint32_t old_cnt = input_cnt;

    if(delta > 0 ){
        new_cnt = input_cnt + (uint32_t) delta;
    }else{
        new_cnt = input_cnt - (uint32_t) (-1*delta);
    }

    if( valid_input_cnt(new_cnt) < 0 ){
        return;
    }

    input_cnt = new_cnt;
    
    draw_input_if_need(old_cnt);
    if ( check_resize_layout(old_cnt,new_cnt) ){
        draw_window();
    }

}

#endif

