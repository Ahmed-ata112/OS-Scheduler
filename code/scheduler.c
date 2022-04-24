#include "headers.h"
#include "hashmap.h"
#include "circular_queue.h"

enum state {
    READY = 1,
    RUNNING = 2,
};


struct PCB {
    int id; //this is the key in the hashmap
    int pid;
    int arrival_time;
    int priority;
    short state;
    int cum_runtime;
    int remaining_time;
    int waiting_time;
};

// 3 functions related to the hashmap
int process_compare(const void *a, const void *b, void *udata) {
    const struct PCB *process_a = a;
    const struct PCB *process_b = b;
    return (process_a->id == process_b->id ? 0 : 1);
}

bool process_iter(const void *item, void *udata) {
    const struct PCB *process = item;
    printf("process: (id=%d) (arrivalTime=%d) (runTime=%d) (priority=%d) (pid=%d) (state=%d) (remainingTime=%d)\n",
           process->pid, process->arrival_time, process->cum_runtime, process->priority, process->pid, process->state,
           process->remaining_time);
    return true;
}

uint64_t process_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct PCB *process = item;
    return hashmap_murmur(&process->pid, process->pid, seed0, seed1);
}

// TODO init this in the main of the scheduler
struct hashmap *process_table;
int process_msg_queue;

// when process generator tells us that there is no more to come
// set this to false

bool more_processes_coming = true;

int main(int argc, char *argv[]) {
    initClk();

    // TODO implement the scheduler :)
    // upon termination release the clock resources.

    destroyClk(true);
}

void RR() {
    /**
     * i loop all the time
     * till a variable tells me that there is no more proccesses comming
     * this is when i quit
     * All the processes that in the circular queue are in the process_table
     * when finished -> u delete from both
     *
     */
    struct c_queue RRqueue;
    // if the Queue is empty then check if there is no more processes that will come
    while (circular_is_empty(&RRqueue) == false || more_processes_coming) {

        struct msqid_ds buf;
        int num_messages;

        msgctl(process_msg_queue, IPC_STAT, &buf);
        num_messages = buf.msg_qnum;
        while (num_messages > 0) {
            // while still a process in the queue
            // take it out
            // add it to both the RRqueue and its PCB to the processTable
            struct process_struct coming_process;
            msgrcv(process_msg_queue, &coming_process, sizeof(coming_process) - sizeof(coming_process.mtype), 0,
                   !IPC_NOWAIT);
            //you have that struct Now
            struct PCB pcb;
            pcb.id = coming_process.id;
            pcb.priority = coming_process.priority;
            pcb.arrival_time = coming_process.arrival;
            pcb.remaining_time = coming_process.runtime; // at the beginning
            hashmap_set(process_table, &pcb); // this copies the content of the struct
            circular_enQueue(&RRqueue, coming_process.id); // add this process to the end of the Queue
            num_messages--;
        }








    }
}
