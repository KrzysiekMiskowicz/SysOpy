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
#include <signal.h>

#include "utils.h"

#define COOK_MAN 6
#define DELIVERY_MAN 6
#define SEM_NR 7

pid_t pids[COOK_MAN + DELIVERY_MAN];

int sem_id;
int shm_id;

void create_semaphore()
{
    key_t sem_key = ftok(getenv("HOME"), 0);
    sem_id = semget(sem_key, SEM_NR, IPC_CREAT | 0666);
    if (sem_id < 0)
    {
        printf("Problems meet during creating semaphore %d\n", errno);
        exit(EXIT_FAILURE);
    }
    union semun arg;
    arg.val = 0;

    for (int i = 0; i < SEM_NR; i++)
    {
        semctl(sem_id, i, SETVAL, arg);
    }
}

void create_shared_memory()
{
    key_t shm_key = ftok(getenv("HOME"), 1);
    shm_id = shmget(shm_key, sizeof(pizzas), IPC_CREAT | 0666);
    if (shm_id < 0)
    {
        printf("Problems meet during creating shared memory segment %d\n", errno);
        exit(EXIT_FAILURE);
    }

    pizzas *pizzas_ptr = shmat(shm_id, NULL, 0);
    for(int i = 0; i < MAX_PIZZAS; i++)
    {
        pizzas_ptr->stove[i] = FREE_SPOT;
        pizzas_ptr->table[i] = FREE_SPOT;
    }
}

void clean()
{
    semctl(sem_id, 0, IPC_RMID, NULL);
    shmctl(shm_id, IPC_RMID, NULL);
    for (int i = 0; i < COOK_MAN + DELIVERY_MAN; i++)
    {
        kill(pids[i], SIGINT);
    }
}

void handle_SIGINT(int signum)
{
    exit(EXIT_SUCCESS);
}

void run()
{
    for (int worker_idx = 0; worker_idx < COOK_MAN; worker_idx++)
    {
        pid_t child_pid = fork();
        if (child_pid == 0)
        {
            execlp("./cookman", "cookman", NULL);
        }
        pids[worker_idx] = child_pid;
    }

    for (int worker_idx = 0; worker_idx < DELIVERY_MAN; worker_idx++)
    {
        pid_t child_pid = fork();
        if (child_pid == 0)
        {
            execlp("./deliveryman", "deliveryman", NULL);
        }
        pids[worker_idx + COOK_MAN] = child_pid;
    }

    for (int i = 0; i < COOK_MAN + DELIVERY_MAN; i++)
    {
        wait(NULL);
    }
}

void init()
{
    signal(SIGINT, handle_SIGINT);
    atexit(clean);
    create_semaphore();
    create_shared_memory();
}

int main()
{
    init();
    run();
    exit(EXIT_SUCCESS);
}