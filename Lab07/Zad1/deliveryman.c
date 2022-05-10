#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "utils.h"

int sem_id;
int shm_id;

void deliver()
{
    int pizza_id = -1;
    int pizzas_in_table = -1;
    pizzas *pizzas_ptr = shmat(shm_id, NULL, 0);
    deliver_pizza(sem_id, &pizza_id, &pizzas_in_table, &pizzas_ptr);

    printf("[%d %ld] Pobieram pizze: %d. Liczba pizz na stole: %d.\n",
           getpid(), time(NULL), pizza_id, pizzas_in_table);

    usleep(rand_int(4000000, 5000000));

    printf("[%d %ld] Dostarczam pizze: %d.\n",
           getpid(), time(NULL), pizza_id);

    shmdt(pizzas_ptr);
    usleep(rand_int(4000000, 5000000));
}

int main()
{
//    srand(time(NULL));

    sem_id = get_semaphore();
    shm_id = get_shared_memory();

    while(1)
    {
        usleep(rand_int(5000000, 8000000));
        deliver();
    }
    exit(EXIT_SUCCESS);
}