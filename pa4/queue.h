#include "utils.h"

extern Queue* queue;

void initQueue(Mesh* mesh);
void push(Request req);
void pop();
Request peek();
int compare(Request r1, Request r2);
Request createRequest(local_id id, timestamp_t time);

