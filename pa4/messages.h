#include "logging.h"

Message createMessage(uint16_t magic, balance_t balance, local_id from, local_id to, MessageType type);
MessageHeader createMessageHeader(uint16_t magic, uint16_t len, int16_t type, timestamp_t time);

Message sendStartedSignal(Mesh* mesh);
void waitForAllStarted(Mesh* mesh);
void sendDoneSignal(Mesh* mesh);
void waitAnyMessage(Message *msg, Mesh *mesh, local_id from);
local_id receiveAny(Mesh *mesh, Message *msg); 

void handleCSMessages(Mesh* mesh, Message *msg, local_id from);
void handleDoneMessages(Message *msg, local_id from);
