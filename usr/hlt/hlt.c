int main(){
    int i=5;
    while(i>0){
        asm volatile ("sti");
        --i;
    }
    return 0;
}