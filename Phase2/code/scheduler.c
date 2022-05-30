#include "headers.h"
#include "hashmap.h"
#include "circular_queue.h"
#include "priority_queue.h"
#include "math.h"
#include "queue.h"
#include "buddy_core.c"

#define pcb_s struct PCB
typedef struct PCB {
    int id; // this is the key in the hashmap
    int pid;
    int arrival_time;
    int priority;
    short state;
    int cum_runtime;
    int burst_time;
    int remaining_time;
    int waiting_time;
    //...............................P2
    int mem_size;
    int memory_start_ind;
    int memory_end_ind;
} PCB;


FILE *sch_log, *sch_perf, *mem_log;
int TotalWaitingTime = 0;
int TotalWTA = 0;
int TotalRunTime = 0;
int TotalNumberOfProcesses = 0;
int *WeightedTA = NULL;
int WTAIterator = 0;

void OutputFinishedProcesses(int CurrTime, PCB *current_pcb, int TA, float WTA);

void print(int CurrentTime, PCB *current, pair_t *Index, char type);

void scheduler_log();

void memory_log();

float CalcStdWTA(float AvgWTA);

void scheduler_perf(int ProcessCount);

void FinishPrinting();

int RR2(int quantum);

void SRTN();

int HPF();

// 3 functions related to the hashmap
int process_compare(const void *a, const void *b, void *udata) {
    const struct PCB *process_a = a;
    const struct PCB *process_b = b;
    return (process_a->id - process_b->id);
}

bool process_iter(const void *item, void *udata) {
    const struct PCB *process = item;
    printf("process: (id=%d) (arrivalTime=%d) (runTime=%d) (priority=%d) (pid=%d) (state=%d) (remainingTime=%d) (mem_size=%d) (memory_start=%d) (memory_end_ind=%d)\n",
           process->pid, process->arrival_time, process->cum_runtime, process->priority, process->pid, process->state,
           process->remaining_time, process->mem_size, process->memory_start_ind, process->memory_end_ind);
    return true;
}

uint64_t process_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct PCB *process = item;
    return hashmap_sip(&process->id, sizeof(process->id), seed0, seed1);
}

// TODO init this in the main of the scheduler
struct hashmap *process_table;
int process_msg_queue;

// when process generator tells us that there is no more to come
// set this to false

bool more_processes_coming = true;

void set_no_more_processes_coming(int signum) {
    more_processes_coming = false;
}

int main(int argc, char *argv[]) {
    // process Gen sends a SIGUSR1 to sch to tell than no more processes are coming
    signal(SIGUSR1, set_no_more_processes_coming);

    // create and open files
    scheduler_log();
    memory_log();
    initClk();
    buddy_init();

    int remain_time_shmid = shmget(REMAIN_TIME_SHMKEY, 4, IPC_CREAT | 0644);
    if (remain_time_shmid == -1)
        perror("cant init remaining time shm: \n");
    shm_remain_time = (int *) shmat(remain_time_shmid, NULL, 0);
    *shm_remain_time = -1;

    process_table = hashmap_new(sizeof(PCB), 0, 0, 0, process_hash, process_compare, NULL);

    struct chosen_algorithm coming;
    int key_id = ftok("keyfile", 'q');
    process_msg_queue = msgget(key_id, 0666 | IPC_CREAT);
    msgrcv(process_msg_queue, &coming, sizeof(coming) - sizeof(coming.mtype), ALGO_TYPE, !IPC_NOWAIT);

    // Set number of processes to use in statistics (scheduler.perf)
    TotalNumberOfProcesses = coming.NumOfProcesses;
    WeightedTA = (int *) malloc(sizeof(int) * TotalNumberOfProcesses);

    printf("\nchosen Algo is %d\n", coming.algo);

    switch (coming.algo) {
        case 1:
            printf("RR with q=%d at time: %d\n", coming.arg, getClk());
            RR2(coming.arg);
            break;

        case 2:
            printf("HPF\n");
            HPF();
            break;
        case 3:
            printf("SRTN\n");
            SRTN();
            break;
    }
    printf("DONE scheduler\n");
    scheduler_perf(TotalNumberOfProcesses);
    FinishPrinting();
    // upon termination release the clock resources.
    hashmap_free(process_table);
    shmctl(remain_time_shmid, IPC_RMID, NULL);
    destroyClk(true);
}

