#include "utils.h"

//int shm_descriptor;
//sem_t *sem[SEM_NR];

//void init_semaphores()
//{
//    for (int i = 0; i < SEM_NR; i++)
//    {
//        sem[i] = sem_open(sem_names[i], O_RDWR);
//        if (sem[i] < 0)
//        {
//            printf("Problems meet during accessing semaphore %d\n", errno);
//            exit(EXIT_FAILURE);
//        }
//    }
//}
//
//void init_shared_memory()
//{
//    shm_descriptor = shm_open(SHM_NAME, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
//    if (shm_descriptor < 0)
//    {
//        printf("Problems meet during accessing shared memory %d\n", errno);
//        exit(EXIT_FAILURE);
//    }
//}

//void worker_handle_SIGINT(int signum)
//{
//    for (int i = 0; i < SEM_NR; i++)
//    {
//        if (sem_close(sem[i]) < 0)
//        {
//            printf("Problems meet during closing semaphore %d\n", errno);
//            exit(EXIT_FAILURE);
//        }
//    }
//    exit(EXIT_SUCCESS);
//}

long long int rand_int(long long int min, long long int max)
{
    return  rand() % (max - min + 1) + min;
}

int get_sem_val(sem_t **sem, int idx)
{
    int val;
    sem_getvalue(sem[idx], &val);
    return val;
}

void wait_for_empty_arr(sem_t **sem, int idx)
{
    while(1)
    {
        usleep(200000);
        int taken_spots = get_sem_val(sem, idx);
        if(taken_spots == 0)
        {
            break;
        }
    }
}

void wait_for_el_in_array(sem_t **sem, int idx)
{
    while(1)
    {
        usleep(200000);
        int taken_spots = get_sem_val(sem, idx);
        if(taken_spots > 0)
        {
            break;
        }
    }
}

void reservate_array(sem_t **sem, int idx)
{
    wait_for_empty_arr(sem, idx);
    sem_post(sem[idx]);
}

int wait_for_ready_pizza(sem_t **sem, int *pizza_id, pizzas **pizzas_ptr_ptr)
{
//    printf("%d In wait_for_ready_pizza\n", getppid());

    wait_for_el_in_array(sem, PIZZAS_ON_TABLE_SEM);
//    sem_wait(sem[PIZZAS_ON_TABLE_SEM]);

    for(int spot_id = 0; spot_id < MAX_PIZZAS; spot_id++)
    {
        if((*pizzas_ptr_ptr)->table[spot_id] != FREE_SPOT)
        {
            *pizza_id = (*pizzas_ptr_ptr)->table[spot_id];
            return spot_id;
        }
    }
//    printf("Error\n");
    exit(EXIT_FAILURE);
}

void wait_for_free_spot(sem_t **sem, int idx)
{
    while(1)
    {
        usleep(200000);
        int taken_spots = get_sem_val(sem, idx);
        if(taken_spots < MAX_PIZZAS)
        {
            break;
        }
    }
}

void add_to_array(sem_t **sem, int idx, int pizza_id, int *place_id_ptr,
                  pizzas **pizzas_ptr_ptr)
{

    sem_post(sem[idx+1]);
    sem_post(sem[idx+2]);

//    printf("%d After semop in add_to_array\n", getpid());
    int made_pizzas = get_sem_val(sem, idx+2);
    int start_spot = (made_pizzas - 1) % MAX_PIZZAS;
    int *arr_ptr;
    if(idx == STOVE_SEM_NR)
        arr_ptr = (*pizzas_ptr_ptr)->stove;
    else
        arr_ptr = (*pizzas_ptr_ptr)->table;

    for(int spot_id = start_spot; spot_id < start_spot + MAX_PIZZAS; spot_id++)
    {
        spot_id %= MAX_PIZZAS;
        if(*(arr_ptr + spot_id) == FREE_SPOT)
        {
//            printf("%d Break in add_to_array\n", getpid());
            *place_id_ptr = spot_id;
//            printf("%d + Break in add_to_array\n", getpid());
            *(arr_ptr + *place_id_ptr) = pizza_id;
//            printf("%d ++ Break in add_to_array\n", getpid());
            return;
        }
    }
    fprintf(stderr, "Invalid array state\n");
    printf("_____________\n");
    for(int i = 0; i < MAX_PIZZAS; i++)
    {
        printf("stove[%d] -> %d\n", i, (*pizzas_ptr_ptr)->stove[i]);
        printf("table[%d] -> %d\n", i, (*pizzas_ptr_ptr)->table[i]);
    }
    printf("_____________\n");
}

