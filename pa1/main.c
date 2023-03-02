#include "communication.h"

int main(int argc, char *argv[]) {
    int proc_qnt = 0;
    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        proc_qnt = atoi(argv[2]);
    } 
    else {
        if (argc > 1) {
            return 1;
        }
    }
    initialize(proc_qnt);
    return 0;
}
