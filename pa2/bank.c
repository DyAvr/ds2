#include "bank.h"

Bank *bank;

void initBank(balance_t* balances, Mesh *mesh){
    bank->balances = balances;
    initHistory(mesh);
    bank->last_b_state_time = 1;
}

void initHistory(Mesh *mesh){
    bank->history = (AllHistory*)malloc(sizeof(AllHistory*));
}