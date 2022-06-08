#define _POSIX_C_SOURCE 200112L
#define MAX_PLAYERS 20
#define MAX_BACKLOG 10
#define MAX_MESSAGE_LENGTH 256
#define DELIM ":"
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "stdbool.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct
{
    char *name;
    int fd;
    int online;
    struct sockaddr addr;
} client;

client *clients[MAX_PLAYERS] = {NULL};
int clients_nr = 0;

int get_rival(int index)
{
    if (index % 2 == 0)
        return index + 1;
    else
        return index - 1;
}

bool is_client_name_used(char *name)
{
    for (int player_idx = 0; player_idx < MAX_PLAYERS; player_idx++)
    {
        if (clients[player_idx] != NULL && strcmp(clients[player_idx]->name, name) == 0)
        {
            return true;
        }
    }
    return false;
}

int get_client_index(char *name, int fd)
{
    int index = -1;
    for (int player_idx = 0; player_idx < MAX_PLAYERS; player_idx += 2)
    {
        if (clients[player_idx] != NULL && clients[player_idx + 1] == NULL)
        {
            index = player_idx + 1;
            break;
        }
    }
    if (index == -1)
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (clients[i] == NULL)
            {
                index = i;
                break;
            }
        }
    }

    if (index != -1)
    {
        client *new_client = calloc(1, sizeof(client));
        new_client->name = calloc(MAX_MESSAGE_LENGTH, sizeof(char));
        strcpy(new_client->name, name);
        new_client->fd = fd;
        new_client->online = 1;

        clients[index] = new_client;
        clients_nr++;
    }

    return index;
}

int add_client(char *name, struct sockaddr addr, int fd)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0)
        {
            return -1;
        }
    }
    int index = -1;
    for (int i = 0; i < MAX_PLAYERS; i += 2)
    {
        if (clients[i] != NULL && clients[i + 1] == NULL)
        {
            index = i + 1;
            break;
        }
    }
    if (index == -1)
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (clients[i] == NULL)
            {
                index = i;
                break;
            }
        }
    }

    if (index != -1)
    {
        client *new_client = calloc(1, sizeof(client));
        new_client->name = calloc(MAX_MESSAGE_LENGTH, sizeof(char));
        strcpy(new_client->name, name);
        new_client->fd = fd;
        new_client->online = 1;
        new_client->addr = addr;
        clients[index] = new_client;
        clients_nr++;
    }

    return index;
}

int get_client_id_by_name(char *name)
{
    for (int client_idx = 0; client_idx < MAX_PLAYERS; client_idx++)
    {
        if (clients[client_idx] != NULL && strcmp(clients[client_idx]->name, name) == 0)
        {
            return client_idx;
        }
    }
    return -1;
}
void free_client(int index)
{
    free(clients[index]->name);
    free(clients[index]);
    clients[index] = NULL;
    clients_nr--;
    int rival = get_rival(index);
    if (clients[rival] != NULL)
    {
        printf("remove_player: %s\n", clients[rival]->name);
        sendto(clients[rival]->fd, "quit: ", MAX_MESSAGE_LENGTH, 0,
               &clients[rival]->addr, sizeof(struct addrinfo));
        free(clients[rival]->name);
        free(clients[rival]);
        clients[rival] = NULL;
        clients_nr--;
    }
}

void remove_client(char *name)
{
    int index = -1;
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0)
        {
            index = i;
        }
    }
    printf("remove_player -> %s\n", name);
    free_client(index);
}
void check_if_clients_active()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] != NULL && !clients[i]->online)
        {
            remove_client(clients[i]->name);
        }
    }
}
void send_ping_to_all_clients()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] != NULL)
        {
            sendto(clients[i]->fd, "ping: ", MAX_MESSAGE_LENGTH, 0, &clients[i]->addr, sizeof(struct addrinfo));
            clients[i]->online = 0;
        }
    }
}
void get_active_players()
{
    while (true)
    {
        printf("[List of active players]:\n");
        pthread_mutex_lock(&mutex);
        check_if_clients_active();
        send_ping_to_all_clients();
        pthread_mutex_unlock(&mutex);
        sleep(10);
    }
}

