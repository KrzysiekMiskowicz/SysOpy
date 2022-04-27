#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "signal.h"
#include "string.h"
#include <unistd.h>

#define SIGNAL SIGUSR1

int main(int argc, char *argv[])
{
    if(strcmp(argv[1], "pending") != 0)
    {
        raise(SIGNAL);
    }
    if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0)
    {
        sigset_t pending_signals;
        sigpending(&pending_signals);
        if(sigismember(&pending_signals, SIGNAL))
            printf("Signal in child after exec is pending\n");
        else
            printf("Signal in child after exec isn't pending\n");
    }
    return 0;
}
