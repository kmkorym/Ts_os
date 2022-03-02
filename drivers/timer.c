#include "../lib/print.h"
#include "../kernel/common.h"
#include "../kernel/task.h"

#define TIMER_FREQUENCY 25

uint32_t secs=0;
uint32_t freq_counter=0;
extern struct Task *current;

void timer_handler(){
    freq_counter+=1;
    if(freq_counter>=TIMER_FREQUENCY){
        secs+=1;
        freq_counter=0; 
    }
    current->ttl-=1;
    if(!current->ttl){
        current->state |= TASK_NEED_SCHED;
    }
}


void init_timer(){
    // initialoze frequnecy
    //current frequency is 
    uint32_t divisor = 1193180 /  TIMER_FREQUENCY;

    // Send the command byte.
    outb(0x43, 0x36);

    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

    // Send the frequency divisor.
    outb(0x40, l);
    outb(0x40, h);
}