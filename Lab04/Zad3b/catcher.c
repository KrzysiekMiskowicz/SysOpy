#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "string.h"
#include <unistd.h>
#include "lib.h"

enum MODE Mode;
int CTR_SIG;
int END_SIG;

void handler(int sig, siginfo_t *info, void *ucontext)
{
    static int sig_nr = 0;
    printf("Catcher handler ctr -> %d \n", sig_nr);
    int sender_pid = info->si_pid;
    if(sender_pid == 0)
    {
        // Bug on MacOS scheduler that causes pid == 0
        perror("Invalid sender PID\n");
        exit(EXIT_FAILURE);
    }
    printf("Sender pid: %d\n", sender_pid);
    send_signal(Mode, sender_pid, sig);
    if(sig == CTR_SIG)
    {
        sig_nr += 1;
    }
    else if(sig == END_SIG)
    {
        printf("Catcher recieved %d signals\n", sig_nr);
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        perror("Invalid number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1], "kill") == 0)
    {
        Mode = KILL;
        CTR_SIG = SIGUSR1;
        END_SIG = SIGUSR2;
    }
    else if(strcmp(argv[1], "queue") == 0)
    {
        Mode = SIGQUEUE;
        CTR_SIG = SIGUSR1;
        END_SIG = SIGUSR2;
    }
    else if(strcmp(argv[1], "sigrt") == 0)
    {
        Mode = SIGRT;
//        CTR_SIG = SIGUSR1;
//        END_SIG = SIGUSR2;
        CTR_SIG = SIGRTMIN + 1;
        END_SIG = SIGRTMIN + 2;
    }
    else
    {
        perror("Invalid type of mode\n");
        exit(EXIT_FAILURE);
    }

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, CTR_SIG);
    sigdelset(&mask, END_SIG);
    sigdelset(&mask, SIGINT);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
    {
        perror("Program failed in blocking signals\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask,  CTR_SIG);
    sigaddset(&act.sa_mask,  END_SIG);
    sigaction(CTR_SIG, &act, NULL);
    sigaction(END_SIG, &act, NULL);

    printf("Catcher PID: %d\n", (int)getpid());

    while(1)
    {
        sleep(1);
    }
    return 0;
}