PCB set_process(process_struct coming_process ){
     struct PCB pcb;
            pcb.id = coming_process.id;
            pcb.pid = 0;
            pcb.priority = coming_process.priority;
            pcb.arrival_time = coming_process.arrival;
            pcb.cum_runtime = 0;
            pcb.remaining_time = coming_process.runtime; // at the beginning
            pcb.burst_time = coming_process.runtime;     // at the beginning
            pcb.mem_size = coming_process.memsize;
            pcb.waiting_time = 0;


            return pcb;

}
int RR2(int quantum)
{
    /**
     * i loop all the time
     * till a variable tells me that there is no more processes coming
     * this is when i quit
     * All the processes that in the circular queue are in the process_table
     * when finished -> u delete from both
     **/
    struct c_queue RRqueue;
    circular_init_queue(&RRqueue);

    queue waiting_queue = initQueue(); // to receive in it

    PCB *current_pcb;
    int curr_q_start;
    int p_count = TotalNumberOfProcesses;
    int need_to_receive = TotalNumberOfProcesses;
    bool process_is_currently_running = false;
    int curr = 0;
    while (!circular_is_empty(&RRqueue) || p_count > 0) {
        printf("\n in loop \n");
        // First check if any process has come
        struct count_msg c = {.count = 0};
        if (more_processes_coming || need_to_receive > 0)
            msgrcv(process_msg_queue, &c, sizeof(int), 10, !IPC_NOWAIT);

        int num_messages = c.count;
        need_to_receive -= c.count;
        curr = getClk();

        while (num_messages > 0) {
            // while still a process in the queue
            // take it out
            // add it to both the RRqueue and its PCB to the processTable
            process_struct coming_process;
            msgrcv(process_msg_queue, &coming_process, sizeof(coming_process) - sizeof(coming_process.mtype), 1,
                   !IPC_NOWAIT);
            printf("\nrecv process with id: %d at time %d\n", coming_process.id, getClk());
            //  you have that struct Now
            PCB pcb= set_process(coming_process);

            pushQueue(&waiting_queue, coming_process.id); // add to the waiting list and will see if you can Run
            hashmap_set(process_table, &pcb); // this copies the content of the struct
            num_messages--;


        }

        //  printf("curr is %d: %d\n", getClk(), more_processes_coming);
        // if its time ended or its quantum -> switch (Advance front)
        // otherwise just let it run in peace
        // if there is a running process -> see if it can be finished or not

//        hashmap_scan(process_table, process_iter, NULL);
        if (!isEmptyQueue(&waiting_queue)) {
            int n_loops = waiting_queue.size;
            while (n_loops--) {
                int id = front(&waiting_queue);
                popQueue(&waiting_queue);
                PCB *_pcb = hashmap_get(process_table, &(PCB) {.id = id});
                pair_t ret;

                if (buddy_allocate(_pcb->mem_size, &ret)) {
                    _pcb->state = READY; // allocated and in ready Queue
                    _pcb->memory_start_ind = ret.start_ind;
                    _pcb->memory_end_ind = ret.end_ind;
                    circular_enQueue(&RRqueue, id);
                    print(curr, _pcb, &ret, 'a');

                } else {
                    pushQueue(&waiting_queue, id);
                }
            }
        }


        if (process_is_currently_running) {
            current_pcb = hashmap_get(process_table, &(PCB) {.id = RRqueue.front->data});

            if (curr - curr_q_start >= current_pcb->remaining_time) { // that process will be finished
                int st;

                current_pcb->remaining_time = 0;
                current_pcb->cum_runtime = current_pcb->burst_time;
                int ret = wait(&st);
                if (ret == -1) {
                    perror("error in waiting the Process:");
                }
                // if a process ended normally -- you're sure that the signal came from a dead process -- not stopped or resumed
                int TA = curr - current_pcb->arrival_time;
                current_pcb->waiting_time = TA - current_pcb->burst_time;
                float WTA = (float) TA / current_pcb->burst_time;

                OutputFinishedProcesses(curr, current_pcb, TA, WTA);
                p_count--;
                circular_deQueue(&RRqueue); // auto advance the queue
                buddy_deallocate(current_pcb->memory_start_ind, current_pcb->memory_end_ind);
                print(curr, current_pcb, NULL, 'd');

                hashmap_delete(process_table, current_pcb);
                process_is_currently_running = false;

                // if its multiple of q finished and there are some other in the Q waiting
            } else if ((curr - curr_q_start) && (curr - curr_q_start) % quantum == 0 &&
                       !circular_is_empty_or_one_left(&RRqueue)) {

                current_pcb->remaining_time -= curr - curr_q_start;
                kill(current_pcb->pid, SIGSTOP);
                current_pcb->cum_runtime += curr - curr_q_start;
                current_pcb->waiting_time = curr - current_pcb->arrival_time - current_pcb->cum_runtime;
                print(curr, current_pcb, NULL, 'p');
                current_pcb->state = READY; // back to Ready state
                circular_advance_queue(&RRqueue);
                process_is_currently_running = false;
            }
        }


        if (!process_is_currently_running && !circular_is_empty(&RRqueue)) {
            // update the current_pcb as the queue is advanced
            current_pcb = hashmap_get(process_table, &(PCB) {.id = RRqueue.front->data});
            if (current_pcb->pid == 0) { // if current process never started before
                *shm_remain_time = current_pcb->remaining_time;
                int pid = fork();
                if (pid == 0) {
                    // child
                    execl("./process.out", "./process.out", NULL);
                }

                // parent
                current_pcb->pid = pid; // update Pid of existing process

                current_pcb->waiting_time = curr - current_pcb->arrival_time;
                print(curr, current_pcb, NULL, 's');


            } else {

                kill(current_pcb->pid, SIGCONT);
                current_pcb->waiting_time = curr - current_pcb->arrival_time - current_pcb->cum_runtime;
                *shm_remain_time = current_pcb->remaining_time;
                print(curr, current_pcb, NULL, 'r');
            }
            current_pcb->state = RUNNING;
            process_is_currently_running = true;
            curr_q_start = curr; // started a quantum
        }

        // if the current's quantum finished and only one left -> no switch
        // if the current terminated and no other in the Queue -> no switching
    }
    printf("\nOut at time %d\n", curr);
    return curr;
}


