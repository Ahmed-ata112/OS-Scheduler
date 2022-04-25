#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    initClk();
    struct process_scheduler pp;
    int msg_id = msgget(getpid(), IPC_CREAT | 0666);
    msgrcv(msg_id, &pp, sizeof(pp.remaining_time), TIME_TYPE, !IPC_NOWAIT);
    ;
    remainingtime = pp.remaining_time;
    printf("\nin Process.c remain_time = %d\n", remainingtime);
    int curr_clk = getClk();
    while (remainingtime > 0)
    {
        if (curr_clk != getClk())
        {
            remainingtime--;
            curr_clk = getClk();
        }
    }

    destroyClk(false);
    return 0;
}
