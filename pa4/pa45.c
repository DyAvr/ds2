#include "communication.h"

int main(int argc, char * argv[])
{
    int p_count;
    int with_cs = 0;
    
    if (strcmp(argv[1], "-p") == 0) {
        p_count = (int)atoi(argv[2]);
        if (argc == 4){
            with_cs = 1;
        }
    } else {
        with_cs = 1;
        p_count = (int)atoi(argv[3]);
    }
    
    initialize(p_count, with_cs);
    
    return 0;
}
