#include "communication.h"

void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount)
{
    // student, please implement me
}

int main(int argc, char * argv[])
{
    int p_count;
    
    if (argc > 4 && strcmp(argv[1], "-p") == 0) {
        p_count = atoi(argv[2]);
    } 
    else {
        return 1;
    }

    balance_t* balances = malloc(sizeof(balance_t) * p_count)];
    for (size_t i = 3; i < p_count; i++)
    {
        balances[i-3] = (balance_t)atoi(argv[i]);
    }
    
    initialize(p_count, balances);
    
    return 0;
}
