#include "algorithm.h"

int my_log = 0;

int request_cs(const void *self) {
    Mesh *mesh = (Mesh*) self;

    if (my_log == 0){
        my_log = open(events_log, O_CREAT | O_WRONLY | O_APPEND, 0644);
    }

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
            queue->replies[i] = 0;
        } else {
            queue->replies[i] = 1;
        }
    }
    send_multicast(mesh, &release);
    
    return 0;
}

int exitMutex(Mesh *mesh) {
    for (int i = 1; i <= mesh->processes_count; i++){
        if (queue->released[i] == 0){
            return 0;
        } 
    }
    return 1;
}

int isProcessInMutex(Mesh* mesh){
    char buf[255];
    if (peek().l_id == mesh->current_id) {
        for(int i = 1; i <= mesh->processes_count; i++) {
            if (queue->replies[i] == 0){
                sprintf(buf, "REPLIES X: %d\n", mesh->current_id);
                write(my_log, buf, strlen(buf));
                return 0;
            }
        }
    } else {
        sprintf(buf, "NOT FIRST X: %d\n", mesh->current_id);
        write(my_log, buf, strlen(buf));
        return 0;
    }
    return 1;
}
