#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "utils.h"

int shm_descriptor;
sem_t *sem[SEM_NR];
char *sem_names[SEM_NR] = {"/STOVE_SEM", "/IN_STOVE", "/MADE","/TABLE_SEM",
                           "/ON_TABLE", "/DELIVERED", "/DELIVER_SEM"};

void init_semaphores()
{
    for (int i = 0; i < SEM_NR; i++)
    {
        sem[i] = sem_open(sem_names[i], O_RDWR);
        if (sem[i] < 0)
        {
            printf("Problems meet during accessing semaphore %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }
}

void init_shared_memory()
{
    shm_descriptor = shm_open(SHM_NAME, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shm_descriptor < 0)
    {
        printf("Problems meet during accessing shared memory %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void worker_handle_SIGINT(int signum)
{
    for (int i = 0; i < SEM_NR; i++)
    {
        if (sem_close(sem[i]) < 0)
        {
            printf("Problems meet during closing semaphore %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}
//
//long long int rand_int(long long int min, long long int max)
//{
//    return  rand() % (max - min + 1) + min;
//}
//
//int get_sem_val(int idx)
//{
//    int val;
//    sem_getvalue(sem[idx], &val);
//    return val;
//}
//
//void wait_for_empty_arr(int idx)
//{
//    while(1)
//    {
//        usleep(200000);
//        int taken_spots = get_sem_val(idx);
//        if(taken_spots == 0)
//        {
//            break;
//        }
//    }
//}
//
//void reservate_array(int idx)
//{
//    wait_for_empty_arr(idx);
//    sem_post(sem[idx]);
//}
//
//int wait_for_ready_pizza(int *pizza_id, pizzas **pizzas_ptr_ptr)
//{
// //    printf("%d In wait_for_ready_pizza\n", getppid());
//    sem_wait(sem[PIZZAS_ON_TABLE_SEM]);
//
//    for(int spot_id = 0; spot_id < MAX_PIZZAS; spot_id++)
//    {
//        if((*pizzas_ptr_ptr)->table[spot_id] != FREE_SPOT)
//        {
//            *pizza_id = (*pizzas_ptr_ptr)->table[spot_id];
//            return spot_id;
//        }
//    }
// //    printf("Error\n");
//    exit(EXIT_FAILURE);
//}
//
//void wait_for_free_spot(int idx)
//{
//    while(1)
//    {
//        usleep(200000);
//        int taken_spots = get_sem_val(idx);
//        if(taken_spots < MAX_PIZZAS)
//        {
//            break;
//        }
//    }
//}
//
//void add_to_array(int idx, int pizza_id, int *place_id_ptr,
//                  pizzas **pizzas_ptr_ptr)
//{
//
//    sem_post(sem[idx+1]);
//    sem_post(sem[idx+2]);
//
////    printf("%d After semop in add_to_array\n", getpid());
//    int made_pizzas = get_sem_val(idx+2);
//    int start_spot = (made_pizzas - 1) % MAX_PIZZAS;
//    int *arr_ptr;
//    if(idx == STOVE_SEM_NR)
//        arr_ptr = (*pizzas_ptr_ptr)->stove;
//    else
//        arr_ptr = (*pizzas_ptr_ptr)->table;
//
//    for(int spot_id = start_spot; spot_id < start_spot + MAX_PIZZAS; spot_id++)
//    {
//        spot_id %= MAX_PIZZAS;
//        if(*(arr_ptr + spot_id) == FREE_SPOT)
//        {
////            printf("%d Break in add_to_array\n", getpid());
//            *place_id_ptr = spot_id;
////            printf("%d + Break in add_to_array\n", getpid());
//            *(arr_ptr + *place_id_ptr) = pizza_id;
////            printf("%d ++ Break in add_to_array\n", getpid());
//            break;
//        }
//    }
//}
//
//void free_array_spot(int idx, int arr_idx, pizzas **pizzas_ptr_ptr)
//{
//    int *arr_ptr = NULL;
//    if(idx - 1 == STOVE_SEM_NR)
//        arr_ptr = (*pizzas_ptr_ptr)->stove;
//    else
//        arr_ptr = (*pizzas_ptr_ptr)->table;
//    *(arr_ptr + arr_idx) = FREE_SPOT;
//
//    sem_wait(sem[idx]);
//}
//
//void free_arrray_reservation(int idx)
//{
//    sem_wait(sem[idx]);
//}
//
//void add_pizza_to_stove(int pizza_id, int *place_id_ptr,
//                        int *pizzas_in_stove_ptr, pizzas **pizzas_ptr_ptr)
//{
//    reservate_array(STOVE_SEM_NR);
////    printf("%d + add_pizza\n", getpid());
//    wait_for_free_spot(PIZZAS_IN_STOVE_SEM);
////    printf("%d ++ add_pizza\n", getpid());
//    add_to_array( STOVE_SEM_NR, pizza_id, place_id_ptr, pizzas_ptr_ptr);
////    printf("%d +++ add_pizza\n", getpid());
//    *pizzas_in_stove_ptr = get_sem_val(PIZZAS_IN_STOVE_SEM);
////    printf("%d ++++ add_pizza\n", getpid());
//    free_arrray_reservation(STOVE_SEM_NR);
////    printf("%d +++++ add_pizza\n", getpid());
//}
//
//void take_pizza_from_stove_to_table(int pizza_id, int *pizzas_in_stove_ptr,
//                                    int *pizzas_in_table_ptr, int stove_place_id,
//                                    pizzas **pizzas_ptr_ptr)
//{
//    int table_spot_id;
////    printf("%d + take_pizza\n", getpid());
//    reservate_array(TABLE_SEM_NR);
////    printf("%d ++ take_pizza\n", getpid());
//    wait_for_free_spot(PIZZAS_ON_TABLE_SEM);
////    printf("%d +++ take_pizza\n", getpid());
//    free_array_spot(PIZZAS_IN_STOVE_SEM, stove_place_id, pizzas_ptr_ptr);
////    printf("%d ++++ take_pizza\n", getpid());
//    add_to_array(TABLE_SEM_NR, pizza_id, &table_spot_id, pizzas_ptr_ptr);
////    printf("%d +++++ take_pizza\n", getpid());
//    *pizzas_in_stove_ptr = get_sem_val(PIZZAS_IN_STOVE_SEM);
////    printf("%d ++++++ take_pizza\n", getpid());
//    *pizzas_in_table_ptr = get_sem_val(PIZZAS_ON_TABLE_SEM);
//    free_arrray_reservation(TABLE_SEM_NR);
//}
//
//void deliver_pizza(int *pizza_id, int *pizzas_in_table_ptr, pizzas **pizzas_ptr_ptr)
//{
////    printf("%d + deliver_pizza\n", getpid());
//    reservate_array(DELIVERY_SEM_NR);
////    printf("%d ++ deliver_pizza\n", getpid());
//    int table_spot_id = wait_for_ready_pizza(pizza_id, pizzas_ptr_ptr);
////    printf("%d +++ deliver_pizza\n", getpid());
//    free_array_spot(PIZZAS_ON_TABLE_SEM, table_spot_id, pizzas_ptr_ptr);
////    printf("%d ++++ deliver_pizza\n", getpid());
//    *pizzas_in_table_ptr = get_sem_val(PIZZAS_ON_TABLE_SEM);
////    printf("%d +++++ deliver_pizza\n", getpid());
//    free_arrray_reservation(DELIVERY_SEM_NR);
//}

void cook()
{
    int pizza_id = rand_int(0, 9);
    printf("[%d %ld] Przygotowuje pizze: %d.\n", getpid(), time(NULL), pizza_id);

    usleep(rand_int(1000000, 2000000));

    int stove_spot_id = -1;
    int pizzas_in_stove = -1;
    pizzas *pizzas_ptr = mmap(NULL, sizeof(pizzas), PROT_READ | PROT_WRITE, MAP_SHARED, shm_descriptor, 0);

    if (pizzas_ptr == (void *) -1)
    {
        printf("Cannot mmap shared memory %d\n", errno);
        exit(EXIT_FAILURE);
    }
//    printf("Before add_pizza_to_stove\n");
    add_pizza_to_stove(sem, pizza_id, &stove_spot_id, &pizzas_in_stove, &pizzas_ptr);

    printf("[%d %ld] Dodałem pizze: %d. Liczba pizz w piecu: %d.\n",
           getpid(), time(NULL), pizza_id, pizzas_in_stove);

    usleep(rand_int(4000000, 5000000));

    int pizzas_in_table = -1;
    take_pizza_from_stove_to_table(sem, pizza_id, &pizzas_in_stove, &pizzas_in_table,
                                   stove_spot_id, &pizzas_ptr);

    printf("[%d %ld] Wyjmuję pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d.\n",
           getpid(), time(NULL), pizza_id, pizzas_in_stove, pizzas_in_table);

    munmap(pizzas_ptr, sizeof(pizzas));
}

void clean()
{
    printf("%d EXIT\n", getpid());
}

int main()
{
//    atexit(clean);
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