#define _POSIX_C_SOURCE 200112L
#define MAX_MESSAGE_LENGTH 256
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "stdbool.h"

#define COL_LEN 3
#define ROW_LEN 3
#define BOARD_SIZE 9

int server_socket;
int binded_socket;
int is_client_O;
char buffer[MAX_MESSAGE_LENGTH + 1];
char *name;

typedef enum
{
    FREE,
    O,
    X
} Field;

Field client_field;

typedef struct
{
    int move;
    Field fields[BOARD_SIZE];

} Board;

int move(Board *board, int position)
{
    if (position < 0 || position > BOARD_SIZE || board->fields[position] != FREE)
        return 0;
    board->fields[position] = board->move ? O : X;
    board->move = !board->move;
    return 1;
}

Field check_column(Board *board, int column_idx)
{
    if(board->fields[column_idx] == board->fields[column_idx+ROW_LEN] &&
       board->fields[column_idx] == board->fields[column_idx+2*ROW_LEN])
        return board->fields[column_idx];
    return FREE;
}

Field check_columns(Board *board)
{
    Field column_result;
    for(int column_idx = 0; column_idx < ROW_LEN; column_idx++)
    {
        column_result = check_column(board, column_idx);
        if(column_result != FREE)
            return column_result;
    }
    return FREE;
}

Field check_row(Board *board, int row_idx)
{
    if(board->fields[row_idx] == board->fields[row_idx+1] &&
       board->fields[row_idx] == board->fields[row_idx+2])
        return board->fields[row_idx];
    return FREE;
}

Field check_rows(Board *board)
{
    Field row_result;
    for(int row_idx = 0; row_idx < BOARD_SIZE; row_idx+=ROW_LEN)
    {
        row_result = check_row(board, row_idx);
        if(row_result != FREE)
            return row_result;
    }
    return FREE;
}

Field check_diagonals(Board *board)
{
    int left_upper_idx = 0;
    int right_upper_idx = 2;
    int center_idx = 4;
    int left_bottom_idx = 6;
    int right_bottom_idx = 8;

    if(board->fields[left_upper_idx] == board->fields[center_idx] &&
       board->fields[left_upper_idx] == board->fields[right_bottom_idx])
        return board->fields[left_upper_idx];

    if(board->fields[left_bottom_idx] == board->fields[center_idx] &&
       board->fields[left_bottom_idx] == board->fields[right_upper_idx])
        return board->fields[left_bottom_idx];

    return FREE;
}

Field check_winner(Board *board)
{
    Field result;
    result = check_columns(board);
    if(result != FREE)
        return result;

    result = check_rows(board);
    if(result != FREE)
        return result;

    result = check_diagonals(board);
    return result;
}
Board board;

typedef enum
{
    IDLE,
    WAITING_FOR_RIVAL,
    WAITING_FOR_RIVAL_MOVE,
    RIVAL_MOVE,
    MOVE,
    END
} State;

State state = IDLE;
char *command, *arg;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void exit_()
{
    char buffer[MAX_MESSAGE_LENGTH + 1];
    sprintf(buffer, "quit: :%s", name);
    send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
    exit(0);
}

bool is_draw(Board *board)
{
    for(int field_idx = 0; field_idx < BOARD_SIZE; field_idx++)
    {
        if(board->fields[field_idx] == FREE)
            return false;
    }
    return true;
}

void check_game()
{
    bool end = false;
    Field winner = check_winner(&board);
    if (winner != FREE)
    {
        if ((is_client_O && winner == O) || (!is_client_O && winner == X))
        {
            printf("You won!!!\n");
        }
        else
        {
            printf("You lost!!!\n");
        }
        end = true;
    }

    bool draw = is_draw(&board);

    if (draw && !end)
    {
        printf("You draw!!!\n");
    }

    if (end || draw)
    {
        state = END;
    }
}
void split(char *reply)
{
    command = strtok(reply, ":");
    arg = strtok(NULL, ":");
}

Board new_board()
{
    Board board = {1,
                   {FREE}};
    return board;
}
void draw()
{
    char symbol;
    printf("\n_____________________\n");
    for (int col = 0; col < COL_LEN; col++)
    {
        for (int row = 0; row < ROW_LEN; row++)
        {
            if (board.fields[col * ROW_LEN + row] == FREE)
            {
                symbol = col * ROW_LEN + row + 1 + '0';
            }
            else if (board.fields[col * ROW_LEN + row] == O)
            {
                symbol = 'O';
            }
            else
            {
                symbol = 'X';
            }
            printf("|  %c  |", symbol);
        }
        printf("\n_____________________\n");
    }
    printf("\n");
}

