#include "headers.h"

/* Modify this file as needed*/
int remainingtime;


int main(int agrc, char *argv[]) {
    initClk();

    int msg_id = msgget(getpid(), IPC_CREAT | 0666);
    //remainingtime = ??;
    printf("in Process.c\n");
    //
    while (remainingtime > 0) {
        // remainingtime = ??;
        //check clk
        // remainingtime--
    }

    destroyClk(false);
    return 0;
}
