//#define CONSOLE_HISTORY_SIZE (4096*2)
#define MAX_INPUT_LINES 3
#define CONSOLE_HISTORY_SIZE 2500
#define TEXT_COLOR  0xF
#define BG_COLOR 0x0

struct ConsoleLine{
    uint32_t offset;
    uint32_t line;
};
typedef struct ConsoleLine ConsoleLine;