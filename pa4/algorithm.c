#include "algorithm.h"


void sendRequest(Mesh *mesh, Message *msg){
    for (int i = 1; i <= mesh->processes_count; i++){
        if (i != mesh->current_id){
            if (!queue->released[i]){
                send(mesh, i, msg);
            } else {
                queue->replies[i] = 1;
            }
        }
    }
}

void sendRelease(Mesh *mesh, Message *msg){
    for (int i = 1; i <= mesh->processes_count; i++){
        if (i != mesh->current_id){
            if (!queue->released[i]){
                send(mesh, i, msg);
            }
        }
    }
    
}

int request_cs(const void *self) {
    Mesh *mesh = (Mesh*) self;

    Message request = createMessage(MESSAGE_MAGIC, 0, 0, 0, CS_REQUEST);
    Request req = createRequest(mesh->current_id, get_lamport_time());
    push(req);
    sendRequest(mesh, &request);
    
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
                    queue->replies[from] = 1;
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
            queue->replies[i] = queue->released[i];
        }
    }
    sendRelease(mesh, &release);
    
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
    while (!allProcessesDone(mesh)){
        Message received;
        local_id from = receiveAny(mesh, &received);
        if (from != -1){
            handleDoneMessages(&received, from);
        }
    }

    logEvent(EVENT_RECEIVED_ALL_DONE, mesh->current_id, 0, get_lamport_time());
}
