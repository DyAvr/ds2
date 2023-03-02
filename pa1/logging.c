#include <stdlib.h>
#include <fcntl.h>

#include "logging.h"

Logger logger;

void initLogger(){
    logger.fd_events_log = open(events_log, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (logger.fd_events_log == -1) {
        perror("Error opening events log file");
        exit(1);
    }
    logger.fd_pipes_log = open(pipes_log, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (logger.fd_pipes_log == -1) {
        perror("Error opening pipes log file");
        exit(1);
    }
}

void closeLogger(){
    if (close(logger.fd_events_log) != 0) {
        perror("Error closing events log file");
    }
    if (close(logger.fd_pipes_log) != 0) {
        perror("Error closing pipes log file");
    }
}

char* logEvent(local_id id, EventStatus status){
    char buf[255];

    //printf("%1d: %u\n", id, status);

    switch(status) {
        case EVENT_STARTED:
            sprintf(buf, log_started_fmt, id, getpid(), getppid());
            break;
        case EVENT_RECEIVED_ALL_STARTED:
            sprintf(buf, log_received_all_started_fmt, id);
            break;
        case EVENT_DONE:
            sprintf(buf, log_done_fmt, id);
            break;
        case EVENT_RECEIVED_ALL_DONE:
            sprintf(buf, log_received_all_done_fmt, id);
            break;
    }

    printf("%s", buf);
    write(logger.fd_events_log, buf, strlen(buf));

    char *result = (char*)malloc(strlen(buf)+1);
    strcpy(result, buf);
    return result;
}

void logPipe(local_id id, int p1, int p2, int ds_read, int ds_write){
    char buf[100];

    if (p1 == 0 && p2 == 0){
        sprintf(buf, log_pipe_closed_fmt, id, ds_read, ds_write);
    } else {
        sprintf(buf, log_pipe_open_fmt, id, p1, p2, ds_read, ds_write);
    }

    write(logger.fd_pipes_log, buf, strlen(buf));
}
