#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "stdbool.h"

#include "utils.h"

#define MAX_CLIENTS 128
#define LOG_BUFFER_SIZE 128
#define INACTIVE -1

typedef struct client_t
{
    int qid;
} client_t;

int server_qid;
client_t clients[MAX_CLIENTS];
int clients_nr = 0;

void update_log(msg_t *msg)
{
    char log_file[] = "log.txt";
    char buf[LOG_BUFFER_SIZE];
    FILE *fp = fopen(log_file, "a");
    if(fp == NULL)
    {
        fprintf(stderr, "Server unable to open file: %s\n", log_file);
        return;
    }
    int sender_id = msg->id;
    if(sender_id >= MAX_CLIENTS || sender_id < 0)
        sender_id = -1;
    time_t t = msg->timestamp;
    sprintf(buf, "COMMAND_TYPE: %2ld ARGS: %10s SENDER_ID: %2d time: %16s",
            msg->type, msg->text, sender_id, asctime(localtime(&t)));
    fprintf(fp, "%s", buf);
    fclose(fp);
}

void clean()
{
    delete_queue(server_qid, get_server_key());
}

void send_to_client(int client_id, msg_t *msg)
{
    if (client_id >= MAX_CLIENTS || clients[client_id].qid == INACTIVE)
    {
        fprintf(stderr, "Invalid client id: %d\n", client_id);
        return;
    }
    if ((send(clients[client_id].qid, msg) == -1))
    {
        fprintf(stderr, "Unable to send message to client (id -> %d)\n", client_id);
    }
}

void handler_sigint(int sig)
{
    if (clients_nr == 0)
    {
        exit(EXIT_SUCCESS);
    }

    msg_t msg;
    msg.type = T_SERVER_SHUTDOWN;
    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i].qid != INACTIVE)
        {
            send_to_client(i, &msg);
        }
    }

    while(clients_nr > 0)
    {
        if(receive(server_qid, &msg) == -1)
        {
            perror("Server unable to receive message");
            continue;
        }
        if(msg.type == T_STOP)
        {
            update_log(&msg);
            clients_nr--;
        }
    }
    printf("Server disconnected!\n");
    exit(EXIT_SUCCESS);
}

int get_client_id()
{
    if(clients_nr >= MAX_CLIENTS)
        return -1;
    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i].qid == INACTIVE)
        {
            clients_nr++;
            return i;
        }
    }
    return -1;
}

void handler_init(msg_t *msg)
{
    key_t client_key;
    sscanf(msg->text, "%d", &client_key);
    int id = get_client_id();

    if(id != -1)
    {
        if ((clients[id].qid = get_queue(client_key)) == -1)
        {
            perror("Server unable to establish client server_qid");
        }

        msg_t init_msg;
        init_msg.type = T_INIT;
        init_msg.id = id;
        send_to_client(id, &init_msg);
    }
    else
    {
        fprintf(stderr, "Unable to register another client\n");
    }
}

void init()
{
    atexit(clean);

    struct sigaction my_sa;
    sigemptyset(&my_sa.sa_mask);
    my_sa.sa_flags = 0;
    my_sa.sa_handler = handler_sigint;
    sigaction(SIGINT, &my_sa, NULL);

    key_t key = get_server_key();

    if((server_qid = create_queue(key)) == -1)
    {
        perror("Server unable to create a server_qid");
        exit(EXIT_FAILURE);
    }

    for(int id = 0; id < MAX_CLIENTS; ++id)
    {
        clients[id].qid = INACTIVE;
    }

    printf("Server established!\n");
}

void handler_stop(msg_t *msg)
{
    int client_id = msg->id;
    if(close_queue(clients[client_id].qid) == -1)
    {
        perror("Unable to close client server_qid");
    }

    clients[client_id].qid = INACTIVE;
    clients_nr--;
    printf("Client (id -> %d) disconnected from server\n", client_id);
}

void handler_list(msg_t *msg)
{
    int request_client_id = msg->id;
    printf("List of active clients (requested by id -> %d):\n", request_client_id);
    printf("----------\n");
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].qid != INACTIVE)
        {
            printf("Client id -> %d\n", i);
        }
    }
    printf("----------\n");
}

void handler_2all(msg_t *msg)
{
    int sender_id = msg->id;
    for(int id = 0; id < MAX_CLIENTS; id++)
    {
        if(clients[id].qid != INACTIVE && id != sender_id)
        {
            send(clients[id].qid, msg);
        }
    }
}

void handler_2one(msg_t *msg)
{
    int sender_id = msg->id;
    int receiver_id = msg->to_id;
    if(sender_id == receiver_id)
    {
        fprintf(stderr, "Invalid operation: client (id -> %d) tried to send message to itself!\n", sender_id);
    }
    else if(sender_id >= MAX_CLIENTS || clients[receiver_id].qid == INACTIVE)
    {
        fprintf(stderr, "Invalid operation: client (id -> %d) tried to send message to inactive client "
                        "(id -> %d)!\n", sender_id, receiver_id);
    }
    else
    {
        send(clients[receiver_id].qid, msg);
    }
}

int main(int argc, char *argv[])
{
    init();
    msg_t msg;
    while(true)
    {
        if(receive(server_qid, &msg) == -1)
        {
            perror("Server unable to receive message");
            continue;
        }
        update_log(&msg);
        switch(msg.type)
        {
            case T_INIT:
            {
                handler_init(&msg);
                break;
            }
            case T_STOP:
            {
                handler_stop(&msg);
                break;
            }
            case T_LIST:
            {
                handler_list(&msg);
                break;
            }
            case T_2ALL:
            {
                handler_2all(&msg);
                break;
            }
            case T_2ONE:
            {
                handler_2one(&msg);
                break;
            }
            default:
            {
                fprintf(stderr, "Server received invalid msg type: %ld\n", msg.type);
                break;
            }
        }
    }

    return 0;
}