#include "logging.h"

Message createMessage(uint16_t magic, local_id id, MessageType type, timestamp_t time);
MessageHeader createMessageHeader(uint16_t magic, uint16_t len, int16_t type, timestamp_t time);

void handleEvent(EventStatus status, Mesh* mesh);

void sendStartedSignal(Mesh* mesh);
void waitForAllStarted(Mesh* mesh);
void sendDoneSignal(Mesh* mesh);
void waitForAllDone(Mesh* mesh);
