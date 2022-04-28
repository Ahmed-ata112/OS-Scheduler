#include "headers.h"
#include "hashmap.h"
#include "circular_queue.h"
#include "priority_queue.h"
#include "priority_queue.c"

enum state
{
    READY = 1,
    RUNNING = 2,
};
#define pcb_s struct PCB

void RR(int quantum);
void SRTN();

//@Ahmed-H300
// change it to typedef instead of struct
typedef struct PCB
{
    int id; // this is the key in the hashmap
    int pid;
    int arrival_time;
    int priority;
    short state;
    int cum_runtime;
    int burst_time;
    int remaining_time;
    int waiting_time;
} PCB;

// 3 functions related to the hashmap
int process_compare(const void *a, const void *b, void *udata)
{
    const struct PCB *process_a = a;
    const struct PCB *process_b = b;
    return (process_a->id - process_b->id);
}

bool process_iter(const void *item, void *udata)
{
    const struct PCB *process = item;
    printf("process: (id=%d) (arrivalTime=%d) (runTime=%d) (priority=%d) (pid=%d) (state=%d) (remainingTime=%d)\n",
           process->pid, process->arrival_time, process->cum_runtime, process->priority, process->pid, process->state,
           process->remaining_time);
    return true;
}

uint64_t process_hash(const void *item, uint64_t seed0, uint64_t seed1)
{
    const struct PCB *process = item;
    return hashmap_sip(&process->id, sizeof(process->id), seed0, seed1);
}

// TODO init this in the main of the scheduler
struct hashmap *process_table;
int process_msg_queue;

// when process generator tells us that there is no more to come
// set this to false

bool more_processes_coming = true;

void set_no_more_processes_coming(int signum)
{
    more_processes_coming = false;
}

int main(int argc, char *argv[])
{
    // process Gen sends a SIGUSR1 to sch to tell than no more processes are coming
    signal(SIGUSR1, set_no_more_processes_coming);

    initClk();

    int remain_time_shmid = shmget(REMAIN_TIME_SHMKEY, 4, IPC_CREAT | 0644);
    if (remain_time_shmid == -1)
        perror("cant init remaining time shm: \n");
    shm_remain_time = (int *)shmat(remain_time_shmid, NULL, 0);
    *shm_remain_time = -1;

    process_table = hashmap_new(sizeof(pcb_s), 0, 0, 0,
                                process_hash, process_compare, NULL);

    struct chosen_algorithm coming;
    int key_id = ftok("keyfile", 65);
    process_msg_queue = msgget(key_id, 0666 | IPC_CREAT);
    msgrcv(process_msg_queue, &coming, sizeof(coming) - sizeof(coming.mtype), ALGO_TYPE,
           !IPC_NOWAIT);

    printf("\nChosen Algo is %d\n", coming.algo);

    switch (coming.algo)
    {
    case 1:
        printf("RR\n");
        RR(coming.arg);
        break;

    case 3:
        printf("SRTN\n");
        SRTN();
        break;
    }

    // upon termination release the clock resources.
    hashmap_free(process_table);
    shmctl(remain_time_shmid, IPC_RMID, NULL);
    destroyClk(true);
}

