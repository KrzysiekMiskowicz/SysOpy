#include "utils.h"

void init_semaphores()
{
    for (int i = 0; i < 6; i++)
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
    shm_des = shm_open(SHM_NAME, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shm_des < 0)
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

long long int rand_int(long long int min, long long int max)
{
    return  rand() % (max - min + 1) + min;
}

int get_sem_val(int idx)
{
    int val;
    sem_getvalue(sem[idx], &val);
    return val;
}

void wait_for_empty_arr(int idx)
{
    while(1)
    {
        usleep(200000);
        int taken_spots = get_sem_val(idx);
        if(taken_spots == 0)
        {
            break;
        }
    }
}

void reservate_array(int idx)
{
    wait_for_empty_arr(idx);
    sem_post(sem[idx]);
}

int wait_for_ready_pizza(int *pizza_id, pizzas **pizzas_ptr_ptr)
{
//    printf("%d In wait_for_ready_pizza\n", getppid());
    sem_wait(sem[PIZZAS_ON_TABLE_SEM]);

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

void wait_for_free_spot(int idx)
{
    while(1)
    {
        usleep(200000);
        int taken_spots = get_sem_val(idx);
        if(taken_spots < MAX_PIZZAS)
        {
            break;
        }
    }
}

void add_to_array(int idx, int pizza_id, int *place_id_ptr,
                  pizzas **pizzas_ptr_ptr)
{

    sem_post(sem[idx+1]);
    sem_post(sem[idx+2]);

//    printf("%d After semop in add_to_array\n", getpid());
    int made_pizzas = get_sem_val(idx+2);
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
            break;
        }
    }
}

void free_array_spot(int idx, int arr_idx, pizzas **pizzas_ptr_ptr)
{
    int *arr_ptr = NULL;
    if(idx - 1 == STOVE_SEM_NR)
        arr_ptr = (*pizzas_ptr_ptr)->stove;
    else
        arr_ptr = (*pizzas_ptr_ptr)->table;
    *(arr_ptr + arr_idx) = FREE_SPOT;

    sem_wait(sem[idx]);
}

void free_arrray_reservation(int idx)
{
    sem_wait(sem[idx]);
}

void add_pizza_to_stove(int pizza_id, int *place_id_ptr,
                        int *pizzas_in_stove_ptr, pizzas **pizzas_ptr_ptr)
{
    reservate_array(STOVE_SEM_NR);
//    printf("%d + add_pizza\n", getpid());
    wait_for_free_spot(PIZZAS_IN_STOVE_SEM);
//    printf("%d ++ add_pizza\n", getpid());
    add_to_array( STOVE_SEM_NR, pizza_id, place_id_ptr, pizzas_ptr_ptr);
//    printf("%d +++ add_pizza\n", getpid());
    *pizzas_in_stove_ptr = get_sem_val(PIZZAS_IN_STOVE_SEM);
//    printf("%d ++++ add_pizza\n", getpid());
    free_arrray_reservation(STOVE_SEM_NR);
//    printf("%d +++++ add_pizza\n", getpid());
}

void take_pizza_from_stove_to_table(int pizza_id, int *pizzas_in_stove_ptr,
                                    int *pizzas_in_table_ptr, int stove_place_id,
                                    pizzas **pizzas_ptr_ptr)
{
    int table_spot_id;
//    printf("%d + take_pizza\n", getpid());
    reservate_array(TABLE_SEM_NR);
//    printf("%d ++ take_pizza\n", getpid());
    wait_for_free_spot(PIZZAS_ON_TABLE_SEM);
//    printf("%d +++ take_pizza\n", getpid());
    free_array_spot(PIZZAS_IN_STOVE_SEM, stove_place_id, pizzas_ptr_ptr);
//    printf("%d ++++ take_pizza\n", getpid());
    add_to_array(TABLE_SEM_NR, pizza_id, &table_spot_id, pizzas_ptr_ptr);
//    printf("%d +++++ take_pizza\n", getpid());
    *pizzas_in_stove_ptr = get_sem_val(PIZZAS_IN_STOVE_SEM);
//    printf("%d ++++++ take_pizza\n", getpid());
    *pizzas_in_table_ptr = get_sem_val(PIZZAS_ON_TABLE_SEM);
    free_arrray_reservation(TABLE_SEM_NR);
}

void deliver_pizza(int *pizza_id, int *pizzas_in_table_ptr, pizzas **pizzas_ptr_ptr)
{
//    printf("%d + deliver_pizza\n", getpid());
    reservate_array(DELIVERY_SEM_NR);
//    printf("%d ++ deliver_pizza\n", getpid());
    int table_spot_id = wait_for_ready_pizza(pizza_id, pizzas_ptr_ptr);
//    printf("%d +++ deliver_pizza\n", getpid());
    free_array_spot(PIZZAS_ON_TABLE_SEM, table_spot_id, pizzas_ptr_ptr);
//    printf("%d ++++ deliver_pizza\n", getpid());
    *pizzas_in_table_ptr = get_sem_val(PIZZAS_ON_TABLE_SEM);
//    printf("%d +++++ deliver_pizza\n", getpid());
    free_arrray_reservation(DELIVERY_SEM_NR);
}
