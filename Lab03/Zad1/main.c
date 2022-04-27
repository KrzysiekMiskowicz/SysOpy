#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        perror("Invalid number of arguments given for main program");
        exit(EXIT_FAILURE);
    }
    pid_t child_pid;
    int n = atoi(argv[argc - 1]);
    printf("PID glownego programu: %d\n", (int)getpid());
    for(int i = 0; i < n; i++)
    {
        child_pid = fork();
        if(child_pid == 0) {
            printf("Proces rodzica ma pid: %d, pid: %d\n", (int)getppid(), (int)getpid());
            break;
        }
    }

    for(int i = 0; i < n; i++)
        wait(NULL);

    return 0;
}
