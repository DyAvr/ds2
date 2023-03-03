#include "messages.h"

extern Bank *bank;

void initBank(balance_t **balances, int processes_count);
void initHistory(Mesh *mesh);

void storeState(timestamp_t time, balance_t balance);
void sendHistory(Mesh *mesh);
void showHistory(Mesh *mesh);

BalanceState createBalanceState(balance_t balance, timestamp_t time, balance_t balance_pending_in);

void transferIn(Mesh* mesh, local_id from, balance_t ammount, timestamp_t time);
void transferOut(Mesh* mesh, local_id to, balance_t ammount, timestamp_t time);
