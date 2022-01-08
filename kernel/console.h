#define MAX_INPUT_LINES 3
#ifdef EARLY_INIT
#define CONSOLE_HISTORY_SIZE 500
#else
#include "mem.h"
#define CONSOLE_HISTORY_SIZE 100
#define CONSOLE_COL_SIZE 20
#define CONSOLE_ROW_SIZE 6
#define INPUT_BUF_SIZE 512
#define INPUT_AREA_MAX_ROW 2
#endif
#define TEXT_COLOR  0xF
#define BG_COLOR 0x0


void window_move_up();
void window_move_down();
void write_console_one(char c);
void console_clear();
void init_console();

#ifndef EARLY_INIT
void console_clear_all();
void console_clear_input();
void flush_input_buffer();
void get_input_char(char c);
void modify_input_cnt(int delta);
#endif