#include "headers.h"

void clearResources(int);

int main(int argc, char *argv[]) {
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock


    initClk();
    // To get time use this
    int x = getClk();

    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources




    /**
     * @atta
     * TODO: Change THESE
     * @brief just for debugging
     */

    int key_id = ftok("keyfile", 65);
    int process_msg_queue = msgget(key_id, 0666 | IPC_CREAT);
    struct chosen_algorithm coming;
    coming.algo = 1; //RR
    coming.arg = 3;
    coming.mtype = 1;
    msgsnd(process_msg_queue, &coming, sizeof(coming) - sizeof(coming.mtype),
           !IPC_NOWAIT);


    destroyClk(true);
}

void clearResources(int signum) {
    //TODO Clears all resources in case of interruption
}
