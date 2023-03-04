#include "logging.h"

Message createMessage(uint16_t magic, balance_t balance, local_id from, local_id to, MessageType type);
MessageHeader createMessageHeader(uint16_t magic, uint16_t len, int16_t type);

TransferOrder createTransferOrder(local_id from, local_id to, balance_t balance);

Message sendStartedSignal(Mesh* mesh);
void waitForAllStarted(Mesh* mesh);
void sendDoneSignal(Mesh* mesh);
void waitForAllDone(Mesh* mesh);
void waitAnyMessage(Message *msg, Mesh *mesh, local_id from);
void sendStopSignal(Mesh* mesh);
