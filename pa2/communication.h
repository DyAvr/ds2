#include "messages.h"
#include "history.h"

void initialize(int processes_count);
void initMesh(int processes_count);

void createMeshProcesses();
void createMeshPipes();

void startChild();
void startParent();

void closeLineCommunication();
void closeUnusedPipes();

void createSinglePipe(int* fdPipes);
void setPipeFileDescriptors(int process1, int process2, int fdRead, int fdWrite, Mesh* mesh);
void closeSinglePipe(Pipe* pipe);

bool isProcessPairUnused(int process1, int process2, int current_id);
