#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2

struct processInfo
{
  int id;
  int arrivalTime;
  int runTime;
  int priority;
  bool isRunning;
  int pid;
  int remainingTime;
};

int process_compare(const void *a, const void *b, void *udata)
{
  const struct processInfo *process_a = a;
  const struct processInfo *process_b = b;
  return (process_a->id == process_b->id ? 0 : 1);
}

bool process_iter(const void *item, void *udata)
{
  const struct processInfo *process = item;
  printf("process: (id=%d) (arrivalTime=%d) (runTime=%d) (priority=%d) (pid=%d) (state=%d) (remainingTime=%d)\n", process->id, process->arrivalTime, process->runTime, process->priority, process->pid, process->isRunning, process->remainingTime);
  return true;
}

uint64_t process_hash(const void *item, uint64_t seed0, uint64_t seed1)
{
  const struct processInfo *process = item;
  return hashmap_sip(&process->id, process->id, seed0, seed1);
}
typedef struct node {
    int priority ;
    int data;
} node ;

typedef struct minHeap {
    int size ;
    node *elem ;
} minHeap ;


/*
    Function to initialize the min heap with size = 0
*/
minHeap initMinHeap() {
    minHeap hp ;
    hp.size = 0 ;
    return hp ;
}


/*
    Function to swap priority within two nodes of the min heap using pointers
*/
void swap(node *n1, node *n2) {
    node temp = *n1 ;
    *n1 = *n2 ;
    *n2 = temp ;
}


/*
    Heapify function is used to make sure that the heap property is never violated
    In case of deletion of a node, or creating a min heap from an array, heap property
    may be violated. In such cases, heapify function can be called to make sure that
    heap property is never violated
*/
void heapify(minHeap *hp, int i) {
    int smallest = (LCHILD(i) < hp->size && hp->elem[LCHILD(i)].priority < hp->elem[i].priority) ? LCHILD(i) : i ;
    if(RCHILD(i) < hp->size && hp->elem[RCHILD(i)].priority < hp->elem[smallest].priority) {
        smallest = RCHILD(i) ;
    }
    if(smallest != i) {
        swap(&(hp->elem[i]), &(hp->elem[smallest])) ;
        heapify(hp, smallest) ;
    }
}


/*
    Function to insert a node into the min heap, by allocating space for that node in the
    heap and also making sure that the heap property and shape propety are never violated.
*/
void push(minHeap *hp, int priority, int data) {
    if(hp->size) {
        hp->elem = realloc(hp->elem, (hp->size + 1) * sizeof(node)) ;
    } else {
        hp->elem = malloc(sizeof(node)) ;
    }

    node nd ;
    nd.priority = priority ;
    nd.data = data;

    int i = (hp->size)++ ;
    while(i && nd.priority < hp->elem[PARENT(i)].priority) {
        hp->elem[i] = hp->elem[PARENT(i)] ;
        i = PARENT(i) ;
    }
    hp->elem[i] = nd ;
}

int isEmpty(minHeap *hp)
{
  return hp->size == 0;
}
/*
    Function to delete a node from the min heap
    It shall remove the root node, and place the last node in its place
    and then call heapify function to make sure that the heap property
    is never violated
*/
node* pop(minHeap *hp) {
    if(hp->size) {
        struct node n;
        struct node* temp = &n;
        temp->data = hp->elem[0].data;
        temp->priority = hp->elem[0].priority;
        // printf("Deleting node %d\n\n", hp->elem[0].priority) ;
        hp->elem[0] = hp->elem[--(hp->size)] ;
        hp->elem = realloc(hp->elem, hp->size * sizeof(node)) ;
        heapify(hp, 0) ;
        return temp;
    } else {
        // printf("\nMin Heap is empty!\n") ;
        free(hp->elem) ;
        return NULL;
    }
}

int main()
{
  // create a new hash map where each item is a `struct user`. The second
  // argument is the initial capacity. The third and fourth arguments are
  // optional seeds that are passed to the following hash function.
  struct hashmap *map = hashmap_new(sizeof(struct processInfo), 0, 0, 0,
                                    process_hash, process_compare, NULL);

  // Here we'll load some users into the hash map. Each set operation
  // performs a copy of the data that is pointed to in the second argument.
  hashmap_set(map, &(struct processInfo){.id = 1, .arrivalTime = 1, .runTime = 26, .priority = 9, .pid = 0, .isRunning = 0});
  hashmap_set(map, &(struct processInfo){.id = 2, .arrivalTime = 2, .runTime = 26, .priority = 9, .pid = 0, .isRunning = 0});
  hashmap_set(map, &(struct processInfo){.id = 3, .arrivalTime = 3, .runTime = 26, .priority = 9, .pid = 0, .isRunning = 0});
  hashmap_set(map, &(struct processInfo){.id = 4, .arrivalTime = 4, .runTime = 26, .priority = 9, .pid = 0, .isRunning = 0});
  hashmap_set(map, &(struct processInfo){.id = 5, .arrivalTime = 5, .runTime = 26, .priority = 9, .pid = 0, .isRunning = 0});
  hashmap_scan(map, process_iter, NULL);
  struct processInfo *process_ptr;
  struct processInfo process = {.id = 1};

  printf("\n-- delete Jane --\n");
  process_ptr = hashmap_delete(map, &process);
  hashmap_scan(map, process_iter, NULL);
  printf("user found age: %d", (process_ptr->id));

  hashmap_free(map);

  minHeap heap = initMinHeap();
  printf("is empty%d\n", isEmpty(&heap));
  push(&heap, 1, 2);
  printf("is empty%d\n", isEmpty(&heap));
  push(&heap, 0, 8);
  printf("is empty%d\n", isEmpty(&heap));
  push(&heap, 5, 9);
  printf("is empty%d\n", isEmpty(&heap));
  printf("data returned %d\n",pop(&heap)->data);
  printf("is empty%d\n", isEmpty(&heap));
  printf("data returned %d\n",pop(&heap)->data);
  printf("is empty%d\n", isEmpty(&heap));
  printf("data returned %d\n",pop(&heap)->data);
  printf("is empty%d\n", isEmpty(&heap));

}
