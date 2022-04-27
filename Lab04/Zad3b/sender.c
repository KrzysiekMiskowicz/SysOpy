#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "string.h"
#include <unistd.h>
#include "lib.h"

enum MODE Mode;
pid_t catcher_pid;
int sig_nr;
int CTR_SIG;
int END_SIG;


void handler(int sig, siginfo_t *info, void *ucontext)
{
    static int sig_ctr = 1;
    printf("Sender handler ctr -> %d \n", sig_ctr);
    if(sig == END_SIG)
    {
        printf("END_SIG!\nSender received %d signals\n", sig_ctr);
        exit(EXIT_SUCCESS);
    }

    if(sig_ctr < sig_nr)
    {
        sig_ctr += 1;
        sig = CTR_SIG;
    }
    else if(sig_ctr == sig_nr)
    {
        sig = END_SIG;
    }
    printf("Catcher pid: %d\n", catcher_pid);
    send_signal(Mode, catcher_pid, sig);
}

int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        perror("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }
    catcher_pid = atoi(argv[1]);
    sig_nr = atoi(argv[2]);
    if(strcmp(argv[3], "kill") == 0)
    {
        Mode = KILL;
        CTR_SIG = SIGUSR1;
        END_SIG = SIGUSR2;
    }
    else if(strcmp(argv[3], "queue") == 0)
    {
        Mode = SIGQUEUE;
        CTR_SIG = SIGUSR1;
        END_SIG = SIGUSR2;
    }
    else if(strcmp(argv[3], "sigrt") == 0)
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

    printf("Sender PID: %d\n", (int)getpid());
    send_signal(Mode, catcher_pid, CTR_SIG);
    while(1)
    {
        sleep(1);
    }
    return 0;
}
