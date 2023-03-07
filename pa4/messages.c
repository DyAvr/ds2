#include "messages.h"

Message createMessage(uint16_t magic, balance_t balance, local_id from, local_id to, MessageType type){
    Message message;
    MessageHeader messageHeader;
    char *buf = (char*)malloc(sizeof(char) * 255); 

    timestamp_t time = inc_lamport_time();
    
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
    logEvent(EVENT_STARTED, mesh->current_id, 0, get_lamport_time());
    Message msg = createMessage(MESSAGE_MAGIC, 0, mesh->current_id, 0, STARTED);
    for (int i = 1; i <= mesh->processes_count; i++){
        if (i != mesh->current_id){
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
    logEvent(EVENT_RECEIVED_ALL_STARTED, mesh->current_id, 0, get_lamport_time());
}

void sendDoneSignal(Mesh* mesh) {
    logEvent(EVENT_DONE, mesh->current_id, 0, get_lamport_time());
    Message msg = createMessage(MESSAGE_MAGIC, 0, mesh->current_id, 0, DONE);
    if(send_multicast(mesh, &msg) != 0) {
        printf("Can't send multicast");
        exit(1);
    }
}

// void handleCSMessages(Mesh* mesh, Message *msg, local_id from){
//     switch (msg->s_header.s_type){
//         case CS_REQUEST: {
//             Message response = createMessage(MESSAGE_MAGIC, 0, 0, 0, CS_REPLY);
//             Request request = createRequest(from, msg->s_header.s_local_time);
//             push(request);
//             send(mesh, from, &response);
//             break;
//         }
//         case CS_REPLY:{
//             queue->replies[from] = 1;
//             break;
//         }
//         case CS_RELEASE:{
//             if (peek().l_id == mesh->current_id){
//                 pop();
//             }
//             break;
//         }
//         default:
//             break;
//     }
// }

void handleDoneMessages(Message *msg, local_id from){
    if (msg->s_header.s_type == DONE){
        queue->released[from] = 1;
    }
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
    
    inc_lamport_time();
}

local_id receiveAny(Mesh *mesh, Message *msg) {
    for (local_id i = 1; i <= mesh->processes_count; i++) {
        if (i != mesh->current_id) {
            int status = receive(mesh, i, msg);
            if (status == 0) {
                set_lamport_time(msg->s_header.s_local_time);
                return i;
            }
        }
    }
    return -1;
}