//----------------------------------------------------------------
void SRTN() {
    printf("Entering SRTN \n");
    // intialize the priority queue
    minHeap sQueue;
    sQueue = init_min_heap();

    minHeap waiting_queue;
    waiting_queue = init_min_heap();
    PCB *current_pcb = NULL;
    int p_count = TotalNumberOfProcesses;
    int need_to_receive = TotalNumberOfProcesses;
    bool process_is_currently_running = false;
    bool can_insert = true;

    // if the Queue is empty then check if there is no more processes that will come
    // the main loop for the scheduler
    while (!is_empty(&sQueue) || p_count > 0) {
        // First check if any process has come
        count_msg c = {.count = 0};
        if (more_processes_coming || need_to_receive > 0)
            msgrcv(process_msg_queue, &c, sizeof(int), 10, !IPC_NOWAIT);

        int current_time = getClk();
        int num_messages = c.count;
        need_to_receive -= c.count;
        while (num_messages > 0) {
            // while still a process in the queue
            // take it out
            // add it to both the Prority Queue (sQueue) and its PCB to the processTable
            process_struct coming_process;

            msgrcv(process_msg_queue, &coming_process, sizeof(coming_process) - sizeof(coming_process.mtype), 1,
                   !IPC_NOWAIT);
            printf("\nrecv process with id: %d at time %d with priority %d\n", coming_process.id, current_time,
                   coming_process.priority);

            //  you have that struct Now
            PCB pcb= set_process(coming_process);
            // pcb.id = coming_process.id;
            // pcb.pid = 0;
            // // pcb.arrival_time = coming_process.arrival;
            // pcb.arrival_time = current_time;
            // pcb.priority = coming_process.priority;
            // pcb.state = READY;
            // pcb.cum_runtime = 0;
            // pcb.burst_time = coming_process.runtime;     // at the beginning
            // pcb.remaining_time = coming_process.runtime; // at the beginning
            // pcb.waiting_time = 0;
            // pcb.mem_size = coming_process.memsize;                           // at the beginning
            hashmap_set(process_table, &pcb);             // this copies the content of the struct
            push(&waiting_queue, pcb.remaining_time, pcb.id); // add this process to the end of the Queue
            num_messages--;
        }

        if (current_pcb != NULL)
        {
            current_pcb->remaining_time -= (current_time - (current_pcb->arrival_time + current_pcb->cum_runtime + current_pcb->waiting_time));
            current_pcb->cum_runtime = current_pcb->burst_time - current_pcb->remaining_time;
            if (current_pcb->remaining_time <= 0)
            {
                current_pcb->remaining_time = 0;
                current_pcb->cum_runtime = current_pcb->burst_time;
                // kill and out new data
                int dum;
                int ret = wait(&dum);
                int TA = current_time - current_pcb->arrival_time;

                float WTA = (float) TA / current_pcb->burst_time;

                OutputFinishedProcesses(current_time, current_pcb, TA, WTA);
                print(current_time, current_pcb, NULL, 'd');


                buddy_deallocate(current_pcb->memory_start_ind, current_pcb->memory_end_ind);
                hashmap_delete(process_table, current_pcb);
                p_count--;
                current_pcb = NULL;
            }
        }
        minHeap tempQueue;
        tempQueue = init_min_heap();
        if (!is_empty(&waiting_queue))
        {
            while (!is_empty(&waiting_queue))
            {
                node *temp = pop(&waiting_queue);
                PCB *_pcb = hashmap_get(process_table, &(PCB){.id = temp->data});
                pair_t ret;
                //printf("mem size %d\n",_pcb->mem_size);
                can_insert = buddy_allocate(_pcb->mem_size, &ret);
                if (can_insert)
                {
                    _pcb->state = READY; // allocated and in ready Queue
                    _pcb->memory_start_ind = ret.start_ind;
                    _pcb->memory_end_ind = ret.end_ind;
                    push(&sQueue, _pcb->remaining_time, _pcb->id); // add this process to the end of the Queue
                    print(current_time, _pcb, &ret, 'a');
                }
                else
                {
                    push(&tempQueue, _pcb->remaining_time, _pcb->id);
                }
            }
        }
        while (!is_empty(&tempQueue))
        {
            node *temp = pop(&tempQueue);
            push(&waiting_queue, temp->priority, temp->data);
        }
        // 3 cases
        // first is first start process or there is gap between processes -> Done
        // second update the remaining time
        // third is a new process with shortest time came
        if (!is_empty(&sQueue) && current_pcb != NULL) {
            node *temp = peek(&sQueue);
            if (temp != NULL) {
                if (temp->priority < current_pcb->remaining_time) {
                    // swap and stop current process
                    kill(current_pcb->pid, SIGSTOP);
                    print(current_time, current_pcb, NULL, 'p');
                    current_pcb->state = READY; // back to Ready state
                    push(&sQueue, current_pcb->remaining_time,
                         current_pcb->id); // add this process to the end of the Queue

                    current_pcb = NULL;
                }
            }
        }
        if (current_pcb == NULL && !is_empty(&sQueue)) {

            node *temp = pop(&sQueue);
            PCB get_process = {.id = temp->data};
            current_pcb = hashmap_get(process_table, &get_process);
            *shm_remain_time = current_pcb->remaining_time;
            //  first time
            if (current_pcb->pid == 0) {
                int pid = fork();
                if (pid == 0) {
                    // child
                    execl("./process.out", "./process.out", NULL);
                }

                // continue scheduler
                current_pcb->pid = pid; // update Pid of existing process
                current_pcb->state = RUNNING;
                current_pcb->waiting_time = current_time - current_pcb->arrival_time;
                print(current_time, current_pcb, NULL, 's');
            }
                // resumed after stopped
            else {
                kill(current_pcb->pid, SIGCONT);

                // continue scheduler
                current_pcb->state = RUNNING;
                current_pcb->waiting_time = current_time - (current_pcb->arrival_time + current_pcb->cum_runtime);

                // if(current_pcb->id == 3){
                //     printf("current %d arrivl %d -- cum %d -- waiting time %d\n", current_time, current_pcb->arrival_time, current_pcb->cum_runtime, current_pcb->waiting_time);
                // }
                print(current_time, current_pcb, NULL, 'r');

            }
        }
    }
    printf("\nOut at time %d\n", getClk());
}

