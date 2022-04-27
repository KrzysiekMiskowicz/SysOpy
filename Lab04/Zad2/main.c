#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "signal.h"
#include "string.h"
#include <unistd.h>

void handler_SIGUSR1(int sig, siginfo_t *info, void *ucontext)
{
    printf("Handler SIGUSR1!\n");
    printf("Signal: %d\n", info->si_signo);
    printf("PID: %d\n", info->si_pid);
    printf("Real user ID: %d\n\n", info->si_uid);
}

void handler_SIGUSR2(int sig, siginfo_t *info, void *ucontext)
{
    printf("Handler SIGUSR2!\n");
    printf("Signal: %d\n", info->si_signo);
    printf("PID: %d\n", info->si_pid);
    printf("Exit value: %d\n\n", info->si_status);
}

void handler_SIGPIPE(int sig, siginfo_t *info, void *ucontext)
{
    printf("Handler SIGPIPE!\n");
    printf("Signal: %d\n", info->si_signo);
    printf("PID: %d\n", info->si_pid);
    printf("An errno value: %d\n\n", info->si_errno);
}

void handler_SIGCONT(int sig)
{
    printf("Handler SIGCONT!\n");
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);

    // SA_SIGINFO flag
    act.sa_flags = SA_SIGINFO;

    act.sa_sigaction = handler_SIGUSR1;
    sigaction(SIGUSR1, &act, NULL);
    raise(SIGUSR1);

    act.sa_sigaction = handler_SIGUSR2;
    sigaction(SIGUSR2, &act, NULL);
    raise(SIGUSR2);

    act.sa_sigaction = handler_SIGPIPE;
    sigaction(SIGPIPE, &act, NULL);
    raise(SIGPIPE);

    // SA_SIGINFO flag
    act.sa_flags = SA_ONSTACK;
    act.sa_handler = handler_SIGCONT;
    sigaction(SIGCONT, &act, NULL);
    raise(SIGCONT);
    return 0;
}
