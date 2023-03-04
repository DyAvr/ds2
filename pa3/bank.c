#include "bank.h"

Bank *bank;

void initBank(int processes_count, char * argv[]){
    bank = (Bank*)malloc(sizeof(Bank));
    bank->b_history = (BalanceHistory*)malloc(sizeof(BalanceHistory));

    for (int i = 0; i < processes_count; i++){
        bank->balances[i] = (balance_t)atoi(argv[i+3]);
    }
}

void initHistory(Mesh *mesh){
    bank->b_history->s_history[0] = createBalanceState(mesh->current_balance, 0, 0);
    bank->b_history->s_history_len = 1;
    bank->b_history->s_id = mesh->current_id;
    bank->last_b_state_time = 1;
}

BalanceState createBalanceState(balance_t balance, timestamp_t time, balance_t balance_pending_in){
    BalanceState state;
    state.s_balance = balance;
    state.s_time = time;
    state.s_balance_pending_in = balance_pending_in;
    return state;
}

void storeState(timestamp_t time, balance_t balance, balance_t balance_pending_in){
    for (int i = bank->last_b_state_time; i < time; i++){
        BalanceState state = createBalanceState(bank->b_history->s_history[i-1].s_balance, i, 0);
        bank->b_history->s_history[i] = state;
    }
    BalanceState new_state = createBalanceState(balance, time, balance_pending_in);
    bank->b_history->s_history[time] = new_state;
    bank->b_history->s_history_len = time + 1;
    bank->last_b_state_time = time + 1;
}

void sendHistory(Mesh *mesh){
    Message msg = createMessage(MESSAGE_MAGIC, mesh->current_balance, mesh->current_id, mesh->parent_id, BALANCE_HISTORY);
    msg.s_header.s_payload_len = sizeof(local_id) + sizeof(uint8_t) + sizeof(BalanceState) * bank->b_history->s_history_len;
    
    memcpy(msg.s_payload, &bank->b_history->s_id, sizeof(local_id));
    memcpy(msg.s_payload + sizeof(local_id), &bank->b_history->s_history_len, sizeof(uint8_t));
    memcpy(msg.s_payload + sizeof(local_id) + sizeof(uint8_t), bank->b_history->s_history, sizeof(BalanceState) * bank->b_history->s_history_len);

    send(mesh, mesh->parent_id, &msg);
}

void showHistory(Mesh *mesh){
    Message msg;
    AllHistory allHistory;
    for (int i = 1; i <= mesh->processes_count; i++){
        inc_lamport_time();
        waitAnyMessage(&msg, mesh, i);
        if (msg.s_header.s_type == BALANCE_HISTORY){
            set_lamport_time(msg.s_header.s_local_time);
            memcpy(allHistory.s_history + allHistory.s_history_len, msg.s_payload, sizeof(BalanceHistory));
            allHistory.s_history_len++;
        }
    }
    print_history(&allHistory);
}

void transferIn(Mesh* mesh, local_id from, balance_t ammount, timestamp_t time){
    mesh->current_balance += ammount;
    storeState(time, mesh->current_balance, 0);
    logEvent(EVENT_TRANSFER_IN, ammount, from, mesh->current_id);
}

void transferOut(Mesh* mesh, local_id to, balance_t ammount, timestamp_t time){
    mesh->current_balance -= ammount;
    storeState(time, mesh->current_balance, ammount);
    logEvent(EVENT_TRANSFER_OUT, ammount, mesh->current_id, to);
}
