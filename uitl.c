#include "print.h"

int fake_print(){
    return 0x1234;
}

// test for parameter can ignore
void test_call(int x,int y){
    int sum = x+y;
    int  sum2 = sum+2;
    print_hex(sum);
    print_hex(sum2);
}