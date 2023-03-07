#include "messages.h"

int isProcessInMutex(Mesh* mesh);
int allProcessesDone(Mesh *mesh);

void waitForAllDone(Mesh* mesh);
void sendRequest(Mesh *mesh, Message *msg);
void sendRelease(Mesh *mesh, Message *msg);
