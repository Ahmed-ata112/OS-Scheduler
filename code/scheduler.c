#include "headers.h"
#include "hashmap.h"
#include "circular_queue.h"
#include "priority_queue.h"

enum state
{
    READY = 1,
    RUNNING = 2,
};
#define pcb_s struct PCB

void RR(int quantum);
void HPF();
struct PCB
{
    int id; // this is the key in the hashmap
    int pid;
    int arrival_time;
    int priority;
    short state;
    int cum_runtime;
    int remaining_time;
    int waiting_time;
};

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

    initClk();
    // process Gen sends a SIGUSR1 to sch to tell than no more processes are coming
    signal(SIGUSR1, set_no_more_processes_coming);
    process_table = hashmap_new(sizeof(pcb_s), 0, 0, 0,
                                process_hash, process_compare, NULL);

    struct chosen_algorithm coming;
    int key_id = ftok("keyfile", 65);
    process_msg_queue = msgget(key_id, 0666 | IPC_CREAT);
    msgrcv(process_msg_queue, &coming, sizeof(coming) - sizeof(coming.mtype), ALGO_TYPE,
           !IPC_NOWAIT);

    printf("\nChosen ALgo is %d\n", coming.algo);

    switch (coming.algo)
    {
    case 1:
        printf("RR\n");

        RR(coming.arg);
        break;
    case 2:
        printf("HPF\n");
        HPF();
        break;
    }

    // upon termination release the clock resources.
    hashmap_free(process_table);
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
     *
     */
    struct c_queue RRqueue;
    circular_init_queue(&RRqueue);
    // if the Queue is empty then check if there is no more processes that will come
    pcb_s *current_pcb;
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
            printf("\nrecv process with id: %d at time %d\n", coming_process.id, getClk());
            // you have that struct Now
            struct PCB pcb;
            pcb.id = coming_process.id;
            pcb.pid = 0;
            pcb.priority = coming_process.priority;
            pcb.arrival_time = coming_process.arrival;
            pcb.remaining_time = coming_process.runtime; // at the beginning

            hashmap_set(process_table, &pcb);              // this copies the content of the struct
            circular_enQueue(&RRqueue, coming_process.id); // add this process to the end of the Queue
            num_messages--;
        }

        // there is a process in the Queue and first time to start
        int previos_clk = getClk();
        int current_clk;

        if (!circular_is_empty(&RRqueue))
        {

            // hashmap_scan(process_table, process_iter, NULL);

            pcb_s get_process = {.id = RRqueue.front->data};
            current_pcb = hashmap_get(process_table, &get_process);

            fflush(0);
            if (current_pcb->pid == 0)
            { // if current process never started before
                int pid = fork();
                if (pid == 0)
                {
                    // child
                    printf("Create process: %d", RRqueue.front->data);
                    execl("./process.out", "./process.out", NULL);
                }
                else
                {
                    // parent take the pid to the hashmap
                    current_pcb->pid = pid; // update Pid of existing process
                    current_pcb->state = RUNNING;
                    struct process_scheduler pp;
                    pp.remaining_time = current_pcb->remaining_time;
                    pp.mtype = TIME_TYPE;
                    int msg_id = msgget(pid, IPC_CREAT | 0666);
                    msgsnd(msg_id, &pp, sizeof(pp.remaining_time), !IPC_NOWAIT);
                }
            }
            // if its time ended or its quantum -> switch (Advance front)
            // otherwise just let it run in peace

            int st;
            int ret = waitpid(-1, &st, WNOHANG);
            // if a process ended normally -- you're sure that the signal came from a dead process -- not stoped or resumed
            bool is_curr_terminated = ret != 0 && WIFEXITED(st);
            if (is_curr_terminated)
            {
                printf("process %d finished at %d", current_pcb->id, getClk());
                circular_deQueue(&RRqueue);
                hashmap_delete(process_table, current_pcb);
            }
            current_clk = getClk();
            // if the current's quantum finished and only one left -> no switch
            // if the current terminated and no other in the Queue -> no switching
            if ((is_curr_terminated && !circular_is_empty(&RRqueue)) ||
                (current_clk - previos_clk >= quantum && !circular_is_only_one_left(&RRqueue)))
            {

                // if terminated -> don't send Stop signal
                if (is_curr_terminated)
                {
                    // TODO add some error printing, bitch
                    kill(current_pcb->pid, SIGSTOP);
                    current_pcb->state = READY;       // back to Ready state
                    circular_advance_queue(&RRqueue); // already advanced if deleted
                }
                current_pcb = hashmap_get(process_table, &(pcb_s){.id = RRqueue.front->data});
                if (current_pcb->pid == 0)
                { // if current process never started before
                    int pid = fork();
                    if (pid == 0)
                    {
                        // child
                        execl("./process.out", "./process.out", NULL);
                    }
                    else
                    {
                        struct process_scheduler pp;
                        pp.remaining_time = current_pcb->remaining_time;
                        pp.mtype = TIME_TYPE;
                        int msg_id = msgget(pid, IPC_CREAT | 0666);
                        msgsnd(msg_id, &pp, sizeof(pp.remaining_time), !IPC_NOWAIT);
                        current_pcb->pid = pid; // update Pid of existing process
                    }
                }
                else
                {
                    kill(current_pcb->pid, SIGCONT);
                }
                current_pcb->state = RUNNING;
                // reset the Quantum counter previos -> start-time of the worker
                previos_clk = getClk();
            }
        }
    }
}
void HPF()
{   
    printf("Entering hpf/n");

    minHeap hpf_queue = init_min_heap();
    pcb_s *current_pcb;

    while (!is_empty(&hpf_queue) || more_processes_coming)
    {
        struct msqid_ds buf;
        int num_messages;

        // First check if any process has come
        msgctl(process_msg_queue, IPC_STAT, &buf);
        num_messages = buf.msg_qnum;
        while (num_messages > 0)
        {
            // while still a process in the queue
            // take it out
            // add it to both the hpf_queue and its PCB to the processTable
            struct process_struct coming_process;
            msgrcv(process_msg_queue, &coming_process, sizeof(coming_process) - sizeof(coming_process.mtype), 0,
                   !IPC_NOWAIT);
            // you have that struct Now
            struct PCB pcb;
            pcb.id = coming_process.id;
            pcb.priority = coming_process.priority;
            pcb.arrival_time = coming_process.arrival;
            pcb.remaining_time = coming_process.runtime; // at the beginning

            hashmap_set(process_table, &pcb);                             // this copies the content of the struct
            push(&hpf_queue, coming_process.priority, coming_process.id); // add this process to the priority queue
            num_messages--;
        }

        int previos_clk; //= getClk();
        int current_clk;
        struct process_scheduler pp;
        if (!is_empty(&hpf_queue))
        {

            current_pcb = hashmap_get(process_table, &(pcb_s){.id = peek(&hpf_queue)->data});

            if (current_pcb->state == READY)
            {
                if (current_pcb->arrival_time <= getClk()) // if it's already the arrival time of the process
                {
                    int pid = fork();
                    if (pid == 0)
                    {
                        // child
                        printf("Create process: %d", peek(&hpf_queue)->data);
                        execl("./process.out", "./process.out", NULL);
                    }
                    else
                    {
                        // parent take the pid to the hashmap
                        current_pcb->pid = pid; // update Pid of existing process
                        current_pcb->state = RUNNING;
                        previos_clk = getClk();

                        pp.remaining_time = current_pcb->remaining_time;
                        pp.mtype = TIME_TYPE;
                        int msg_id = msgget(pid, IPC_CREAT | 0666);
                        msgsnd(msg_id, &pp, sizeof(pp.remaining_time), !IPC_NOWAIT);
                    }
                }
            }

            else
            {

                current_clk = getClk();

                current_pcb->remaining_time = current_pcb->cum_runtime - (current_clk - previos_clk); // update remaining time of process
                //Sends remaining time to process
                pp.remaining_time = current_pcb->remaining_time;
                pp.mtype = TIME_TYPE;
                int msg_id = msgget(current_pcb->pid, IPC_CREAT | 0666);
                msgsnd(msg_id, &pp, sizeof(pp.remaining_time), !IPC_NOWAIT);

                if (current_pcb->remaining_time <= 0)
                // the process has finished, so we have to wait for it to terminate
                {
                    int st;
                    int ret = waitpid(-1, &st, WNOHANG);
                    // if a process ended normally -- you're sure that the signal came from a dead process -- not stoped or resumed
                    bool is_curr_terminated = (ret != 0) && WIFEXITED(st);
                    if (is_curr_terminated)
                    {
                        printf("process %d finished at %d", current_pcb->id, getClk());
                        pop(&hpf_queue);
                        hashmap_delete(process_table, current_pcb);
                    }
                }
            }
        }
    }
}
