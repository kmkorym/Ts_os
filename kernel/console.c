/*
 * current implemation has 1MB limted size for kernel image because must load at real mode on floppy disk
 * and bss section is in data section so using kmalloc for screen buffer
 */

#include "../kernel/vga.h"
#include "console.h"
#include "malloc.h"
#include "../lib/print.h"
#include "../lib/string.h"
#include "common.h"

/*
    console:
    [input area] ( TODO: input area can exapnd while input is much than one line)
    [output area] (it's user view of screen buffer and can lift up or down)
                  (when )
    [last line for console status and its error message]

    Principal:
    1. a history buffer content most N bytes of console history
    2. what user see on the console is a window buffer , the content of window from
        - history content select by user 
        - the latest output to console
    3. output the window contetn from left up corner of console and output as more as possible

    4. user can view history content by press UP/DOWN key
       - UP/DOWN key show previous/next one row
    5. user can clean window
    6. a row is termating by MAX_COL or \n, and start of row is terminating of previous row
    7. the lateset output of console will shown to
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


uint32_t console_max_col = 0 ;
uint32_t console_max_row  = 0 ;
uint32_t cosole_ctl_flag = 0;

char *history;
uint32_t history_cnt = 0;

char *window ;
uint32_t max_window_size       = 0 ;
uint32_t window_first_history  = 0 ;
uint32_t window_last_history   = 0 ; // this is open interval, represent "next coming history number"
uint32_t window_size    = 0 ;

//uint32_t view_row_num = 0;
// view_size ...

//char *   input_buffer;
uint32_t input_area_lines = 1 ;
uint32_t prompt_x = 0 ;
uint32_t status_bar_lines = 1 ;



void init_console(){

    const VGA_PARAM* vga = vga_init();

    console_max_col = vga->max_col;
    console_max_row = vga->max_row;
    
    history = (char*) malloc(CONSOLE_HISTORY_SIZE);
    memzero(history,CONSOLE_HISTORY_SIZE);

    window    = (char*) malloc(vga->max_col*(console_max_row-input_area_lines-status_bar_lines));
    max_window_size = (console_max_col-input_area_lines-status_bar_lines)*console_max_row ;
    
    //input_buffer = (char*) malloc(MAX_INPUT_LINES*vga->max_col);
    //memzero(input_buffer,MAX_INPUT_LINES*vga->max_col);
}

/*
static inline char * get_history_by_offset(uint32_t offset){
    return & history[offset%CONSOLE_HISTORY_SIZE];
}
static inline char * get_view_row(uint32_t row){
    return view+ (row*console_max_col);
}

*/

static inline uint32_t get_history_min(){
    return history_cnt > CONSOLE_HISTORY_SIZE? history_cnt - CONSOLE_HISTORY_SIZE :0 ;
}

uint32_t line_start_history(uint32_t line_end_offset){
    
    uint32_t offset      = line_end_offset ;
    uint32_t histroy_min = get_history_min() ;
    uint32_t cnt         = 1 ;

    if( histroy_min >= offset ){
        return histroy_min ;
    }

    while( offset > histroy_min   &&  cnt < console_max_col  &&
           '\n' != history[ (offset-1) % CONSOLE_HISTORY_SIZE ]   ){
           -- offset;  
           ++cnt ; 
    }

    return offset;
}

// this end is close interval

uint32_t line_end_history(uint32_t line_start_offset){
    
    uint32_t offset = line_start_offset;
    uint32_t cnt = 1;
    
    if(! history_cnt ){
        return 0;
    }

    if( offset >= history_cnt-1 ){
        return offset;
    }

    while(  offset < history_cnt - 1  &&  cnt < console_max_col  &&
            '\n' !=  history[ offset % CONSOLE_HISTORY_SIZE ] ){
            ++offset;
            ++cnt;
    }
    
    return offset;
}


void draw_window(){

    uint32_t vga_row    = input_area_lines;
    uint32_t window_row = 0;

    while( vga_row  < console_max_col - status_bar_lines ){
        vga_print_row(vga_row,window+console_max_col*window_row);
        ++window_row;
        ++vga_row;
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
    
    uint32_t line_end = line_end_history(window_first_history);
    
    if( line_end+1 >= history_cnt  ){
        return -1;
    }

    window_first_history = line_end+1;

    return 0;
}

int  move_up_window_start_position(){
    
    uint32_t new_start;

    if( ! window_first_history ){
        return -1;
    }

    new_start = line_start_history(window_first_history-1);

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

    int follow_current = 0;

    history[ (history_cnt++) % CONSOLE_HISTORY_SIZE ] = c;

    if( history_cnt == window_size+1 ){  // window is following the latest output
        window_size++;
        follow_current = 1;    
    }else{
        window_first_history = history_cnt-1;
        window_size = 1 ;
    }

    window_last_history = history_cnt;
    
    if( window_size > max_window_size ){   
        move_down_window_start_position();
    }

    if(follow_current){
        switch(c){
            case '\n':
                window_size = window_size + console_max_col - 1 +  (window_size+console_max_col-1) % console_max_col;
                break;
            default:
                vga_write_char( c, (window_size-1) % console_max_col , (window_size-1)/console_max_col );
        }
    }else{
        update_window_buffer();
        draw_window();
    }

}
