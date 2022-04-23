#include "circular_queue.h"

int main(int argc, char **argv)
{

    printf("Testing ---------------------------\n");
    struct c_queue qq;

    circular_enQueue(&qq, 4);
    circular_enQueue(&qq, 5);
    circular_enQueue(&qq, 6);
    circular_enQueue(&qq, 7);
    displayQueue(&qq);
    circular_deQueue(&qq);
    displayQueue(&qq);
    circular_advance_queue(&qq);
    displayQueue(&qq);
    circular_deQueue(&qq);
    displayQueue(&qq);
    circular_advance_queue(&qq);
    displayQueue(&qq);
    circular_deQueue(&qq);
    displayQueue(&qq);
    circular_advance_queue(&qq);
    displayQueue(&qq);
    return 0;
}