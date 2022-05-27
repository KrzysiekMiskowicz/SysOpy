#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define REINDEERS_NR 9
#define ELVES_NR 10
#define MAX_ELVES_WAITING_NR 3
#define MAX_PRESENTS_ROUNDS 3

int presents_rounds_ctr = 0;

int elves_queue[ELVES_NR];
int elves_waiting_nr = 0;
bool elves_problems_solved = true;

bool reindeers_waiting = false;
int reindeers_waiting_nr = 0;

pthread_mutex_t mutex_santa = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_reindeer = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_reindeer_delivering = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_elf = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_elf_solving = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_santa = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_reindeer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_reindeer_delivering = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_elf = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_elf_solving = PTHREAD_COND_INITIALIZER;

void reset_elves_queue()
{
    for(int ctr = 0; ctr < MAX_ELVES_WAITING_NR; ctr++)
    {
        elves_queue[ctr] = -1;
    }
}

void work(int min, int max)
{
    srand(time(NULL));
    int timer = rand() % (max - min + 1) + min;
    sleep(timer);
}

void *santa(void *arg)
{
    printf("[Komunikat]: Mikolaj -> zasypiam\n");
    while(true)
    {
        pthread_mutex_lock(&mutex_santa);
        while(elves_waiting_nr < MAX_ELVES_WAITING_NR && reindeers_waiting_nr < REINDEERS_NR)
        {
            pthread_cond_wait(&cond_santa, &mutex_santa);
        }
        pthread_mutex_unlock(&mutex_santa);

        printf("[Komunikat]: Mikolaj -> budze sie\n");

        pthread_mutex_lock(&mutex_reindeer);
        if(reindeers_waiting_nr == REINDEERS_NR)
        {
            reindeers_waiting_nr = 0;
            printf("[Komunikat]: Mikolaj -> dostarczam zabawki\n");

            pthread_mutex_lock(&mutex_reindeer_delivering);
            work(2, 4);
            presents_rounds_ctr++;
            reindeers_waiting = false;
            pthread_cond_broadcast(&cond_reindeer_delivering);
            pthread_mutex_unlock(&mutex_reindeer_delivering);
        }
        pthread_mutex_unlock(&mutex_reindeer);
        
        if(presents_rounds_ctr == MAX_PRESENTS_ROUNDS)
        {
            exit(EXIT_SUCCESS);
        }

        pthread_mutex_lock(&mutex_elf);
        if(elves_waiting_nr == MAX_ELVES_WAITING_NR)
        {
            pthread_mutex_lock(&mutex_elf_solving);
            printf("[Komunikat]: Mikołaj -> rozwiązuje problemy elfów: "
                   "ID -> %d, ID -> %d, ID -> %d\n", elves_queue[0], elves_queue[1], elves_queue[2]);
            work(1, 2);
            reset_elves_queue();
            elves_waiting_nr = 0;
            elves_problems_solved = true;
            pthread_cond_broadcast(&cond_elf_solving);
            pthread_mutex_unlock(&mutex_elf_solving);

            pthread_cond_broadcast(&cond_elf);
        }
        pthread_mutex_unlock(&mutex_elf);

        printf("[Komunikat]: Mikolaj -> zasypiam\n");
    }
}

void *reindeer(void *arg)
{
    int id = *((int *)arg);
    while(true)
    {
        work(5, 10);
        pthread_mutex_lock(&mutex_reindeer);
        reindeers_waiting_nr++;
        printf("[Komunikat]: Renifer (id = %d) -> czeka %d reniferów na Mikołaja\n", id, reindeers_waiting_nr);
        if(reindeers_waiting_nr == 1)
        {
            pthread_mutex_lock(&mutex_reindeer_delivering);
            reindeers_waiting = true;
            pthread_mutex_unlock(&mutex_reindeer_delivering);
        }
        else if(reindeers_waiting_nr == REINDEERS_NR)
        {
            pthread_mutex_lock(&mutex_santa);
            printf("[Komunikat]: Renifer (id = %d) -> wybudzam Mikołaja\n", id);
            pthread_cond_broadcast(&cond_santa);
            pthread_mutex_unlock(&mutex_santa);
        }

        pthread_mutex_unlock(&mutex_reindeer);

        pthread_mutex_lock(&mutex_reindeer_delivering);
        while(reindeers_waiting)
        {
            pthread_cond_wait(&cond_reindeer_delivering, &mutex_reindeer_delivering);
        }
        pthread_mutex_unlock(&mutex_reindeer_delivering);
    }
}

void *elf(void *arg)
{
    int id = *((int *)arg);
    while(true)
    {
        work(2, 5);
        pthread_mutex_lock(&mutex_elf);

        while(elves_waiting_nr >= MAX_ELVES_WAITING_NR)
        {
            printf("[Komunikat]: Elf (id = %d) -> czeka na powrót elfów\n", id);
            pthread_cond_wait(&cond_elf, &mutex_elf);
        }
        elves_queue[elves_waiting_nr] = id;
        elves_waiting_nr++;
        printf("[Komunikat]: Elf (id = %d) -> czeka %d elfów na Mikołaja\n", id, elves_waiting_nr);
        if(elves_waiting_nr == 1)
        {
            pthread_mutex_lock(&mutex_elf_solving);
            elves_problems_solved = false;
            pthread_cond_broadcast(&cond_elf_solving);
            pthread_mutex_unlock(&mutex_elf_solving);
        }
        else if(elves_waiting_nr == MAX_ELVES_WAITING_NR)
        {
            pthread_mutex_lock(&mutex_santa);
            printf("[Komunikat]: Elf (id = %d) -> wybudzam Mikołaja\n", id);
            pthread_cond_broadcast(&cond_santa);
            pthread_mutex_unlock(&mutex_santa);
        }
        pthread_mutex_unlock(&mutex_elf);

        pthread_mutex_lock(&mutex_elf_solving);
        while(!elves_problems_solved)
        {
            pthread_cond_wait(&cond_elf_solving, &mutex_elf_solving);
        }
        pthread_mutex_unlock(&mutex_elf_solving);
    }
}

int main()
{
    reset_elves_queue();

    pthread_t santa_thread;
    pthread_create(&santa_thread, NULL, &santa, NULL);

    pthread_t reindeers_threads[REINDEERS_NR];
    for(int reindeer_ctr = 0; reindeer_ctr < REINDEERS_NR; reindeer_ctr++)
    {
        int* id = malloc(sizeof(int));
        *id = reindeer_ctr;
        pthread_create(&reindeers_threads[reindeer_ctr], NULL, &reindeer, id);
    }

    pthread_t elves_threads[ELVES_NR];
    for(int elf_ctr = 0; elf_ctr < ELVES_NR; elf_ctr++)
    {
        int* id = malloc(sizeof(int));
        *id = elf_ctr;
        pthread_create(&elves_threads[elf_ctr], NULL, &elf, id);
    }

    pthread_join(santa_thread, NULL);
    for(int i = 0; i < REINDEERS_NR; i++)
    {
        pthread_join(reindeers_threads[i], NULL);
    }

    for(int i = 0; i < ELVES_NR; i++)
    {
        pthread_join(elves_threads[i], NULL);
    }

    return EXIT_SUCCESS;
}