void RR(int quantum)
{
    /**
     * i loop all the time
     * till a variable tells me that there is no more proccesses comming
     * this is when i quit
     * All the processes that in the circular queue are in the process_table
     * when finished -> u delete from both
     * @bug: if the process gen sends a SIGUSR1 immediately after sending Processes -> it finishes too
     *       @solution -> make Process gen sleep for a 1 sec or st after sending all
     */

    struct c_queue RRqueue;
    circular_init_queue(&RRqueue);
    // if the Queue is empty then check if there is no more processes that will come
    pcb_s *current_pcb;
    int curr_q_start;

    while (!circular_is_empty(&RRqueue) || more_processes_coming)
    {

        // First check if any process has come
        struct msqid_ds buf;
        int num_messages;
        msgctl(process_msg_queue, IPC_STAT, &buf);
        num_messages = buf.msg_qnum;
        while (num_messages > 0)
        {
            // while still a process in the queue
            // take it out
            // add it to both the RRqueue and its PCB to the processTable
            struct process_struct coming_process;
            msgrcv(process_msg_queue, &coming_process, sizeof(coming_process) - sizeof(coming_process.mtype), 1,
                   !IPC_NOWAIT);
            // printf("\nrecv process with id: %d at time %d\n", coming_process.id, getClk());
            //  you have that struct Now
            struct PCB pcb;
            pcb.id = coming_process.id;
            pcb.pid = 0;
            pcb.priority = coming_process.priority;
            pcb.arrival_time = coming_process.arrival;
            pcb.cum_runtime = 0;
            pcb.remaining_time = coming_process.runtime;   // at the beginning
            pcb.burst_time = coming_process.runtime;       // at the beginning
            hashmap_set(process_table, &pcb);              // this copies the content of the struct
            circular_enQueue(&RRqueue, coming_process.id); // add this process to the end of the Queue
            num_messages--;
        }

        // there is a process in the Queue and first time to start
        if (!circular_is_empty(&RRqueue))
        {
            // hashmap_scan(process_table, process_iter, NULL);
            pcb_s get_process = {.id = RRqueue.front->data};
            current_pcb = hashmap_get(process_table, &get_process);
            if (current_pcb->pid == 0)
            { // if current process never started before
                *shm_remain_time = current_pcb->remaining_time;
                int pid = fork();
                if (pid == 0)
                {
                    // child
                    printf("Create process: %d", RRqueue.front->data);
                    execl("./process.out", "./process.out", NULL);
                }
                // parent take the pid to the hashmap
                current_pcb->pid = pid; // update Pid of existing process
                current_pcb->state = RUNNING;
                printf("At time %d process %d started arr %d total %d remain %d wait 0\n", getClk(), current_pcb->id,
                       current_pcb->arrival_time, current_pcb->burst_time, *shm_remain_time);
                curr_q_start = getClk(); // started a quantum
            }
            // if its time ended or its quantum -> switch (Advance front)
            // otherwise just let it run in peace
            int curr = getClk();
            bool try_to_switch_if_terminated = false;
            bool try_to_switch_if_q = false;
            if (curr - curr_q_start >= current_pcb->remaining_time)
            { // that process will be finished
                int st;
                *shm_remain_time = 0;
                current_pcb->cum_runtime = current_pcb->burst_time;
                int ret = wait(&st);
                // if a process ended normally -- you're sure that the signal came from a dead process -- not stoped or resumed
                int TA = curr - current_pcb->arrival_time;
                current_pcb->waiting_time = TA - current_pcb->burst_time;
                float WTA = (float)TA / current_pcb->burst_time;
                printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(),
                       current_pcb->id,
                       current_pcb->arrival_time, current_pcb->burst_time, *shm_remain_time, current_pcb->waiting_time,
                       TA, WTA);
                circular_deQueue(&RRqueue); // auto advance the queue
                hashmap_delete(process_table, current_pcb);
                // if the terminated is the last one so no switching
                try_to_switch_if_terminated = !circular_is_empty(&RRqueue);

                // if its q finished and there are some other in the Q waiting
            }
            else if (curr - curr_q_start >= quantum && !circular_is_empty_or_one_left(&RRqueue))
            {
                // its quantum finished
                //  TODO add some error printing, bitch
                *shm_remain_time -= quantum;

                kill(current_pcb->pid, SIGSTOP);
                current_pcb->cum_runtime += quantum;
                current_pcb->waiting_time = curr - current_pcb->arrival_time - current_pcb->cum_runtime;

                printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), current_pcb->id,
                       current_pcb->arrival_time, current_pcb->burst_time, *shm_remain_time, current_pcb->waiting_time);
                current_pcb->remaining_time = *shm_remain_time;
                current_pcb->state = READY;       // back to Ready state
                circular_advance_queue(&RRqueue); // already advanced if deleted
                try_to_switch_if_q = true;
            }

            if (try_to_switch_if_terminated || try_to_switch_if_q)
            {
                // printf("switch\n");

                current_pcb = hashmap_get(process_table, &(pcb_s){.id = RRqueue.front->data});
                if (current_pcb->pid == 0)
                { // if current process never started before
                    *shm_remain_time = current_pcb->remaining_time;
                    int pid = fork();
                    if (pid == 0)
                    {
                        // child
                        execl("./process.out", "./process.out", NULL);
                    }

                    // parent
                    current_pcb->pid = pid; // update Pid of existing process

                    current_pcb->waiting_time = curr - current_pcb->arrival_time;

                    printf("At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(),
                           current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time, *shm_remain_time,
                           current_pcb->waiting_time);
                }
                else
                {

                    kill(current_pcb->pid, SIGCONT);
                    current_pcb->waiting_time = curr - current_pcb->arrival_time - current_pcb->cum_runtime;
                    *shm_remain_time = current_pcb->remaining_time;
                    printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(),
                           current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time, *shm_remain_time,
                           current_pcb->waiting_time);
                }
                current_pcb->state = RUNNING;
                curr_q_start = getClk(); // started a quantum
            }

            // if the current's quantum finished and only one left -> no switch
            // if the current terminated and no other in the Queue -> no switching
        }
    }

    printf("\nOut at time %d\n", getClk());
}

