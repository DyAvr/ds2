#include "time.h"

timestamp_t lamport_time = 0;

timestamp_t get_lamport_time(){
    return lamport_time;
}

void set_lamport_time(timestamp_t lamp_time){
    if (lamport_time < lamp_time) {
        lamport_time = lamp_time;
    }
}

timestamp_t inc_lamport_time(){
    return ++lamport_time;
}
