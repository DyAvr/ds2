#include "utils.h"
#include "pa2345.h"
#include "common.h"

static const char * const log_pipe_open_fmt =
    "OPEN: Pipe: (from: %5d, to: %5d), Descriptors: (r: %5d, w: %5d)\n";

static const char * const log_pipe_closed_fmt =
    "CLOSED: Pipe: (from: %5d, to: %5d), Descriptors: (r: %5d, w: %5d)\n";

typedef enum {
    EVENT_STARTED,
    EVENT_RECEIVED_ALL_STARTED,
    EVENT_DONE,
    EVENT_RECEIVED_ALL_DONE,
    EVENT_TRANSFER_OUT,
    EVENT_TRANSFER_IN,
    EVENT_LOOP_OPERATION
} EventStatus;

typedef enum {
    PIPE_STARTED,
    PIPE_CLOSED
} PipeStatus;

void initLogger();
void closeLogger();

void logEvent(EventStatus status, balance_t balance, local_id from, local_id to);
void logPipe(PipeStatus status, int p1, int p2, int ds_read, int ds_write);