/*----------------------------------------------------------------
Author: Ahmed Hany @Ahmed-H300
function: SRTN
param: void ()
output: void
*/
//----------------------------------------------------------------
void SRTN()
{
    printf("Entering SRTN \n");
    // intialize the priority queue
    minHeap sQueue;
    sQueue = init_min_heap();

    PCB *current_pcb = NULL;
    int curr_q_start;

    // if the Queue is empty then check if there is no more processes that will come
    // the main loop for the scheduler
    while (!is_empty(&sQueue) || more_processes_coming)
    {
        // First check if any process has come
        int current_time = getClk();
        struct msqid_ds buf;
        int num_messages;
        msgctl(process_msg_queue, IPC_STAT, &buf);
        num_messages = buf.msg_qnum;
        while (num_messages > 0)
        {
            // while still a process in the queue
            // take it out
            // add it to both the Prority Queue (sQueue) and its PCB to the processTable
            process_struct coming_process;
            msgrcv(process_msg_queue, &coming_process, sizeof(coming_process) - sizeof(coming_process.mtype), 1,
                   !IPC_NOWAIT);
            printf("\nrecv process with id: %d at time %d with priority %d\n", coming_process.id, current_time, coming_process.priority);
            //  you have that struct Now
            PCB pcb;
            pcb.id = coming_process.id;
            pcb.pid = 0;
            //pcb.arrival_time = coming_process.arrival;
            pcb.arrival_time = current_time;
            pcb.priority = coming_process.priority;
            pcb.state = READY;
            pcb.cum_runtime = 0;
            pcb.burst_time = coming_process.runtime;     // at the beginning
            pcb.remaining_time = coming_process.runtime; // at the beginning
            pcb.waiting_time = 0;                        // at the beginning
            hashmap_set(process_table, &pcb);            // this copies the content of the struct
            push(&sQueue, pcb.remaining_time, pcb.id);         // add this process to the end of the Queue
            heapify(&sQueue, 0);
            num_messages--;
        }
        // test queue
        // for (int i = 0; i < 5; i++)
        // {
        //     printf("id = %d\n", pop(&sQueue)->data);
        // }
        // break;
        ////////////////////////////////
        //  if the first start then put the first on in queue
        // if not check if the current process running is the one on top
        // if yes continue runnig else switch

        // 3 cases
        // first is first start process or there is gap between processes -> Done
        // second update the remaining time
        // third is a new process with shortest time came
        
        if (current_pcb == NULL)
        {
            node* temp = peek(&sQueue);
            PCB get_process = {.id = temp->data};
            current_pcb = hashmap_get(process_table, &get_process);
            *shm_remain_time = current_pcb->remaining_time;
            // first time
            if (current_pcb->pid == 0)
            {
                int pid = fork();
                if (pid == 0)
                {
                    // child
                    execl("./process.out", "./process.out", NULL);
                }

                // continue scheduler
                //int curr = current_time;
                current_pcb->pid = pid; // update Pid of existing process
                current_pcb->state = RUNNING;
                current_pcb->waiting_time = current_time - current_pcb->arrival_time;
                printf("At time %d process %d started arr %d total %d remain %d wait %d\n",
                       current_time, current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time,
                        *shm_remain_time, current_pcb->waiting_time);
            }
            // resumed after stopped
            else
            {
                int pid = fork();
                if (pid == 0)
                {
                    // child
                    execl("./process.out", "./process.out", NULL);
                }

                // continue scheduler
                //int curr = current_time;
                current_pcb->pid = pid; // update Pid of existing process
                current_pcb->state = RUNNING;
                current_pcb->waiting_time += current_time - (current_pcb->arrival_time + current_pcb->cum_runtime);
                printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n",
                        current_time,current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time,
                         *shm_remain_time,current_pcb->waiting_time);

                
            }
            
        }
        else
        {
            //int curr = current_time;
            *shm_remain_time -= (current_time - (current_pcb->arrival_time + current_pcb->cum_runtime + current_pcb->waiting_time));
            current_pcb->remaining_time = *shm_remain_time;
            if (*shm_remain_time <= 0)
            {
                *shm_remain_time = 0;
                current_pcb->remaining_time = 0;
                current_pcb->cum_runtime = current_pcb->burst_time;
                // kill and out new data
                int dum;
                int ret = wait(&dum);
                int TA = current_time - current_pcb->arrival_time;
                float WTA = (float)TA / current_pcb->burst_time;
                printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
                        current_time,current_pcb->id,current_pcb->arrival_time,
                        current_pcb->burst_time, *shm_remain_time,
                        current_pcb->waiting_time,
                        TA, WTA);
                hashmap_delete(process_table, current_pcb);
                pop(&sQueue);
                heapify(&sQueue, 0);
                current_pcb = NULL;
                continue;
                
            }
            current_pcb->cum_runtime = current_pcb->burst_time - *shm_remain_time;
            node* temp = peek(&sQueue);
            if (temp->priority < current_pcb->remaining_time)
            {
                // swap and stop current process
                *shm_remain_time = 0;
                int dum;
                int ret = wait(&dum);

                printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n",
                    current_time, current_pcb->id,current_pcb->arrival_time,
                    current_pcb->burst_time, *shm_remain_time, current_pcb->waiting_time);

                current_pcb->state = READY;       // back to Ready state
                //push(&sQueue, current_pcb->remaining_time, current_pcb->id);
            }
        }
        

    }

    printf("\nOut at time %d\n", getClk());
}
