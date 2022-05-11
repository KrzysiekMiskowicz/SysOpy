#ifndef SYSTEMYOPERACYJNE_UTILS_H
#define SYSTEMYOPERACYJNE_UTILS_H

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
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_PIZZAS 5
#define FREE_SPOT -1
#define STOVE_SEM_NR 0
#define PIZZAS_IN_STOVE_SEM 1
#define MADE_PIZZAS_SEM 2
#define TABLE_SEM_NR 3
#define PIZZAS_ON_TABLE_SEM 4
#define DELIVERED_PIZZAS_SEM 5
#define DELIVERY_SEM_NR 6

#define SEM_NR 7
#define SHM_NAME "/SHARED_MEMORY"

//int shm_descriptor;
//sem_t *sem[SEM_NR];
//char *sem_names[SEM_NR] = {"/STOVE_SEM", "/IN_STOVE", "/MADE","/TABLE_SEM",
//                           "/ON_TABLE", "/DELIVERED", "/DELIVER_SEM"};

typedef struct
{
    int stove[MAX_PIZZAS];
    int table[MAX_PIZZAS];
} pizzas;

typedef struct sembuf sembuf;

//void init_semaphores();
//void init_shared_memory();

long long int rand_int(long long int min, long long int max);

void add_pizza_to_stove(sem_t **sem, int pizza_id, int *place_id_ptr,
                        int *pizzas_in_stove_ptr, pizzas **pizzas_ptr_ptr);

void take_pizza_from_stove_to_table(sem_t **sem, int pizza_id, int *pizzas_in_stove_ptr,
                                    int *pizzas_in_table_ptr, int stove_place_id,
                                    pizzas **pizzas_ptr_ptr);

void deliver_pizza(sem_t **sem, int *pizza_id, int *pizzas_in_table_ptr, pizzas **pizzas_ptr_ptr);

void worker_handle_SIGINT(int signum);

#endif //SYSTEMYOPERACYJNE_UTILS_H
