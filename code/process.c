#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    initClk();
    init_remain_time();

    struct process_scheduler pp;
    //int msg_id = msgget(getpid(), IPC_CREAT | 0666);
    //msgrcv(msg_id, &pp, sizeof(pp.remaining_time), TIME_TYPE, !IPC_NOWAIT);
    //remainingtime = pp.remaining_time;
    printf("\nin Process.c remain_time = %d\n", remainingtime);
    int prev_clk = getClk();
    while (remainingtime > 0)
    {
        int curr_clk = getClk();
        if (prev_clk != curr_clk)
        {
            // printf("\n%d\n", remainingtime);
            remainingtime--;
            printf("\n%d\n", remainingtime);
            prev_clk = curr_clk;
        }
    }

    destroyClk(false);
    return 0;
}
