#include "communication.h"

Mesh *mesh;

void initialize(int processes_count, balance_t** balances){
    initLogger();
    initMesh(processes_count);
    initBank(balances, processes_count);
    createMeshProcesses();
}

void initMesh(int processes_count){
    mesh = (Mesh*)malloc(sizeof(Mesh));
    if (mesh == NULL){
        perror("Failed to allocate memory");
        exit(1);
    }
    mesh->processes_count = processes_count;
    mesh->current_id = getpid();
    mesh->parent_id = getpid();
    createMeshPipes();
}

void createMeshProcesses(){
    pid_t pid;
    for (int i = 0; i < mesh->processes_count; i++) {
        mesh->current_balance = *(bank->balances)[i];
        pid = fork();
        if (pid == -1) {
            printf("Cannot create fork process");
            exit(1);
        } else if (pid == 0) {
            mesh->current_id = i;
            mesh->current = pid;
            initHistory(mesh);
            exit(0);
        } else {
            for (int j = 0; j <= mesh->processes_count; j++){
                if (j!= (i+1)){
                    close(mesh->pipes[i+1][j]->fdWrite);
                }
            }
        }
    }

    closeUnusedPipes();

    if (mesh->parent == mesh->current){
        startParent();
    } else {
        startChild();
    }

    closeLineCommunication();
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
    //start
    sendStartedSignal(mesh);
    waitForAllStarted(mesh);
    //work
    workChild();
    //ending
    sendDoneSignal(mesh);
    waitForAllDone(mesh);
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
    showHistory(mesh);
}

void workChild(){
    int exit_flag = 1;

    while (exit_flag) {
        int status;
        Message received_msg;

        while ((status = receive_any(mesh, &received_msg)) == 1);

        if (status == 0) {
            switch (received_msg.s_header.s_type) {
                case STOP:
                    storeState(received_msg.s_header.s_local_time, mesh->current_balance);
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

    if (transfer_order.s_dst == mesh->current_id) {
        timestamp_t time = get_physical_time();
        transferIn(mesh, transfer_order.s_src, transfer_order.s_amount, time);

        Message ack_msg = createMessage(MESSAGE_MAGIC, mesh->current_balance, mesh->current_id, mesh->parent_id, ACK);
        ack_msg.s_header.s_local_time = time;
        send(mesh, mesh->parent_id, &ack_msg);

    } else if (transfer_order.s_src == mesh->current_id) {
        timestamp_t time = get_physical_time();
        transferOut(mesh, transfer_order.s_dst, transfer_order.s_amount, time);

        received_msg->s_header.s_local_time = time;
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
