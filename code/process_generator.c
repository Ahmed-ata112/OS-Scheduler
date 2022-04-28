#include "headers.h"
#define FILE_NAME_LENGTH 100

struct Process
{
    long mtype;
    //==============================
    //index ---->    meaning      ||
    //  0   ---->    ID           ||
    //  1   ---->    arrival time ||
    //  2   ---->    runtime      ||
    //  3   ---->    priority     ||
    //============================== 
    int ProcessData[4];
};

struct SchedulerInit
{
    long mtype;
    //=====================================
    //index ---->    meaning             ||
    //  0   ---->    scheduling Algorithm||
    //  1   ---->    quantum if any      ||
    //=====================================
    int Info[2]; 
};

//message queue id
int msgq_id;

void clearResources(int);
int NumOfProcesses (FILE* file , char FileName[]);
void ReadProcessesData (FILE* file, struct Process Processes[],int ProcessesNum);
void Create_Scheduler_Clk(int& sch_pid, int& clk_pid);

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    
    // 1. Read the input files.
    char FileName[FILE_NAME_LENGTH];
    strcpy(FileName,argv[1]);
    FILE* file = fopen(FileName,"r");
    int ProcessesNum = NumOfProcesses(file);
    //make an array of processes and store data of each process in it
    struct Process Processes[ProcessesNum];
    ReadProcessesData(file,Processes,ProcessesNum);
    fclose(file);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int Algorithm = argv[2];
    int quantum = 0;
    if (argc == 4)
    {
        quantum = atoi(argv[3]);
    }

    // 3. Initiate and create the scheduler and clock processes.
    //using forking
    int sch_pid,clk_pid,stat_loc;
    Create_Scheduler_Clk(sch_pid,clk_pid);
    
    // 4. Use this function after creating the clock process to initialize clock
    initClk();

    // To get time use this getClk();
    
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    //create a message queue
    //create the queue id
    key_t qid = ftok(FileName,"q");
    msgq_id = msgget(qid,0666|IPC_CREAT);
    if (msgq_id == -1)
    {
        perror("error has been occured in the message queue\n");
        exit(-1);
    }

    // 6. Send the information to the scheduler at the appropriate time.
    //firtly, initialize the scheduler
    struct SchedularInit Initializer;
    Initializer.Info[0] = Algorithm;
    Initializer.Info[1] = quantum;
    Initializer.mtype = 1;
    //send the initiator struct to the scheduler
    int send_val = msgsnd(msgq_id,&Initializer,sizeof(Initializer.Info),!IPC_NOWAIT);
    if (send_val == -1)
    {
        perror("error has been occured in scheduler initation operation\n");
        exit(-1);
    }
    //secondly, sending processes in the appropiate time
    int ProcessIterator = 0;
    while (getClk()>=Processes->ProcessData[1] && ProcessIterator < ProcessesNum)
    {
        //send the process to the schedular
        send_val = msgsnd(msgq_id,&Processes[ProcessIterator],sizeof(Processes[ProcessIterator].ProcessData),!IPC_NOWAIT);
        if (send_val == -1)
        {
            perror("error has been occured while sending a process number %d to the schedular\n",ProcessIterator);
        }
        ProcessIterator++;
    }   
    sleep(1);
    //send a signal to the schedular indicating that there are no more processes
    kill(sch_pid,SIGUSR1);
    //wait for schedular to finish
    waitpid(sch_pid,&stat_loc,0);
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    msgctl(msgq_id,IPC_RMID,(struct msqid_ds*)0);
    kill(getpgrp(),SIGKILL);
    //is it required to clear clock resources also????
    signal(SIGINT,clearResources);
}

//remember to add the file name in it
int NumOfProcesses (FILE* file,char FileName[])
{
    if (file == NULL)
    {
        perror("the file %s does not exist",FileName);
        exit(-1);
    }
    //number of lines in the file determines number of process
    //note : do not count any line stating with #
    int count = 0;
    char ch;
    for (ch = getc(file); ch!= EOF; ch = getc(file))
    {
        switch (ch)
        {
        case '\n':count++;break;
        default:break;
        }
    }
    //subtract one from the counter because we must skip the first line as it is a comment
    count--;
    return count;
}
void ReadProcessesData(FILE* file, struct Process Processes[],int ProcessesNum)
{
    //note that the pointer of the file points to the end of it
    //because we have count the number of processes so we need to move it again to the begining of the file
    fseek(file,0,SEEK_SET);
    //skip the first line because it is a comment
    char dummy[10];
    for (int i=0;i<4;i++)
    {
        fscanf(file,"%s",dummy);
    }
    for (int i=0;i<ProcessesNum;i++)
    {
        Processes[i].mtype = 1;
        fscanf(file,"%d",&Processes[i].ProcessData[0]);
        fscanf(file,"%d",&Processes[i].ProcessData[1]);
        fscanf(file,"%d",&Processes[i].ProcessData[2]);
        fscanf(file,"%d",&Processes[i].ProcessData[3]);
    }
}
void Create_Scheduler_Clk(int& sch_pid, int& clk_pid)
{
    sch_pid = fork();
    if (sch_pid == -1)
    {
        perror("error has been occured while initiating the scheduler\n");
        exit(-1);
    }
    if (sch_pid == 0)
    {
        execl("./scheduler.out","./scheduler.out",NULL);
    }
    clk_pid = fork();
    if (clk_pid == -1)
    {
        perror("error has been occured while initiating the clock\n");
        exit(-1);
    }
    if (clk_pid == 0)
    {
        execl("./clk.out","./clk.out",NULL);
    }
}
