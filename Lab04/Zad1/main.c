#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "signal.h"
#include "string.h"
#include <unistd.h>

#define SIGNAL SIGUSR1

void handler(int sig)
{
    printf("Handler -> receive signal\n");
}

int main(int argc, char *argv[])
{
    sigset_t pending_signals;
    sigset_t parent_mask;
    sigemptyset(&parent_mask);

    if(strcmp(argv[1], "ignore") == 0)
    {
        signal(SIGNAL, SIG_IGN);
    }
    else if(strcmp(argv[1], "handler") == 0)
    {
        signal(SIGNAL, handler);
    }
    else if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0)
    {
        sigaddset(&parent_mask, SIGNAL);
        if (sigprocmask(SIG_SETMASK, &parent_mask, NULL) < 0)
            perror("Program failed in blocking signal");
    }
    else
    {
        perror("Program recived inappropiate argument\n");
        exit(EXIT_FAILURE);
    }
    raise(SIGNAL);

    if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0)
    {
        sigpending(&pending_signals);
        if(sigismember(&pending_signals, SIGNAL))
            printf("Signal in parent is pending\n");
        else
            printf("Signal in parent isn't pending\n");
    }

    pid_t child_pid = fork();
    if (child_pid == 0)
    {
        if(strcmp(argv[1], "pending") != 0)
            raise(SIGNAL);

        if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0)
        {
            sigpending(&pending_signals);
            if(sigismember(&pending_signals, SIGNAL))
                printf("Signal in child after fork is pending\n");
            else
                printf("Signal in child after fork isn't pending\n");
        }
        execl("./child", "./child", argv[1], NULL);
    }
    wait(&child_pid);

    return 0;
}