int HPF() {
    minHeap hpf_queue = init_min_heap();

    pcb_s *current_pcb;
    bool process_is_currently_running = false;
    int started_clk, current_clk=0;
    int need_to_receive = TotalNumberOfProcesses;

    int p_count = TotalNumberOfProcesses;

    while (!is_empty(&hpf_queue) || p_count > 0 || process_is_currently_running) {

        struct count_msg c = {.count = 0};
        if (more_processes_coming || need_to_receive > 0)
            msgrcv(process_msg_queue, &c, sizeof(int), 10, !IPC_NOWAIT);

        current_clk = getClk();
        int num_messages = c.count;
        need_to_receive -= c.count;

        while (num_messages > 0) {
            // while still a process in the queue
            // take it out
            // add it to both the hpf_queue and its PCB to the processTable
            struct process_struct coming_process;
            msgrcv(process_msg_queue, &coming_process, sizeof(coming_process) - sizeof(coming_process.mtype), 0,
                   !IPC_NOWAIT);
            // you have that struct Now
             PCB pcb =set_process(coming_process);
            hashmap_set(process_table, &pcb);                             // this copies the content of the struct
            push(&hpf_queue, coming_process.priority, coming_process.id); // add this process to the priority queue
                        // printf("queue after push %d\n",is_empty(&hpf_queue));

            printf("Received process with priority %d and id %d at time %d \n", coming_process.priority,
                   coming_process.id, getClk());

            num_messages--;
            // printf("End of recieving \n");
        }
        if (!is_empty(&hpf_queue) || process_is_currently_running) {
            if (process_is_currently_running) {


                if (current_clk - started_clk == current_pcb->remaining_time) {
                    int st;
                    // (*shm_remain_time) = 0;
                    int ret = wait(&st);
                    int TA = current_clk - current_pcb->arrival_time;
                    current_pcb->remaining_time = 0;
                    current_pcb->waiting_time = TA - current_pcb->burst_time;
                    float WTA = (float) TA / current_pcb->burst_time;
                    p_count--;
                    buddy_deallocate(current_pcb->memory_start_ind, current_pcb->memory_end_ind);
                    print(current_clk, current_pcb, NULL, 'd');
                    OutputFinishedProcesses(current_clk, current_pcb, TA, WTA);

                    hashmap_delete(process_table, current_pcb);
                    process_is_currently_running = false;
                    // printf("hena\n");
                }
                // printf("AAAa\n");
            }
            // printf("queue %d process is currently running %d\n",is_empty(&hpf_queue),process_is_currently_running);
            if (!process_is_currently_running && !is_empty(&hpf_queue))
            {
                pcb_s get_process = {.id = peek(&hpf_queue)->data};
                current_pcb = hashmap_get(process_table, &get_process);

                (*shm_remain_time) = current_pcb->remaining_time;

                int pid = fork();
                if (pid == 0) {
                    // child
                    // printf("Create process: %d with priority: %d\n", peek(&hpf_queue)->data, peek(&hpf_queue)->priority);
                    execl("./process.out", "./process.out", NULL);
                }
                pair_t ret;
                buddy_allocate(current_pcb->mem_size, &ret);
                // printf("hh\n");   
                started_clk = current_clk;
                process_is_currently_running = true;
                // parent take the pid to the hashmap
                current_pcb->pid = pid; // update Pid of existing process
                current_pcb->state = RUNNING;
                current_pcb->waiting_time = current_clk - current_pcb->arrival_time;
                current_pcb->memory_start_ind = ret.start_ind;
                current_pcb->memory_end_ind = ret.end_ind;
                // printf("current_pcb mem start %d end %d\n",current_pcb->memory_start_ind,current_pcb->memory_end_ind);
                // printf(CYN "At time %d allocated %d bytes for process %d from %d to %d\n" RESET, current_clk, current_pcb->mem_size,
                //        current_pcb->id, ret.start_ind, ret.end_ind);

                // printf("At time %d process %d started arr %d total %d remain %d wait %d\n", current_clk,
                //        current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time,//*shm_remain_time,
                //        current_pcb->waiting_time);
                // fprintf(sch_log, "At time %d process %d started arr %d total %d remain %d wait %d\n", current_clk,
                //         current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time,//*shm_remain_time,
                //         current_pcb->waiting_time);

                // fprintf(mem_log, "At time %d allocated %d bytes for process %d from %d to %d\n" RESET, current_clk, current_pcb->mem_size,
                //        current_pcb->id, ret.start_ind, ret.end_ind);
                // // circular_enQueue(&RRqueue, id);
                print(current_clk, current_pcb, &ret, 'a');
                print(current_clk, current_pcb, NULL, 's');
                pop(&hpf_queue);
                // printf("hena aho\n");
            }
        }
    }
    printf("\nOut at time %d\n", current_clk);
    return current_clk;
}