void play()
{
    while (1)
    {
        if (state == IDLE)
        {
            if (strcmp(arg, "name_taken") == 0)
            {
                printf("Cannot acces the name: name is already taken\n");
                exit(EXIT_FAILURE);
            }
            else if (strcmp(arg, "no_enemy") == 0)
            {
                printf("Waiting for joining of a rival\n");
                state = WAITING_FOR_RIVAL;
            }
            else
            {
                board = new_board();
                is_client_O = arg[0] == 'O';
                state = is_client_O ? MOVE : WAITING_FOR_RIVAL_MOVE;
            }
        }
        else if (state == WAITING_FOR_RIVAL)
        {
            pthread_mutex_lock(&mutex);
            while (state != IDLE && state != END)
            {
                pthread_cond_wait(&cond, &mutex);
            }
            pthread_mutex_unlock(&mutex);

            board = new_board();
            is_client_O = arg[0] == 'O';
            state = is_client_O ? MOVE : WAITING_FOR_RIVAL_MOVE;
        }
        else if (state == WAITING_FOR_RIVAL_MOVE)
        {
            printf("Rival's turn\n\n");

            pthread_mutex_lock(&mutex);
            while (state != RIVAL_MOVE && state != END)
            {
                pthread_cond_wait(&cond, &mutex);
            }
            pthread_mutex_unlock(&mutex);
        }
        else if (state == RIVAL_MOVE)
        {
            int pos = atoi(arg);
            move(&board, pos);
            check_game();
            if (state != END)
            {
                state = MOVE;
            }
        }
        else if (state == MOVE)
        {
            draw();

            int pos;
            do
            {
                printf("\nYour turn -> select square for %c: ", is_client_O ? 'O' : 'X');
                scanf("%d", &pos);
                printf("\n");
                pos--;
            } while (!move(&board, pos));

            draw();

            char buffer[MAX_MESSAGE_LENGTH + 1];
            sprintf(buffer, "move:%d:%s", pos, name);
            send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);

            check_game();
            if (state != END)
            {
                state = WAITING_FOR_RIVAL_MOVE;
            }
        }
        else if (state == END)
        {
            exit_();
        }
    }
}
void init_connection_with_server(char *destination, bool is_local)
{
    struct sockaddr_un local_sockaddr;

    if (is_local)
    {
        server_socket = socket(AF_UNIX, SOCK_DGRAM, 0);

        memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
        local_sockaddr.sun_family = AF_UNIX;
        strcpy(local_sockaddr.sun_path, destination);

        connect(server_socket, (struct sockaddr *)&local_sockaddr,
                sizeof(struct sockaddr_un));
        binded_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
        struct sockaddr_un binded_sockaddr;
        memset(&binded_sockaddr, 0, sizeof(struct sockaddr_un));
        binded_sockaddr.sun_family = AF_UNIX;
        sprintf(binded_sockaddr.sun_path, "%d", getpid());
        bind(binded_socket, (struct sockaddr *)&binded_sockaddr,
             sizeof(struct sockaddr_un));
    }
    else
    {
        struct addrinfo *info;

        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        getaddrinfo("127.0.0.1", destination, &hints, &info);

        server_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        connect(server_socket, info->ai_addr, info->ai_addrlen);
        freeaddrinfo(info);
    }
    char buffer[MAX_MESSAGE_LENGTH + 1];
    sprintf(buffer, "add: :%s", name);
    if (is_local)
    {
        sendto(binded_socket, buffer, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&local_sockaddr, sizeof(struct sockaddr_un));
    }
    else
    {
        send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
    }
}
void listen_for_server_msg(bool is_local)
{
    int game_thread_running = 0;
    while (1)
    {
        if (is_local)
        {
            recv(binded_socket, buffer, MAX_MESSAGE_LENGTH, 0);
        }
        else
        {
            recv(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
        }

        split(buffer);

        pthread_mutex_lock(&mutex);
        if (strcmp(command, "add") == 0)
        {
            state = IDLE;
            if (!game_thread_running)
            {
                pthread_t t;
                pthread_create(&t, NULL, (void *(*)(void *))play, NULL);
                game_thread_running = 1;
            }
        }
        else if (strcmp(command, "move") == 0)
        {
            state = RIVAL_MOVE;
        }
        else if (strcmp(command, "quit") == 0)
        {
            state = END;
            exit(0);
        }
        else if (strcmp(command, "ping") == 0)
        {
            sprintf(buffer, "active: :%s", name);
            send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
        }
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[])
{
    name = argv[1];
    char *type = argv[2];
    char *destination = argv[3];

    signal(SIGINT, exit_);
    bool is_local = strcmp(type, "local") == 0;
    init_connection_with_server(destination, is_local);

    listen_for_server_msg(is_local);
    return 0;
}
