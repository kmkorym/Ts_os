#ifndef PRINT_H
#define PRINT_H
unsigned int   __vga_addr(unsigned int x,unsigned int y);
unsigned int    __current_vga_addr();
void __move_cursor_next();
void __move_cursor_next_line();
void set_cursor(unsigned int x,unsigned int y);
void print_char(char c);
void print_chars(char * s,int n);
void printstr(const char * s);
void printl(char * s);
void print_hex(int);
void clear();
void print_screen(char c,unsigned int x,unsigned int y);
void back_char();
void print_byte(unsigned char x);
void printf(char *fmt,...);
#endif
