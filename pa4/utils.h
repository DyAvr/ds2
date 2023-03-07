#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

#include "time.h"

#define MAX_PROCESSES_COUNT 10

typedef struct{
    int fdRead;
    int fdWrite;
} Pipe;

typedef struct{
    int processes_count;
    Pipe *pipes[MAX_PROCESSES_COUNT+1][MAX_PROCESSES_COUNT+1];
    local_id current_id;
    local_id parent_id;
    pid_t current;
    pid_t parent;
} Mesh;

typedef struct{
    int fd_events_log;
    int fd_pipes_log;
} Logger;

typedef struct{
    local_id l_id;
    timestamp_t l_time;
} Request;

typedef struct{
    int length;
    Request requests[MAX_PROCESSES_COUNT+1];
    int replies[MAX_PROCESSES_COUNT+1]; 
    int released[MAX_PROCESSES_COUNT+1];
} Queue;
