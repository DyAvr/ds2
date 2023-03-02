#include "communication.h"

Mesh *mesh;

void initialize(int processes_count){
    initLogger();
    //printf("initLogger\n");
    initMesh(processes_count + 1);
    //printf("initMesssg\n");
    createMeshProcesses();
    //printf("the end :)\n");
}

void initMesh(int processes_count){
    mesh = (Mesh*)malloc(sizeof(Mesh));
    if (mesh == NULL){
        perror("Failed to allocate momory");
        exit(1);
    }
    mesh->processes_count = processes_count;
    mesh->current_id = getpid();
    createMeshPipes();
}

void createMeshProcesses(){
    pid_t pid;
    for (int i = 1; i < mesh->processes_count; i++) {
        pid = fork();
        if (pid == -1) {
            printf("Cannot create fork process");
            exit(1);
        } else if (pid == 0) {
            //printf("startChild\n");
            mesh->current_id = i;
            // for (int j = 0; j < mesh->processes_count; j++){
            //     if (j!= i){
            //         close(mesh->pipes[j][i]->fdRead);
            //     }
            // }
            startChild();
            exit(0);
        } else {
            for (int j = 0; j < mesh->processes_count; j++){
                if (j!= i){
                    close(mesh->pipes[i][j]->fdWrite);
                }
            }
        }
    }
    mesh->current_id = 0;
    //printf("startParent\n");
    startParent();
}

void createMeshPipes(){
    //printf("count: %d\n", mesh->processes_count);
    //printf("cur_id: %d\n", mesh->current_id);
    for (int i = 0; i < mesh->processes_count; i++) {
        for (int j = 0; j < mesh->processes_count; j++) {
            if (i != j) {
                mesh->pipes[i][j] = (Pipe*)malloc(sizeof(Pipe));
                if (mesh->pipes[i][j] == NULL){
                    perror("Failed to allocate memmory for pipe");
                    exit(1);
                }
                //printf("pipe (%d, %d) init\n", i, j);
            }
        }
    }
    int fdPipes[2];
    for (int i = 0; i < mesh->processes_count; i++) {
        for (int j = 0; j < mesh->processes_count; j++) {
            if (i != j) {
                //printf("good: %d,%d\n", i, j);
                createSinglePipe(fdPipes);
                //printf("pipe created \n");
                //printf("ds: %d, %d\n", fdPipes[0], fdPipes[1]);
                setPipeFileDescriptors(i, j, fdPipes[0], fdPipes[1], mesh);
                logPipe(mesh->current_id, i, j, fdPipes[0], fdPipes[1]);
            }
        }
    }
}

void startChild(){
    closeUnusedPipes();
    //printf("c_close_u\n");
    handleEvent(EVENT_STARTED, mesh);
    //printf("c_started\n");
    handleEvent(EVENT_DONE, mesh);
    //printf("c_done\n");
    closeLineCommunication();
    //printf("c_close_l\n");
}

void startParent(){
    closeUnusedPipes();
    handleEvent(EVENT_RECEIVED_ALL_STARTED, mesh);
    //printf("all_started\n");
    handleEvent(EVENT_RECEIVED_ALL_DONE, mesh);
    //printf("all_done\n");
    int value = wait(NULL);
    while (value > 0) {
        value = wait(NULL);
    }
    //printf("p_wait\n");
    closeUnusedPipes();
    //printf("p_closed_u\n");
    closeLogger();
    //printf("p_close_log\n");
    exit(0);
}

void closeLineCommunication(){
    for (int i = 0; i < mesh->processes_count; i++) {
        if (i != mesh->current_id) {
            closeSinglePipe(mesh->pipes[mesh->current_id][i]);
        }
    }
}

void closeUnusedPipes(){
    for (int i = 0; i < mesh->processes_count; i++) {
        for (int j = 0; j < mesh->processes_count; j++) {
            if (isProcessPairUnused(i, j, mesh->current_id)) {
                closeSinglePipe(mesh->pipes[i][j]);
            }
        }
    }
}

void createSinglePipe(int* fdPipes){
    //printf("ds in create: %d, %d\n", fdPipes[0], fdPipes[1]);
    if (pipe(fdPipes) == -1) {
        perror("pipe creation failed");
        exit(1);
    }
    //printf("ds after create: %d, %d\n", fdPipes[0], fdPipes[1]);
}

void setPipeFileDescriptors(int process1, int process2, int fdRead, int fdWrite, Mesh* mesh){
    mesh->pipes[process2][process1]->fdRead = fdRead;
    mesh->pipes[process1][process2]->fdWrite = fdWrite;
}

void closeSinglePipe(Pipe* pipe){
    close(pipe->fdRead);
    close(pipe->fdWrite);
    logPipe(mesh->current_id, 0, 0, pipe->fdRead, pipe->fdWrite);
}

bool isProcessPairUnused(int process1, int process2, int current_id){
    return process1 != process2 && process1 != current_id;
}
