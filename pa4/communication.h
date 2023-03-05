#include <fcntl.h>

#include "messages.h"

void initialize(int processes_count, int with_cs);
void initMesh(int processes_count);

void createMeshProcesses();
void createMeshPipes();

void startChild();
void startParent();

void closeLineCommunication();
void closeUnusedPipes();

void createSinglePipe(int* fdPipes);
void setPipeFileDescriptors(int process1, int process2, int fdRead, int fdWrite, Mesh* mesh);

void workChild();
void handle_transfer(Message* received_msg);
void waitAllChilds();