int init_server_local_socket(char *path)
{
    int local_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un local_sockaddr;
    memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
    local_sockaddr.sun_family = AF_UNIX;
    strcpy(local_sockaddr.sun_path, path);
    unlink(path);
    bind(local_socket, (struct sockaddr *)&local_sockaddr,
         sizeof(struct sockaddr_un));
    listen(local_socket, MAX_BACKLOG);
    return local_socket;
}

int init_server_network_socket(char *port)
{
    struct addrinfo *info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, port, &hints, &info);
    int network_socket =
            socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    bind(network_socket, info->ai_addr, info->ai_addrlen);
    listen(network_socket, MAX_BACKLOG);
    freeaddrinfo(info);

    return network_socket;
}

int check_clients_msg(int local_socket, int network_socket)
{
    struct pollfd fds[2];
    fds[0].fd = local_socket;
    fds[0].events = POLLIN;
    fds[1].fd = network_socket;
    fds[1].events = POLLIN;
    poll(fds, 2, -1);
    for (int i = 0; i < 2; i++)
    {
        if (fds[i].revents & POLLIN)
        {
            return fds[i].fd;
        }
    }

    return -1;
}

int main(int argc, char *argv[])
{
    char *port = argv[1];
    char *socket_path = argv[2];
    srand(time(NULL));
    int local_socket = init_server_local_socket(socket_path);
    int network_socket = init_server_network_socket(port);

    pthread_t t;
    pthread_create(&t, NULL, (void * (*)(void *))get_active_players, NULL);
    while (true)
    {
        int socket_fd = check_clients_msg(local_socket, network_socket);
        char buffer[MAX_MESSAGE_LENGTH + 1];
        struct sockaddr from_addr;
        socklen_t from_length = sizeof(struct sockaddr);
        recvfrom(socket_fd, buffer, MAX_MESSAGE_LENGTH, 0, &from_addr, &from_length);

        char *command = strtok(buffer, DELIM);
        char *arg = strtok(NULL, DELIM);
        char *name = strtok(NULL, DELIM);
        pthread_mutex_lock(&mutex);
        if (strcmp(command, "add") == 0)
        {
            printf("add_player -> %s\n", name);
            int index = add_client(name, from_addr, socket_fd);

            if (index == -1)
            {
                sendto(socket_fd, "add:name_taken", MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&from_addr, sizeof(struct addrinfo));
            }
            else if (index % 2 == 0)
            {
                sendto(socket_fd, "add:no_enemy", MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&from_addr, sizeof(struct addrinfo));
            }
            else
            {
                int random_num = rand() % 100;
                int first, second;
                if (random_num % 2 == 0)
                {
                    first = index;
                    second = get_rival(index);
                }
                else
                {
                    second = index;
                    first = get_rival(index);
                }

                sendto(clients[first]->fd, "add:O",
                       MAX_MESSAGE_LENGTH, 0, &clients[first]->addr, sizeof(struct addrinfo));
                sendto(clients[second]->fd, "add:X",
                       MAX_MESSAGE_LENGTH, 0, &clients[second]->addr, sizeof(struct addrinfo));
            }
        }
        if (strcmp(command, "move") == 0)
        {
            int move = atoi(arg);
            int player = get_client_id_by_name(name);
            printf("player_move [%s] -> square_id [%d]\n", name, move+1);

            sprintf(buffer, "move:%d", move);
            sendto(clients[get_rival(player)]->fd, buffer, MAX_MESSAGE_LENGTH,
                   0, &clients[get_rival(player)]->addr, sizeof(struct addrinfo));
        }
        if (strcmp(command, "quit") == 0)
        {
            remove_client(name);
        }
        if (strcmp(command, "active") == 0)
        {
            printf("***%s***\n", name);
            int player = get_client_id_by_name(name);
            if (player != -1)
            {
                clients[player]->online = 1;
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}
