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

#define COOK_MAN 6
#define DELIVERY_MAN 6

pid_t pids[COOK_MAN + DELIVERY_MAN];

int sem_id;
int shm_id;

void create_semaphore()
{
    sem_t *sem_ptr;
    for (int i = 1; i < SEM_NR; i++)
    {
        sem_ptr = sem_open(sem_names[i], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
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
    int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd < 0)
    {
        printf("Problems meet during creating shared memory %d\n", errno);
        exit(EXIT_FAILURE);
    }
    if(ftruncate(fd, sizeof(pizzas)) < 0)
    {
        printf("Problems meet during setting shared memory size %d\n", errno);
        exit(EXIT_FAILURE);
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