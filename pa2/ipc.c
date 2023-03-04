#include <errno.h>

#include "utils.h"

int send(void * self, local_id dst, const Message * msg){
    Mesh *mesh = (Mesh*) self;
    Pipe *pipe = mesh->pipes[mesh->current_id][dst];

    size_t total_size = sizeof(MessageHeader) + msg->s_header.s_payload_len;
    int bytes_written = write(pipe->fdWrite, msg, total_size);

    if (bytes_written != total_size) {
        return 1;
    }

    //printf("SEND: %d->%d type:%d\n", mesh->current_id, dst, msg->s_header.s_type);
    return 0;
}

int send_multicast(void * self, const Message * msg){
    Mesh *mesh = (Mesh*) self;

    for (int dst_id = 0; dst_id <= mesh->processes_count; dst_id++) {
        if (dst_id != mesh->current_id) {
            if (send(mesh, dst_id, msg) == 1){
                return 1;
            }
        }
    }

    return 0;
}

int receive(void * self, local_id from, Message * msg){
    Mesh *mesh = (Mesh*) self;
    Pipe *pipe = mesh->pipes[from][mesh->current_id];

    MessageHeader header;
    int bytes_read = read(pipe->fdRead, &header, sizeof(MessageHeader));
    if (bytes_read != sizeof(MessageHeader)) {
        if (bytes_read == -1 &&  errno == EAGAIN){
            return 2;
        }
        return 1;
    }

    char payload_buffer[header.s_payload_len];
    int payload_bytes_read;
    while (1){
        payload_bytes_read = read(pipe->fdRead, payload_buffer, header.s_payload_len);
        if (payload_bytes_read == header.s_payload_len) {
            break;
        } 
        if (payload_bytes_read == -1 && errno == EAGAIN){
            continue;
        } 

        return 1;
    }

    msg->s_header = header;
    memcpy(msg->s_payload, payload_buffer, header.s_payload_len);

    //printf("RECEIVE: %d<-%d type:%d\n", mesh->current_id, from, header.s_type);
    return 0;
}

int receive_any(void * self, Message * msg){
    Mesh *mesh = (Mesh*) self;

    for (local_id i = 0; i <= mesh->processes_count; i++) {
        if (i != mesh->current_id) {
            int status = receive(mesh, i, msg);
            if (status == 0) {
                return 0;
            }

            if (status == 2) {
                continue;
            }

            return 1;
        }
    }

    return 2;
}
