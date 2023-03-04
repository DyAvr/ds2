#include "communication.h"

Mesh *mesh;

void initialize(int processes_count, char * argv[]){
    initLogger();
    initMesh(processes_count);
    initBank(processes_count, argv);
    createMeshProcesses();
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

void createMeshProcesses(){
    pid_t pid;
    for (int i = 1; i <= mesh->processes_count; i++) {
        mesh->current_balance = bank->balances[i-1];
        pid = fork();
        if (pid == -1) {
            printf("Cannot create fork process");
            exit(1);
        } else if (pid == 0) {
            mesh->current_id = i;
            mesh->current = pid;
            initHistory(mesh);
            break;
        } 
    }
    closeUnusedPipes();

    if (mesh->parent == mesh->current){
        startParent();
    } else {
        startChild();
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

void startChild(){
    //star
    sendStartedSignal(mesh);
    waitForAllStarted(mesh);
    inc_lamport_time();
    Message msg = createMessage(MESSAGE_MAGIC, mesh->current_balance, mesh->current_id, 0, STARTED);
    if (send(mesh, 0, &msg) != 0){
        exit(1);
    }
    //work
    workChild();
    //ending
    sendDoneSignal(mesh);
    waitForAllDone(mesh);
    inc_lamport_time();
    sendHistory(mesh);
}

void startParent(){
    //start
    waitForAllStarted(mesh);
    //work
    bank_robbery(mesh, mesh->processes_count);
    sendStopSignal(mesh);
    //ending
    waitForAllDone(mesh);
    inc_lamport_time();
    showHistory(mesh);
}

void workChild(){
    int exit_flag = 1;

    while (exit_flag) {
        Message received_msg;
        inc_lamport_time();
        int status = receive_any(mesh, &received_msg);
        while (status == 2){
            status = receive_any(mesh, &received_msg);
        }

        if (status == 0) {
            switch (received_msg.s_header.s_type) {
                case STOP:
                    set_lamport_time(received_msg.s_header.s_local_time);
                    storeState(inc_lamport_time(), mesh->current_balance, 0);
                    exit_flag = 0;
                    break;
                case TRANSFER:
                    handle_transfer(&received_msg);
                    break;
            }
        }
    }
}

void handle_transfer(Message* received_msg) {
    TransferOrder transfer_order;
    memcpy(&transfer_order, received_msg->s_payload, sizeof(TransferOrder));

    set_lamport_time(received_msg->s_header.s_local_time);
    if (transfer_order.s_dst == mesh->current_id) { 
        storeState(get_lamport_time(), mesh->current_balance, transfer_order.s_amount);
        printf("IN1: %d\n", get_lamport_time());
        transferIn(mesh, transfer_order.s_src, transfer_order.s_amount, inc_lamport_time());
        printf("IN2: %d\n", get_lamport_time());

        Message ack_msg = createMessage(MESSAGE_MAGIC, mesh->current_balance, mesh->current_id, mesh->parent_id, ACK);
        ack_msg.s_header.s_local_time = inc_lamport_time();
        send(mesh, mesh->parent_id, &ack_msg);
    } else if (transfer_order.s_src == mesh->current_id) {
        printf("OUT1: %d\n", get_lamport_time());
        transferOut(mesh, transfer_order.s_dst, transfer_order.s_amount, inc_lamport_time());
        printf("OUT2: %d\n", get_lamport_time());

        received_msg->s_header.s_local_time = inc_lamport_time();
        send(mesh, transfer_order.s_dst, received_msg);
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
