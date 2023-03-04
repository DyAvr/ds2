#include "messages.h"

Message createMessage(uint16_t magic, balance_t balance, local_id from, local_id to, MessageType type){
    Message message;
    MessageHeader messageHeader;
    TransferOrder order;
    char *buf = (char*)malloc(sizeof(char) * 255); 

    timestamp_t time = get_lamport_time();
    
    switch (type){
        case STARTED:
            sprintf(buf, log_started_fmt, time, from, getpid(), getppid(), balance);
            messageHeader = createMessageHeader(magic, strlen(buf), type, time);
            message.s_header = messageHeader;
            strcpy(message.s_payload, buf);
            return message;
        case DONE:
            sprintf(buf, log_done_fmt, time, from, balance);
            messageHeader = createMessageHeader(magic, strlen(buf), type, time);
            message.s_header = messageHeader;
            strcpy(message.s_payload, buf);
            return message;
        case TRANSFER:
            order = createTransferOrder(from, to, balance);
            messageHeader = createMessageHeader(magic, sizeof(TransferOrder), type, time);
            message.s_header = messageHeader;
            memcpy(message.s_payload, &order, sizeof(TransferOrder));
            return message;
        default:
            messageHeader = createMessageHeader(magic, 0, type, time);
            message.s_header = messageHeader;
            return message;
    }
}

MessageHeader createMessageHeader(uint16_t magic, uint16_t p_len, int16_t type, timestamp_t time){
    MessageHeader header;
    header.s_magic = MESSAGE_MAGIC;
    header.s_payload_len = p_len;
    header.s_type = type;
    header.s_local_time = time;
    return header;
}

Message sendStartedSignal(Mesh* mesh) {
    Message msg;
    inc_lamport_time();
    logEvent(EVENT_STARTED, mesh->current_balance, mesh->current_id, 0);
    for (int i = 1; i <= mesh->processes_count; i++){
        if (i != mesh->current_id){
            msg = createMessage(MESSAGE_MAGIC, mesh->current_balance, mesh->current_id, 0, STARTED);
            if (send(mesh, i, &msg) != 0){
                exit(1);
            }
        }
    }
    
    return msg;
}

void waitForAllStarted(Mesh* mesh) {
    Message msg;
    inc_lamport_time();
    for(int i = 1; i <= mesh->processes_count; i++) {
        if (i != mesh->current_id){
            int status = receive(mesh, i, &msg);
            //printf("WAS: %d, i: %d, status: %d\n",mesh->current_id, i, status);
            if (status == 0 && msg.s_header.s_type != STARTED){
                exit(2);
            } else if (status == 1){
                exit(1);
            } else if (status == 2){
                i--;
            } else if (status == 0){
                set_lamport_time(msg.s_header.s_local_time);
            }
        }
    }
    logEvent(EVENT_RECEIVED_ALL_STARTED, 0, mesh->current_id, 0);
}

void sendDoneSignal(Mesh* mesh) {
    inc_lamport_time();
    Message msg = createMessage(MESSAGE_MAGIC, mesh->current_balance, mesh->current_id, 0, DONE);
    if(send_multicast(mesh, &msg) != 0) {
        printf("Can't send multicast");
        exit(1);
    }
}

void waitForAllDone(Mesh* mesh) {
    Message msg;
    inc_lamport_time();
    for(int i = 1; i <= mesh->processes_count; i++) {
        if (i != mesh->current_id){
            int status = receive(mesh, i, &msg);
            if (status == 0 && msg.s_header.s_type != DONE){
                i--;
            } else if (status == 1){
                exit(1);
            } else if (status == 2){
                i--;
            } else if (status == 0){
                set_lamport_time(msg.s_header.s_local_time);
            }
        }
    }
    logEvent(EVENT_RECEIVED_ALL_DONE, 0, mesh->current_id, 0);
}


TransferOrder createTransferOrder(local_id from, local_id to, balance_t balance){
    TransferOrder order;
    order.s_src = from;
    order.s_dst = to;
    order.s_amount = balance;
    return order;
}

void waitAnyMessage(Message *msg, Mesh *mesh, local_id from){
    inc_lamport_time();
    int status;
    while (1){
        status = receive(mesh, from, msg);
        
        if (status == 0) {
            set_lamport_time(msg->s_header.s_local_time);        
            break;
        } else if (status == 1){
            exit(1);
        }
    }
}

void sendStopSignal(Mesh* mesh) {
    inc_lamport_time();
    Message send = createMessage(MESSAGE_MAGIC, mesh->current_balance, mesh->current_id, mesh->parent_id, STOP);
    if(send_multicast(mesh, &send) != 0) {
        printf("Can't send multicast");
        exit(1);
    }
}
