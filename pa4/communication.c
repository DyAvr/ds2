#include "communication.h"

Mesh *mesh;

void initialize(int processes_count, int with_cs){
    initLogger();
    initMesh(processes_count);
    createMeshProcesses(with_cs);
}

void initMesh(int processes_count){
    mesh = (Mesh*)malloc(sizeof(Mesh));
    if (mesh == NULL){
        perror("Failed to allocate memory");
        exit(1);
    }
    mesh->processes_count = processes_count;
    mesh->current = getpid();
    mesh->parent = getpid();
    createMeshPipes();
}

void createMeshProcesses(int with_cs){
    pid_t pid;
    for (int i = 1; i <= mesh->processes_count; i++) {
        pid = fork();
        if (pid == -1) {
            printf("Cannot create fork process");
            exit(1);
        } else if (pid == 0) {
            mesh->current_id = i;
            mesh->current = pid;
            break;
        } 
    }
    
    initQueue(mesh);
    closeUnusedPipes();

    if (mesh->parent == mesh->current){
        startParent();
    } else {
        startChild(with_cs);
    }

    closeLineCommunication();

    if (mesh->parent == mesh->current){
        waitAllChilds();
    }
}

void createMeshPipes(){
    for (int i = 0; i <= mesh->processes_count; i++) {
        for (int j = 0; j <= mesh->processes_count; j++) {
            if (i != j) {
                mesh->pipes[i][j] = (Pipe*)malloc(sizeof(Pipe));
                if (mesh->pipes[i][j] == NULL){
                    perror("Failed to allocate memmory for pipe");
                    exit(1);
                }
            }
        }
    }
    int fdPipes[2];
    for (int i = 0; i <= mesh->processes_count; i++) {
        for (int j = 0; j <= mesh->processes_count; j++) {
            if (i != j) {
                createSinglePipe(fdPipes);
                setPipeFileDescriptors(i, j, fdPipes[0], fdPipes[1], mesh);
                logPipe(PIPE_STARTED, i, j, fdPipes[0], fdPipes[1]);
            }
        }
    }
}

void startChild(int with_cs){
    //start
    Message msg = sendStartedSignal(mesh);
    waitForAllStarted(mesh);
    msg.s_header.s_local_time = inc_lamport_time();
    if (send(mesh, 0, &msg) != 0){
        exit(1);
    }
    //work
    workChild(with_cs);
    //ending
    sendDoneSignal(mesh);
    waitForAllDone(mesh);
}

void startParent(){
    //start
    waitForAllStarted(mesh);
    //work

    //ending
    waitForAllDone(mesh);
}

void workChild(int with_cs){
    int iterations = mesh->current_id * 5;

    for (int i = 1; i <= iterations; i++) {
        if (with_cs){
            request_cs(mesh);
        }
        logEvent(EVENT_LOOP_OPERATION, mesh->current_id, i, iterations);
        if (with_cs){
            release_cs(mesh);
        }
    }
}

void createSinglePipe(int* fdPipes){
    if (pipe(fdPipes) == -1) {
        perror("pipe creation failed");
        exit(1);
    }
}

void setPipeFileDescriptors(int process1, int process2, int fdRead, int fdWrite, Mesh* mesh){
    mesh->pipes[process1][process2]->fdRead = fdRead;
    mesh->pipes[process1][process2]->fdWrite = fdWrite;
    if (fcntl(fdRead, F_SETFL, O_NONBLOCK)<0){
        perror("pipe fail nonblock");
        exit(2);
    }
    if (fcntl(fdWrite, F_SETFL, O_NONBLOCK)<0){
        perror("pipe fail nonblock");
        exit(2);
    }
}

void waitAllChilds(){
    int value;
    for (int i = 0; i < mesh->processes_count; i++){
        if (wait(&value) == -1){
            exit(1);
        }
    }
}

void closeLineCommunication(){
    for (int i = 0; i < mesh->processes_count; i++) {
        if (i != mesh->current_id) {
            close(mesh->pipes[i][mesh->current_id]->fdRead);
            close(mesh->pipes[mesh->current_id][i]->fdWrite);
            logPipe(PIPE_CLOSED, i, mesh->current_id, mesh->pipes[i][mesh->current_id]->fdRead, 0);
            logPipe(PIPE_CLOSED, mesh->current_id, i, 0, mesh->pipes[mesh->current_id][i]->fdWrite);
        }
    }
}

void closeUnusedPipes(){
    for (int i = 0; i <= mesh->processes_count; i++) {
        for (int j = 0; j <= mesh->processes_count; j++) {
            if (i != j) {
                if (i != mesh->current_id){
                    close(mesh->pipes[i][j]->fdWrite);
                    logPipe(PIPE_CLOSED, i, j, 0, mesh->pipes[i][j]->fdWrite);
                }
                if (j != mesh->current_id){
                    close(mesh->pipes[i][j]->fdRead);
                    logPipe(PIPE_CLOSED, i, j, 0, mesh->pipes[i][j]->fdRead);
                }
            }
        }
    }
}
