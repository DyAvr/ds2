#include "queue.h"

Queue *queue;

void initQueue(Mesh* mesh){
    queue = (Queue*)malloc(sizeof(Queue));
    queue->length = 0;
    queue->replies[mesh->current_id] = 1;
    queue->released[mesh->current_id] = 1;
}

void push(Request request) {
    if (queue->length == 0) {
        queue->requests[queue->length++] = request;
        return;
    }
    queue->length++;
    for (int i = 0; i < queue->length - 1; i++) {
        int cmp = compare(request, queue->requests[i]) ;
        if (cmp == -1) {
            for (int j = queue->length - 2; j >= i; j--) {
                queue->requests[j + 1] = queue->requests[j];
            }
            queue->requests[i] = request;
            return;
        }
    }
    queue->requests[queue->length - 1] = request;
}

void pop() {
    for (int i = 0; i < queue->length - 1; i++) {
        queue->requests[i] = queue->requests[i + 1];
    }
    queue->length = queue->length - 1;
}

Request peek(){
    return queue->requests[0];
}

int compare(Request r1, Request r2){
    if (r1.l_time == r2.l_time) {
        if (r1.l_id > r2.l_id){
            return 1;
        } else if (r1.l_id == r2.l_id){
            return 0;
        } else {
            return -1;
        }
    } else {
        if (r1.l_time > r2.l_id){
            return 1;
        } else {
            return -1;
        }
    }
}

Request createRequest(local_id id, timestamp_t time) {
    Request request;
    request.l_id = id;
    request.l_time = time;
    return request;
}
