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
uint32_t window_first_history  = 0 ;
uint32_t window_last_history   = 0 ;
uint32_t window_last_offset    = 0 ;
uint32_t cursor_index          = 0 ;
uint32_t tracker_start     
//uint32_t view_row_num = 0;
// view_size ...

char *   input_buffer;
uint32_t input_area_lines = 1 ;
uint32_t prompt_x = 0 ;
uint32_t status_bar_lines = 1 ;



void init_console(){

    const VGA_PARAM* vga = vga_init();
    console_max_col = vga->max_col;
    console_max_row = vga->max_row;

    history = (char*) malloc(CONSOLE_HISTORY_SIZE);
    memzero(history,CONSOLE_HISTORY_SIZE);

    view    = (char*) malloc(vga->max_col*(console_max_row-input_area_lines-status_bar_lines));
    view_row_num = console_max_row-input_area_lines-status_bar_lines;
    
    input_buffer = (char*) malloc(MAX_INPUT_LINES*vga->max_col);
    memzero(input_buffer,MAX_INPUT_LINES*vga->max_col);
    

}

static inline char * get_view_row(uint32_t row){
    return view+ (row*console_max_col);
}


static inline uint32_t get_history_min(){
    return history_cnt > CONSOLE_HISTORY_SIZE? history_cnt - CONSOLE_HISTORY_SIZE :0 ;
}

static inline char * get_history_by_offset(uint32_t offset){
    return & history[offset%CONSOLE_HISTORY_SIZE];
}

void flush_view_buffer(){

    uint32_t vga_start_row  = input_area_lines;
    uint32_t vga_bottom_row = console_max_row - 1 - status_bar_lines;
    uint32_t cnt = 0;

    while( vga_bottom_row >= vga_start_row ){
        vga_print_row(vga_bottom_row-cnt,get_view_row(view_row_num-1-cnt));
        --vga_bottom_row ; 
        ++cnt;
    }  

}

// ofset must be end  of a history line for max_console_col condition
uint32_t line_start_history_byte(uint32_t offset){
    
    uint32_t histroy_min =  get_history_min();
    uint32_t cnt = console_max_col - 1;


    if( histroy_min >= offset ){
        return histroy_min;
    }


    while( offset > histroy_min   &&  cnt   &&
           '\n' != *get_history_by_offset(offset)   ){
           -- offset;  
           --cnt ; 
    }

    return offset;
}

// ofset must be start of a history line for max_console_col condition
uint32_t line_end_history_byte(uint32_t offset){
    
    uint32_t cnt = console_max_col - 1;
    
    if( offset >= history_cnt - 1 ){
        return offset;
    }

    while( offset < history_cnt - 1  &&  cnt  &&
           '\n' !=  *get_history_by_offset(offset) ){
            ++offset;
            --cnt;
    }
    
    return offset;
}


void  generate_view_row(uint32_t y,uint32_t start_offset,uint32_t end_offset){ 
    
    char * p;
    int i ;
    int new_line = 0; 
    
    p = get_view_row(y);

    if( *get_history_by_offset(i+end_offset) == '\n'){
        new_line = 1;
    }

    if(new_line){
        for(i=0;i<console_max_col;++i){ 
            if( start_offset+i <= end_offset && p[i]!='\n' ){
                p[i] = *get_history_by_offset(i+start_offset) ; 
            }else{
                p[i] = 0;    
            }
        }
    }else{
        for(i=console_max_col-1;i>=0;--i){
            if( end_offset>=start_offset ){
                if(p[i]!='\n'){
                    p[i] = *get_history_by_offset(end_offset) ; 
                }else{
                    p[i] = 0;
                }
                --end_offset;
            }else{
                p[i] = 0;
            }
        }
    }
}

uint32_t generate_view_from_bottom(uint32_t start_x , uint32_t start_y , uint32_t start_offset){
    
    uint32_t start_line = start_offset - start_x;
    uint32_t end_line   = start_offset ;
    uint32_t view_byte_cnt = 0;

    if( start_line < get_history_min() || start_offset >= history_cnt ){
        return;
    }

    uint32_t row_cnt = start_y + 1;
    uint32_t n;

    while(row_cnt){
        n = end_line - start_line +1;
        break;
        generate_view_row(row_cnt-1,start_line,start_line+n-1);
        end_line   = start_line-1;
        start_line = line_start_history_byte(end_line);
        view_byte_cnt+=n;
        --row_cnt;
    }

    return view_byte_cnt;

}

/*
    TODO: optimize these two function
*/

void lift_console_view(){

    if(!view_first_byte){
        return ;
    }

    int i ;
    uint32_t  start_0;

    start_0 = line_start_history_byte(view_first_byte-1);

    if( start_0 == view_first_byte){
        return;
    }
    
    for( i = view_row_num-2; i >= 0 ; --i){
        memcpy(get_view_row(i),get_view_row(i+1),console_max_col);
    }

    generate_view_row(0,start_0,view_first_byte-1);
    view_first_byte = start_0;
   
}

void down_console_view(){

    uint32_t next_view_end = line_end_history_byte(view_last_byte+1);
    int i ;

    for(i=1;i<view_row_num-1;++i){
        memcpy(get_view_row(i),get_view_row(i-1),console_max_col);
    }

    if( next_view_end == view_last_byte){
        memset(get_view_row(view_row_num-1),0,console_max_col);
        return;
    }

    generate_view_row(view_row_num-1,view_last_byte+1,next_view_end);
    view_last_byte = next_view_end;

}


// put input to buffer
// check need to adjust view port to follow current line
//      if need : generate view buffer from history and adjust view XX variable
//      if not  : just print the character
//      backspace only support for input area, printf not working
void write_console_one(char c){

    char * p = get_history_by_offset(history_cnt);
    int follow_cur_flag = (history_cnt-1 == view_last_byte ? 1:0);
    uint32_t view_cnt ;

    *p = c ; 
    ++history_cnt;
    ++cursor_x;

    view_last_byte = history_cnt - 1;

    if(!follow_cur_flag){
        view_cnt  = generate_view_from_bottom(cursor_x-1,view_row_num-1,cursor_x-1);
        view_first_byte = view_last_byte-view_cnt+1;
    }
        
    switch(c){
        case '\n':
            cursor_x = 0;
            down_console_view();
            break;
        default:
            cursor_x = cursor_x % console_max_col;
            break;
    }

    if(!follow_cur_flag){
        flush_view_buffer();
    }else{
        vga_write_char(c,cursor_x-1,console_max_row - 1 - status_bar_lines);
    }

}

void  input_console_one(char c){
    
    switch(c){
        case '\n':



    }

    ++prompt_x;
}

