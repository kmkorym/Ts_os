#define TEXT_COLOR  0xF
#define BG_COLOR 0x0
#define VGA_BASE  0XB8000
#define VGA_MAX_COL 80
#define VGA_MAX_ROW 25

static unsigned int current_x=0;
static unsigned int current_y=0;


void set_cursor(unsigned int x,unsigned int y){
    current_x = x;
    current_y =y;
}

unsigned int  __vga_addr(unsigned int x,unsigned int y){
    return  VGA_BASE+(y * VGA_MAX_COL + x) * 2;
}

unsigned int   __current_vga_addr(){
    return  __vga_addr(current_x,current_y);
}

void __move_cursor_next_line(){
    current_x=0;
     ++current_y;
    if(current_y==VGA_MAX_ROW){
        current_y=0;
    }
}

void __move_cursor_next(){
    ++current_x;
    if(current_x==VGA_MAX_COL){
        current_x=0;
         __move_cursor_next_line();
    }
}

void print_char(char c){
    if(c=='\n'){
        __move_cursor_next_line();
        return;
    }
    char* p = (char*) __current_vga_addr();
    *p=c;
    __move_cursor_next();
}


void printstr(char * s){
    while(*s){
        print_char(*s++);
    }
}

void printl(char * s){
    printstr(s);
    print_char('\n');
}



int main(){

    printl("hello world omg!");   

    return 0;

}