#include "headers.h"
#define FILE_NAME_LENGTH 100

// message queue id
int msgq_id;

void clearResources(int);
int NumOfProcesses(FILE *file, char FileName[]);
void ReadProcessesData(FILE *file, struct process_struct Processes[], int ProcessesNum);
void Create_Scheduler_Clk(int *sch_pid, int *clk_pid);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    // 1. Read the input files.
    char FileName[FILE_NAME_LENGTH] = "processes.txt";
    // strcpy(FileName, argv[1]);
    FILE *file = fopen(FileName, "r");
    int ProcessesNum = NumOfProcesses(file, FileName);
    // make an array of processes and store data of each process in it
    struct process_struct Processes[ProcessesNum];
    ReadProcessesData(file, Processes, ProcessesNum);
    fclose(file);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    struct chosen_algorithm Initializer;
    Initializer.algo = atoi(argv[1]);
    Initializer.arg = 6;
    if (argc == 1) //RR
    {
        Initializer.arg = atoi(argv[3]);
    }
    Initializer.mtype = 2;

    // 3. Initiate and create the scheduler and clock processes.
    // using forking
    int sch_pid, clk_pid, stat_loc;
    Create_Scheduler_Clk(&sch_pid, &clk_pid);

    // 4. Use this function after creating the clock process to initialize clock
    initClk();

    // To get time use this getClk();

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.

<<<<<<< HEAD
    /**
     * @atta
     * TODO: Change THESE
     * @note just for debugging
     */


    int key_id = ftok("keyfile", 65);
    process_msg_queue = msgget(key_id, 0666 | IPC_CREAT);
    struct chosen_algorithm coming;
    coming.algo = 3; // SRTN
    //coming.arg = 1;  // q
    coming.mtype = ALGO_TYPE;
    msgsnd(process_msg_queue, &coming, sizeof(coming) - sizeof(coming.mtype),
           !IPC_NOWAIT);

    // I will send some Process to simulate this
    struct process_struct pp;
    for (int i = 0; i < 5; ++i) {
        pp.mtype = PROC_TYPE;
        pp.runtime = i+1;
        pp.priority = i;
        pp.arrival = getClk();
        pp.id = i;
        msgsnd(process_msg_queue, &pp, sizeof(pp) - sizeof(pp.mtype),
               !IPC_NOWAIT);
=======
    // create a message queue
    // create the queue id
    key_t qid = ftok("keyfile", 'q');
    msgq_id = msgget(qid, 0666 | IPC_CREAT);

    if (msgq_id == -1)
    {
        perror("error has been occured in the message queue\n");
        exit(-1);
    }


    // 6. Send the information to the scheduler at the appropriate time.
    // send the initiator struct to the scheduler
    int send_val = msgsnd(msgq_id, &Initializer, sizeof(Initializer) - sizeof(Initializer.mtype), !IPC_NOWAIT);
    if (send_val == -1)
    {
        perror("error has been occured in scheduler initation operation\n");
        exit(-1);
    }
    // secondly, sending processes in the appropiate time
    int ProcessIterator = 0;
    while (ProcessIterator < ProcessesNum)
    {
        if (getClk() >= Processes[ProcessIterator].arrival)
        {
            // send the process to the schedular
            send_val = msgsnd(msgq_id, &Processes[ProcessIterator], sizeof(Processes[ProcessIterator]) - sizeof(Processes[ProcessIterator].mtype), !IPC_NOWAIT);
            if (send_val == -1)
            {
                perror("error has been occured while sending to the schedular\n");
            }
            ProcessIterator++;
        }
>>>>>>> main
    }


    // send a signal to the schedular indicating that there are no more processes
    kill(sch_pid, SIGUSR1);
    // wait for schedular to finish
    waitpid(sch_pid, &stat_loc, 0);

    // 7. Clear clock resources

    destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
    kill(getpgrp(), SIGKILL);
    // is it required to clear clock resources also????
    signal(SIGINT, clearResources);
}

// remember to add the file name in it
int NumOfProcesses(FILE *file, char FileName[])
{
    if (file == NULL)
    {
        perror("the file does not exist");
        exit(-1);
    }
    // number of lines in the file determines number of process
    // note : do not count any line stating with #
    int count = 0;
    char ch;
    for (ch = getc(file); ch != EOF; ch = getc(file))
    {
        switch (ch)
        {
        case '\n':
            count++;
            break;
        default:
            break;
        }
    }
    // subtract one from the counter because we must skip the first line as it is a comment
    count--;
    return count;
}
void ReadProcessesData(FILE *file, struct process_struct Processes[], int ProcessesNum)
{
    // note that the pointer of the file points to the end of it
    // because we have count the number of processes so we need to move it again to the begining of the file
    fseek(file, 0, SEEK_SET);
    // skip the first line because it is a comment
    char dummy[10];
    for (int i = 0; i < 4; i++)
    {
        fscanf(file, "%s", dummy);
    }
    for (int i = 0; i < ProcessesNum; i++)
    {
        Processes[i].mtype = 1;
        fscanf(file, "%d", &Processes[i].id);
        fscanf(file, "%d", &Processes[i].arrival);
        fscanf(file, "%d", &Processes[i].runtime);
        fscanf(file, "%d", &Processes[i].priority);
    }
}
void Create_Scheduler_Clk(int *sch_pid, int *clk_pid)
{
    *sch_pid = fork();
    if (*sch_pid == -1)
    {
        perror("error has been occured while initiating the scheduler\n");
        exit(-1);
    }
    if (*sch_pid == 0)
    {
        execl("./scheduler.out", "./scheduler.out", NULL);
    }
    *clk_pid = fork();
    if (*clk_pid == -1)
    {
        perror("error has been occured while initiating the clock\n");
        exit(-1);
    }
    if (*clk_pid == 0)
    {
        execl("./clk.out", "./clk.out", NULL);
    }
}
