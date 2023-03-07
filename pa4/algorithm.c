#include "algorithm.h"

int request_cs(const void *self) {
    Mesh *mesh = (Mesh*) self;

    Message request = createMessage(MESSAGE_MAGIC, 0, 0, 0, CS_REQUEST);
    Request req = createRequest(mesh->current_id, get_lamport_time());
    push(req);
    send_multicast(mesh, &request);
    
    while (!isProcessInMutex(mesh)) {
        Message received;
        local_id from = receiveAny(mesh, &received);
        
        if (from > 0) {
            switch (received.s_header.s_type) {
                case CS_REQUEST: {                    
                    req = createRequest(from, received.s_header.s_local_time);
                    push(req);

                    Message response = createMessage(MESSAGE_MAGIC, 0, 0, 0, CS_REPLY);
                    send(mesh, from, &response);
                    break;
                }
                case CS_REPLY: {
                    queue->replies[from] = 1;
                    break;
                }
                case CS_RELEASE: {
                    if (peek().l_id == from) {
                        pop();
                    }
                    break;
                }
                case DONE: {
                    queue->released[from] = 1;
                    break;
                }
            }
        }
    }
    return 0;
}

int release_cs(const void *self) {
    Mesh *mesh = (Mesh*) self;

    pop();    
    Message release = createMessage(MESSAGE_MAGIC, 0, 0, 0, CS_RELEASE);
    for (int i = 1; i <= mesh->processes_count; i++) {
        if (i == mesh->current_id){
            queue->replies[i] = 1;
        } else {
            queue->replies[i] = 0;
        }
    }
    send_multicast(mesh, &release);
    
    return 0;
}

int allProcessesDone(Mesh *mesh) {
    for (int i = 1; i <= mesh->processes_count; i++){
        if (queue->released[i] == 0){
            return 0;
        } 
    }
    return 1;
}

int isProcessInMutex(Mesh* mesh){
    if (peek().l_id == mesh->current_id) {
        for(int i = 1; i <= mesh->processes_count; i++) {
            if (queue->replies[i] == 0){
                return 0;
            }
        }
    } else {
        return 0;
    }
    return 1;
}

void waitForAllDone(Mesh* mesh) {
    // Message msg;
    // inc_lamport_time();
    // for(int i = 1; i <= mesh->processes_count; i++) {
    //     if (i != mesh->current_id){
    //         int status = receive(mesh, i, &msg);
    //         if (status == 0 && msg.s_header.s_type != DONE){
    //             i--;
    //         } else if (status == 1){
    //             exit(1);
    //         } else if (status == 2){
    //             i--;
    //         } 
    //         if (status == 0){
    //             set_lamport_time(msg.s_header.s_local_time);
    //         }
    //     }
    // }
    while (!allProcessesDone(mesh)){
        Message received;
        local_id from = receiveAny(mesh, &received);
        if (from != -1){
            if (mesh->current != mesh->parent){
                handleCSMessages(mesh, &received, from);
            }
            handleDoneMessages(&received, from);
        }
    }

    logEvent(EVENT_RECEIVED_ALL_DONE, mesh->current_id, 0, get_lamport_time());
}
