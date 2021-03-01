#include "../lib/string.h"


void test_string_main(){
    // test strcpy
    char src[1024];
    char* a = "";
    char* b = "8902";
    char* c = "1234 5678";
    char* d="1234 5678 90 ";
    char buffer[256]={0};
    char * start=NULL;
    int i=0;
    char* ans1[3]={"1234","5678","90"};
    

    /*
    strcpy(src,a);
    if(src[0]!=0){
        printl("strcpy: error 1 ");
    }
    */
    
    strcpy(src,b);
    if(!string_equal(src,b)){
        printl("strcpy: error 2 ");
    }
    
    
    // test  strlen
    if(strlen(a)){
        printl("strlen: error 1 ");
    }
    if(9!=strlen(c)){
        printl("strlen: error 1 ");
    }

    //strfd
    if(strfd(c,"234 ")!=&c[1]){
        printl("strfd: error 1 ");
    }
    if(NULL!=strfd(c,"234 6")){
        printl("strfd: error 2 ");
    }

    //strtok
    if(strtok("","",buffer)){
        printl("strtok: error  1");
    }else if(*buffer){
        printl("strtok: error  2");
    }
    if(strtok("","1234",buffer)){
        printl("strtok: error  3");
    }
    if(strtok("1234","",buffer)){
        printl("strtok: error  4-1");
    }else if(!string_equal("1234",buffer)){
        printl("strtok: error  4-2");
    }
    start = d;
    while(start){
        start=strtok(start," ",buffer);
        if(!string_equal(ans1[i],buffer)){
            printl("strtok: error 5");
            printl(ans1[i]);
            printl(buffer); 
        }
        ++i;
    }
    if(i!=3){
        print_hex(i);
        printl("strtok: error 5: i!=3");
       
    }
    i=0;
    // no trailing pattern
    char* e = "  1234 5678    90  ";
    start =e ;
    while(start){
        start=strtok(start," ",buffer);
        if(!string_equal(ans1[i],buffer)){
            printl("strtok: error 6");
            print_char('[');
            printstr(buffer); 
            print_char(']');
        }
        ++i;
    }
    if(i!=3){
        print_hex(i);
        printl("strtok: error 6: i!=3");
    }
  
    //
    char* num="01234560";
    if(1234560!=atoi(num,10)){
        printl("atoi error 1");
    }

    const char *f="  345678  gh ghh   ";
    if(no_leading(f," ")!=f+2){
        printl("no leading error 1"); 
    }
    if(no_trailing(f," ")!=f+15){
        printl("no traling error 1"); 
    }
}