void free_array_spot(sem_t **sem, int idx, int arr_idx, pizzas **pizzas_ptr_ptr)
{
    int *arr_ptr;
    if(idx - 1 == STOVE_SEM_NR)
        arr_ptr = (*pizzas_ptr_ptr)->stove;
    else
        arr_ptr = (*pizzas_ptr_ptr)->table;
    *(arr_ptr + arr_idx) = FREE_SPOT;
//    int val = get_sem_val(sem, idx);
    sem_wait(sem[idx]);
}

void free_arrray_reservation(sem_t **sem, int idx)
{
    sem_wait(sem[idx]);
}

void raport(sem_t **sem)
{
    for(int i = 0; i < SEM_NR; i++)
    {
        printf("sem[%d] -> %d\n", i, get_sem_val(sem, i));
    }
}

void add_pizza_to_stove(sem_t **sem, int pizza_id, int *place_id_ptr,
                        int *pizzas_in_stove_ptr, pizzas **pizzas_ptr_ptr)
{
    reservate_array(sem, STOVE_SEM_NR);
//    printf("%d + add_pizza\n", getpid());
    wait_for_free_spot(sem, PIZZAS_IN_STOVE_SEM);
//    printf("%d ++ add_pizza\n", getpid());
    add_to_array(sem, STOVE_SEM_NR, pizza_id, place_id_ptr, pizzas_ptr_ptr);
//    printf("%d +++ add_pizza\n", getpid());
    *pizzas_in_stove_ptr = get_sem_val(sem, PIZZAS_IN_STOVE_SEM);
//    printf("%d ++++ add_pizza\n", getpid());
    free_arrray_reservation(sem, STOVE_SEM_NR);
//    printf("%d +++++ add_pizza\n", getpid());
//    raport(sem);
}

void take_pizza_from_stove_to_table(sem_t **sem, int pizza_id, int *pizzas_in_stove_ptr,
                                    int *pizzas_in_table_ptr, int stove_place_id,
                                    pizzas **pizzas_ptr_ptr)
{
    int table_spot_id;
//    printf("%d + take_pizza\n", getpid());
    reservate_array(sem, TABLE_SEM_NR);
//    printf("%d ++ take_pizza\n", getpid());
    wait_for_free_spot(sem, PIZZAS_ON_TABLE_SEM);
//    printf("%d +++ take_pizza\n", getpid());
    free_array_spot(sem, PIZZAS_IN_STOVE_SEM, stove_place_id, pizzas_ptr_ptr);
//    printf("%d ++++ take_pizza\n", getpid());
//    int pizzas_on_table = get_sem_val(sem, PIZZAS_ON_TABLE_SEM);
//    printf("Before pizzas on table -> %d\n", pizzas_on_table);
    add_to_array(sem, TABLE_SEM_NR, pizza_id, &table_spot_id, pizzas_ptr_ptr);
//    pizzas_on_table = get_sem_val(sem, PIZZAS_ON_TABLE_SEM);
//    printf("After pizzas on table -> %d\n", pizzas_on_table);
//    printf("%d +++++ take_pizza\n", getpid());
    *pizzas_in_stove_ptr = get_sem_val(sem, PIZZAS_IN_STOVE_SEM);
//    printf("%d ++++++ take_pizza\n", getpid());
    *pizzas_in_table_ptr = get_sem_val(sem, PIZZAS_ON_TABLE_SEM);
    free_arrray_reservation(sem, TABLE_SEM_NR);
//    raport(sem);
}

void deliver_pizza(sem_t **sem, int *pizza_id, int *pizzas_in_table_ptr, pizzas **pizzas_ptr_ptr)
{
//    printf("%d + deliver_pizza\n", getpid());
    reservate_array(sem, DELIVERY_SEM_NR);
//    printf("%d ++ deliver_pizza\n", getpid());
    int table_spot_id = wait_for_ready_pizza(sem, pizza_id, pizzas_ptr_ptr);
//    printf("%d +++ deliver_pizza\n", getpid());
    free_array_spot(sem, PIZZAS_ON_TABLE_SEM, table_spot_id, pizzas_ptr_ptr);
//    printf("%d ++++ deliver_pizza\n", getpid());
    *pizzas_in_table_ptr = get_sem_val(sem, PIZZAS_ON_TABLE_SEM);
//    printf("%d +++++ deliver_pizza\n", getpid());
    free_arrray_reservation(sem, DELIVERY_SEM_NR);
//    raport(sem);
}
