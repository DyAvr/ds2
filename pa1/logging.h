#include "utils.h"
#include "pa1.h"
#include "common.h"

static const char * const log_pipe_open_fmt =
    "Process %1d, Pipe: (from: %5d, to: %5d), Descriptors: (r: %5d, w: %5d)\n";

static const char * const log_pipe_closed_fmt =
    "CLOSED: Process %1d, Descriptors: (r: %5d, w: %5d)\n";

typedef enum {
    EVENT_STARTED,
    EVENT_RECEIVED_ALL_STARTED,
    EVENT_DONE,
    EVENT_RECEIVED_ALL_DONE
} EventStatus;

void initLogger();
void closeLogger();

char* logEvent(local_id id, EventStatus status);
void logPipe(local_id id, int p1, int p2, int ds_read, int ds_write);
