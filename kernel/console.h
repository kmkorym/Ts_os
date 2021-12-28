//#define CONSOLE_HISTORY_SIZE (4096*2)
#define MAX_INPUT_LINES 3
#ifdef EARLY_INIT
#define CONSOLE_HISTORY_SIZE 2500
#else
#include "mem.h"
#define CONSOLE_HISTORY_SIZE 2500
#endif
#define TEXT_COLOR  0xF
#define BG_COLOR 0x0

void write_console_one(char c);
void clear();