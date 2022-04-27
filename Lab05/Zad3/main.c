#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "string.h"

int main(int argc, char *argv[])
{
    char *cons[] = {"./consumer", "./TestFiles/fifo", "./TestFiles/result.txt", "20", NULL};
    char *prod1[] = {"./producer", "./TestFiles/fifo","1", "./TestFiles/a.txt", "5", NULL};
    char *prod2[] = {"./producer", "./TestFiles/fifo","8", "./TestFiles/b.txt", "7", NULL};
    char *prod3[] = {"./producer", "./TestFiles/fifo","9", "./TestFiles/c.txt", "8", NULL};
    char *prod4[] = {"./producer", "./TestFiles/fifo","4", "./TestFiles/1.txt", "9", NULL};
    char *prod5[] = {"./producer", "./TestFiles/fifo","2", "./TestFiles/2.txt", "6", NULL};

    mkfifo("./TestFiles/fifo", 0777);

    pid_t pids[6];

    if((pids[0] = fork()) == 0)
        execvp(cons[0], cons);

    if((pids[1] = fork()) == 0)
        execvp(prod1[0], prod1);

    if((pids[2] = fork()) == 0)
        execvp(prod2[0], prod2);

    if((pids[3] = fork()) == 0)
        execvp(prod3[0], prod3);

    if((pids[4] = fork()) == 0)
        execvp(prod4[0], prod4);

    if((pids[5] = fork()) == 0)
        execvp(prod5[0], prod5);

    for (int pid_ctr = 0; pid_ctr < 6; pid_ctr++)
        wait(0);

    printf("Program has finished successfully\n");
    return 0;
}