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

void logEvent(EventStatus status, local_id from, local_id to, timestamp_t time){
    char buf[255];

    switch(status) {
        case EVENT_STARTED:
            sprintf(buf, log_started_fmt, time, from, getpid(), getppid(), 0);
            break;
        case EVENT_DONE:
            sprintf(buf, log_done_fmt, time, from, 0);
            break;
        case EVENT_RECEIVED_ALL_STARTED:
            sprintf(buf, log_received_all_started_fmt, time, from);
            break;
        case EVENT_RECEIVED_ALL_DONE:
            sprintf(buf, log_received_all_done_fmt, time, from);
            break;
        case EVENT_TRANSFER_IN:
            sprintf(buf, log_transfer_in_fmt, time, to, 0, from);
            break;
        case EVENT_TRANSFER_OUT:
            sprintf(buf, log_transfer_out_fmt, time, from, 0, to);
            break;
        case EVENT_LOOP_OPERATION:
            sprintf(buf, log_loop_operation_fmt, from, to, time);
            break;
    }

    printf("%s", buf);
    write(logger.fd_events_log, buf, strlen(buf));
}

void logPipe(PipeStatus status, int p1, int p2, int ds_read, int ds_write){
    char buf[100];

    switch(status) {
        case PIPE_STARTED:
            sprintf(buf, log_pipe_open_fmt, p1, p2, ds_read, ds_write);
            break;
        case PIPE_CLOSED:
            sprintf(buf, log_pipe_closed_fmt, p1, p2, ds_read, ds_write);
            break;
    }

    //printf("%s", buf);
    write(logger.fd_pipes_log, buf, strlen(buf));
}
