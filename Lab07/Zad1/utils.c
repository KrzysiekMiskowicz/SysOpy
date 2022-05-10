#include "utils.h"

int get_semaphore()
{
    key_t sem_key = ftok(getenv("HOME"), 0);
    int sem_id = semget(sem_key, 0, 0);
    if (sem_id < 0)
    {
        printf("Problems meet during accessing semaphore %d\n", errno);
        exit(EXIT_FAILURE);
    }
    return sem_id;
}

int get_shared_memory()
{
    key_t shm_key = ftok(getenv("HOME"), 1);
    int shm_id = shmget(shm_key, 0, 0);
    if (shm_id < 0)
    {
        printf("Problems meet during accessing shared memory segment %d\n", errno);
        exit(EXIT_FAILURE);
    }
    return shm_id;
}

long long int rand_int(long long int min, long long int max)
{
    return  rand() % (max - min + 1) + min;
}

void reservate_array(int sem_id, int arr_id)
{
    sembuf *reservation = calloc(2, sizeof(sembuf));

    reservation[0].sem_num = arr_id;
    reservation[0].sem_op = 0;
    reservation[0].sem_flg = 0;

    reservation[1].sem_num = arr_id;
    reservation[1].sem_op = 1;
    reservation[1].sem_flg = 0;

    semop(sem_id, reservation, 2);
    free(reservation);
}

int get_pizzas_nr_in_array(int sem_id, int arr_id)
{
    int pizzas_nr = semctl(sem_id, arr_id, GETVAL, NULL);
    return pizzas_nr;
}

int wait_for_ready_pizza(int sem_id, int *pizza_id, pizzas **pizzas_ptr_ptr)
{
//    printf("%d In wait_for_ready_pizza\n", getppid());
    while(1)
    {
        usleep(20000);
        int pizzas_in_arr = get_pizzas_nr_in_array(sem_id, PIZZAS_ON_TABLE_SEM);
        if(pizzas_in_arr > 0)
        {
//            printf("break\n");
            break;
        }
    }

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

void wait_for_free_spot(int sem_id, int arr_id)
{
    while(1)
    {
        usleep(1000000);
        int pizzas_in_arr = get_pizzas_nr_in_array(sem_id, arr_id);
        if(pizzas_in_arr < MAX_PIZZAS)
        {
            break;
        }
    }
}

void add_to_array(int sem_id, int arr_id, int pizza_id, int *place_id_ptr,
                  pizzas **pizzas_ptr_ptr)
{

    sembuf *add = calloc(2, sizeof(sembuf));

    add[0].sem_num = arr_id+1;
    add[0].sem_op = 1;
    add[0].sem_flg = 0;

    add[1].sem_num = arr_id+2;
    add[1].sem_op = 1;
    add[1].sem_flg = 0;

    semop(sem_id, add, 2);
    free(add);

//    printf("%d After semop in add_to_array\n", getpid());
    int made_pizzas = semctl(sem_id, arr_id+2, GETVAL, NULL);
    int start_spot = (made_pizzas - 1) % MAX_PIZZAS;
    int *arr_ptr;
    if(arr_id == STOVE_SEM_NR)
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

void free_array_spot(int sem_id, int arr_id, int arr_idx, pizzas **pizzas_ptr_ptr)
{
    sembuf *free_spot = calloc(1, sizeof(sembuf));

    free_spot[0].sem_num = arr_id;
    free_spot[0].sem_op = -1;
    free_spot[0].sem_flg = 0;

    semop(sem_id, free_spot, 1);
    free(free_spot);

    int *arr_ptr = NULL;
    if(arr_id - 1 == STOVE_SEM_NR)
        arr_ptr = (*pizzas_ptr_ptr)->stove;
    else
        arr_ptr = (*pizzas_ptr_ptr)->table;
    *(arr_ptr + arr_idx) = FREE_SPOT;
}

void free_arrray_reservation(int sem_id, int arr_id)
{
    sembuf *free_reservation = calloc(1, sizeof(sembuf));

    free_reservation[0].sem_num = arr_id;
    free_reservation[0].sem_op = -1;
    free_reservation[0].sem_flg = 0;

    semop(sem_id, free_reservation, 1);
    free(free_reservation);
}

void add_pizza_to_stove(int sem_id, int pizza_id, int *place_id_ptr,
                        int *pizzas_in_stove_ptr, pizzas **pizzas_ptr_ptr)
{
    reservate_array(sem_id, STOVE_SEM_NR);
//    printf("%d + add_pizza\n", getpid());
    wait_for_free_spot(sem_id, PIZZAS_IN_STOVE_SEM);
//    printf("%d ++ add_pizza\n", getpid());
    add_to_array(sem_id, STOVE_SEM_NR, pizza_id, place_id_ptr, pizzas_ptr_ptr);
//    printf("%d +++ add_pizza\n", getpid());
    *pizzas_in_stove_ptr = get_pizzas_nr_in_array(sem_id, PIZZAS_IN_STOVE_SEM);
//    printf("%d ++++ add_pizza\n", getpid());
    free_arrray_reservation(sem_id, STOVE_SEM_NR);
//    printf("%d +++++ add_pizza\n", getpid());
}

void take_pizza_from_stove_to_table(int sem_id, int pizza_id, int *pizzas_in_stove_ptr,
                                    int *pizzas_in_table_ptr, int stove_place_id,
                                    pizzas **pizzas_ptr_ptr)
{
    int table_spot_id;
//    printf("%d + take_pizza\n", getpid());
    reservate_array(sem_id, TABLE_SEM_NR);
//    printf("%d ++ take_pizza\n", getpid());
    wait_for_free_spot(sem_id, PIZZAS_ON_TABLE_SEM);
//    printf("%d +++ take_pizza\n", getpid());
    free_array_spot(sem_id, PIZZAS_IN_STOVE_SEM, stove_place_id, pizzas_ptr_ptr);
//    printf("%d ++++ take_pizza\n", getpid());
    add_to_array(sem_id, TABLE_SEM_NR, pizza_id, &table_spot_id, pizzas_ptr_ptr);
//    printf("%d +++++ take_pizza\n", getpid());
    *pizzas_in_stove_ptr = get_pizzas_nr_in_array(sem_id, PIZZAS_IN_STOVE_SEM);
//    printf("%d ++++++ take_pizza\n", getpid());
    *pizzas_in_table_ptr = get_pizzas_nr_in_array(sem_id, PIZZAS_ON_TABLE_SEM);
    free_arrray_reservation(sem_id, TABLE_SEM_NR);
}

void deliver_pizza(int sem_id, int *pizza_id, int *pizzas_in_table_ptr, pizzas **pizzas_ptr_ptr)
{
//    printf("%d + deliver_pizza\n", getpid());
    reservate_array(sem_id, DELIVERY_SEM_NR);
//    printf("%d ++ deliver_pizza\n", getpid());
    int table_spot_id = wait_for_ready_pizza(sem_id, pizza_id, pizzas_ptr_ptr);
//    printf("%d +++ deliver_pizza\n", getpid());
    free_array_spot(sem_id, PIZZAS_ON_TABLE_SEM, table_spot_id, pizzas_ptr_ptr);
//    printf("%d ++++ deliver_pizza\n", getpid());
    *pizzas_in_table_ptr = get_pizzas_nr_in_array(sem_id, PIZZAS_ON_TABLE_SEM);
//    printf("%d +++++ deliver_pizza\n", getpid());
    free_arrray_reservation(sem_id, DELIVERY_SEM_NR);
}
