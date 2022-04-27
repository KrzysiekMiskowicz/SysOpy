#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "string.h"

void send_signal(enum MODE send_mode, int sender_pid, int sig)
{
    if(send_mode == KILL || send_mode == SIGRT)
        kill(sender_pid, sig);
    else
    {
        union sigval value;
        value.sival_ptr = NULL;
        sigqueue(sender_pid, sig, value);
    }
}