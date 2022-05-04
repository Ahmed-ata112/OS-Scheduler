#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>

void proc_exit(int i)
{
    int wstat;

    pid_t pid;

    while (1)
    {
        printf("Return code: %d\n", i);
    }
}
int main()
{
    // signal(SIGCHLD, proc_exit);
    // switch (fork())
    // {
    // case -1:
    //     perror("main: fork");
    //     exit(0);
    // case 0:
    //     printf("I'm alive (temporarily)\n");
    //     exit(1);
    // default:
    //     sleep(5);
    // }

    printf("%d", !(1!=2));
    return 0;
}