void scheduler_log() {
    sch_log = fopen("scheduler.log", "w");
    if (sch_log == NULL) {
        printf("error has been occured while creation or opening scheduler.log\n");
    } else {
        fprintf(sch_log, "#At time x process y state arr w total z remain y wait k\n");
    }
}

void memory_log() {
    mem_log = fopen("memory.log", "w");
    if (mem_log == NULL) {
        printf("error has been occured while creation or opening memory.log\n");
    } else {
        fprintf(mem_log, "#At time x allocated y bytes for process z from i to j\n");
    }
}

void scheduler_perf(int ProcessesCount) {
    sch_perf = fopen("scheduler.perf", "w");
    if (sch_perf == NULL) {
        printf("error has been occured while creation or opening scheduler.perf\n");
    } else {
        float CPU_Utilization = ((float) TotalRunTime / getClk()) * 100;
        fprintf(sch_perf, "CPU utilization = %.2f %%\n", CPU_Utilization);
        fprintf(sch_perf, "Avg WTA = %.2f\n", ((float) TotalWTA) / ProcessesCount);
        fprintf(sch_perf, "Avg Waiting = %.2f\n", ((float) TotalWaitingTime) / ProcessesCount);
        fprintf(sch_perf, "Std WTA = %.2f\n", CalcStdWTA(((float) TotalWTA) / ProcessesCount));
    }
}

