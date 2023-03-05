#include "communication.h"

void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount)
{
    Mesh *mesh = (Mesh*) parent_data;
    
    Message msg = createMessage(MESSAGE_MAGIC, amount, src, dst, TRANSFER);
    if (send(mesh, src, &msg) != 0){
        exit(1);
    }

    waitAnyMessage(&msg, mesh, dst);
    if (msg.s_header.s_type != ACK){
        exit(1);
    }

    set_lamport_time(msg.s_header.s_local_time);
}

int main(int argc, char * argv[])
{
    int p_count;
    
    if (argc > 4 && strcmp(argv[1], "-p") == 0) {
        p_count = (int)atoi(argv[2]);
    } 
    else {
        return 1;
    }
    
    initialize(p_count, argv);
    
    return 0;
}
