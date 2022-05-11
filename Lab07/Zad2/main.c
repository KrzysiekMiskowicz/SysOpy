#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "utils.h"

#define COOK_MAN 3
#define DELIVERY_MAN 3

pid_t pids[COOK_MAN + DELIVERY_MAN];

int shm_descriptor;
sem_t *sem[SEM_NR];
char *sem_names[SEM_NR] = {"/STOVE_SEM", "/IN_STOVE", "/MADE","/TABLE_SEM",
                           "/ON_TABLE", "/DELIVERED", "/DELIVER_SEM"};

void create_semaphore()
{
    sem_t *sem_ptr;
    for (int i = 0; i < SEM_NR; i++)
    {
        sem_ptr = sem_open(sem_names[i], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
//        printf("+\n");
        if(sem_ptr == SEM_FAILED)
        {
            printf("Problems meet during creating semaphore %d\n", errno);
            exit(EXIT_FAILURE);
        }
        sem_close(sem_ptr);
    }
}

void create_shared_memory()
{
    shm_descriptor = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if(shm_descriptor < 0)
    {
        printf("Problems meet during creating shared memory %d\n", errno);
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shm_descriptor, sizeof(pizzas)) < 0)
    {
        printf("Problems meet during setting shared memory size %d\n", errno);
        exit(EXIT_FAILURE);
    }
    pizzas *pizzas_ptr = mmap(NULL, sizeof(pizzas), PROT_READ | PROT_WRITE, MAP_SHARED, shm_descriptor, 0);
    for(int i = 0; i < MAX_PIZZAS; i++)
    {
        pizzas_ptr->stove[i] = FREE_SPOT;
        pizzas_ptr->table[i] = FREE_SPOT;
    }
}

void clean()
{
    for (int i = 0; i < COOK_MAN + DELIVERY_MAN; i++)
    {
        kill(pids[i], SIGINT);
    }
    for (int i = 0; i < SEM_NR; i++)
    {
//        printf("++\n");
        if (sem_unlink(sem_names[i]) < 0)
        {
            printf("Problems meet during unlinking semaphore %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }
    if (shm_unlink(SHM_NAME))
    {
        printf("Problems meet during deleting shared memory %d\n", errno);
        exit(EXIT_FAILURE);
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
//        printf("+\n");
        pid_t child_pid = fork();
        if (child_pid == 0)
        {
            execlp("./cookman", "cookman", NULL);
        }
        pids[worker_idx] = child_pid;
    }

    for (int worker_idx = 0; worker_idx < DELIVERY_MAN; worker_idx++)
    {
//        printf("++\n");
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
//    printf("-\n");
    signal(SIGINT, handle_SIGINT);
//    printf("--\n");
    atexit(clean);
//    printf("---\n");
    create_semaphore();
//    printf("----\n");
    create_shared_memory();
//    printf("-----\n");
}

int main()
{
    init();
    run();
    exit(EXIT_SUCCESS);
}