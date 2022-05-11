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

void cook()
{
    int pizza_id = rand_int(0, 9);
    printf("[%d %ld] Przygotowuje pizze: %d.\n", getpid(), time(NULL), pizza_id);

    usleep(rand_int(1000000, 2000000));

    int stove_spot_id = -1;
    int pizzas_in_stove = -1;
    pizzas *pizzas_ptr = mmap(NULL, sizeof(pizzas), PROT_READ | PROT_WRITE, MAP_SHARED, shm_des, 0);
//    printf("Before add_pizza_to_stove\n");
    add_pizza_to_stove(pizza_id, &stove_spot_id, &pizzas_in_stove, &pizzas_ptr);

    printf("[%d %ld] Dodałem pizze: %d. Liczba pizz w piecu: %d.\n",
           getpid(), time(NULL), pizza_id, pizzas_in_stove);

    usleep(rand_int(4000000, 5000000));

    int pizzas_in_table = -1;
    take_pizza_from_stove_to_table(pizza_id, &pizzas_in_stove, &pizzas_in_table,
                                   stove_spot_id, &pizzas_ptr);

    printf("[%d %ld] Wyjmuję pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d.\n",
           getpid(), time(NULL), pizza_id, pizzas_in_stove, pizzas_in_table);

    munmap(pizzas_ptr, sizeof(pizzas));
}

int main()
{
//    srand(time(NULL));

    signal(SIGINT, worker_handle_SIGINT);
    init_semaphores();
    init_shared_memory();

    while(1)
    {
        usleep(rand_int(5000000, 8000000));
        cook();
    }
    exit(EXIT_SUCCESS);
}