void OutputFinishedProcesses(int CurrTime, PCB *current_pcb, int TA, float WTA) {
    // printing in a file
    fprintf(sch_log, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", CurrTime,
            current_pcb->id,
            current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time, current_pcb->waiting_time,
            TA, WTA);

    printf(RED "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n" RESET,
           CurrTime, current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time,
           current_pcb->waiting_time, TA, WTA);
    // update stats variables
    TotalRunTime += current_pcb->burst_time;
    TotalWTA += WTA;
    TotalWaitingTime += current_pcb->waiting_time;
    WeightedTA[WTAIterator] = WTA;
    WTAIterator++;
}

void print(int CurrentTime, PCB *current_pcb, pair_t *Index, char type) {
    switch (type) {
        case 's':   //starting process
            fprintf(sch_log, "At time %d process %d started arr %d total %d remain %d wait %d\n", CurrentTime,
                    current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time,
                    current_pcb->waiting_time);

            printf("At time %d process %d started arr %d total %d remain %d wait %d\n", CurrentTime, current_pcb->id,
                   current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time,
                   current_pcb->waiting_time);
            break;

        case 'p' :  //stopping process
            fprintf(sch_log, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", CurrentTime,
                    current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time,
                    current_pcb->waiting_time);
            printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", CurrentTime, current_pcb->id,
                   current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time,
                   current_pcb->waiting_time);
            break;

        case 'r' :  //resuming process
            fprintf(sch_log, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", CurrentTime,
                    current_pcb->id, current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time,
                    current_pcb->waiting_time);

            printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", CurrentTime, current_pcb->id,
                   current_pcb->arrival_time, current_pcb->burst_time, current_pcb->remaining_time,
                   current_pcb->waiting_time);
            break;

        case 'd' :  //memory dellocation
            printf("At time %d freed %d bytes for process %d from %d to %d\n",
                   CurrentTime, current_pcb->mem_size, current_pcb->id, current_pcb->memory_start_ind,
                   current_pcb->memory_end_ind);
            fprintf(mem_log, "At time %d freed %d bytes for process %d from %d to %d\n",
                    CurrentTime, current_pcb->mem_size, current_pcb->id, current_pcb->memory_start_ind,
                    current_pcb->memory_end_ind);
            break;

        case 'a' :  //memory allocation
            printf(CYN "At time %d allocated %d bytes for process %d from %d to %d\n" RESET, CurrentTime,
                   current_pcb->mem_size,
                   current_pcb->id, Index->start_ind, Index->end_ind);

            fprintf(mem_log, "At time %d allocated %d bytes for process %d from %d to %d\n", CurrentTime,
                    current_pcb->mem_size,
                    current_pcb->id, Index->start_ind, Index->end_ind);
            break;
        default:
            break;
    }
}

float CalcStdWTA(float AvgWTA) {
    float numerator = 0;
    float Variance = 0;
    for (int i = 0; i < TotalNumberOfProcesses; i++) {
        numerator += pow((WeightedTA[i] - AvgWTA), 2);
    }
    Variance = numerator / TotalNumberOfProcesses;
    return sqrt(Variance);
}

void FinishPrinting() {
    fclose(sch_log);
    fclose(sch_perf);
    fclose(mem_log);
    free(WeightedTA);
}
