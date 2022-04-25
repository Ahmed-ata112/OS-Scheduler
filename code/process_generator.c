#include "headers.h"

void clearResources(int);
int process_msg_queue;
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
    int sch_pid = fork();
    if(sch_pid == 0){
        execl("./scheduler.out","./scheduler.out",NULL);
    }

    int key_id = ftok("keyfile", 65);
     process_msg_queue = msgget(key_id, 0666 | IPC_CREAT);
    struct chosen_algorithm coming;
    coming.algo = 2; //RR
    coming.arg = 3; //q
    coming.mtype = ALGO_TYPE;
    msgsnd(process_msg_queue, &coming, sizeof(coming) - sizeof(coming.mtype),
           !IPC_NOWAIT);

    // I will send some Process to simulate this
    struct process_struct pp;
   // for (int i = 0; i < 3; ++i) {
       for(int i = 5; i >0; i--){
        pp.mtype =PROC_TYPE;
        pp.runtime=4;
        pp.priority=i;
        pp.arrival=0;
        pp.id =i;
        msgsnd(process_msg_queue, &pp, sizeof(pp) - sizeof(pp.mtype),
               !IPC_NOWAIT);
       //sleep(1);
    }
    sleep(10);
    kill(sch_pid,SIGUSR1); //
    sleep(40);
    
    destroyClk(true);
}

void clearResources(int signum) {
    //TODO Clears all resources in case of interruption
    msgctl(process_msg_queue,IPC_RMID,0);
}
