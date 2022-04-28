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
    char qname[CLIENT_NAME_LEN];
    mqd_t qdesc;
} client_t;

mqd_t server_qdesc;
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
    sprintf(buf, "COMMAND_TYPE: %2ld ARGS: %30s SENDER_ID: %2d time: %16s",
            msg->type, msg->text, sender_id, asctime(localtime(&t)));
    fprintf(fp, "%s", buf);
    fclose(fp);
}

void clean()
{
    delete_queue(SERVER_NAME);
}

void send_to_client(int client_id, msg_t *msg)
{
    if (client_id >= MAX_CLIENTS || clients[client_id].qdesc == INACTIVE)
    {
        fprintf(stderr, "Invalid client id: %d\n", client_id);
        return;
    }
    send_message(clients[client_id].qdesc, msg, msg->type);

}

void handler_sigint(int sig)
{
    msg_t msg;
    msg.type = T_SERVER_SHUTDOWN;
    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i].qdesc != INACTIVE)
        {
            send_to_client(i, &msg);
        }
    }

    int type;
    while(clients_nr > 0)
    {
        receive_message(server_qdesc, &msg, &type);
        if(type == T_STOP)
        {
            update_log(&msg);
            clients_nr--;
        }
        delete_queue(clients[msg.id].qname);
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
        if (clients[i].qdesc == INACTIVE)
        {
            clients_nr++;
            return i;
        }
    }
    return -1;
}

void handler_init(msg_t *msg)
{
    char client_qname[CLIENT_NAME_LEN];
    sscanf(msg->text, "%s", client_qname);
    int id = get_client_id();

    if(id != -1)
    {
        clients[id].qdesc = get_queue(client_qname);
        strcpy(clients[id].qname, client_qname);
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

    if((server_qdesc = create_queue(SERVER_NAME)) == -1)
    {
        perror("Server unable to create a server queue");
        exit(EXIT_FAILURE);
    }

    for(int id = 0; id < MAX_CLIENTS; ++id)
    {
        strcpy(clients[id].qname, "");
        clients[id].qdesc = INACTIVE;
    }

    printf("Server established!\n");
}

void handler_stop(msg_t *msg)
{
    int client_id = msg->id;
    delete_queue(clients[client_id].qname);

    clients[client_id].qdesc = INACTIVE;
    strcpy(clients[client_id].qname, "");
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
        if(clients[i].qdesc != INACTIVE)
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
        if(clients[id].qdesc != INACTIVE && id != sender_id)
        {
            send_message(clients[id].qdesc, msg, T_2ALL);
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
    else if(sender_id >= MAX_CLIENTS || clients[receiver_id].qdesc == INACTIVE)
    {
        fprintf(stderr, "Invalid operation: client (id -> %d) tried to send message to inactive client "
                        "(id -> %d)!\n", sender_id, receiver_id);
    }
    else
    {
        send_message(clients[receiver_id].qdesc, msg, T_2ONE);
    }
}

int main(int argc, char *argv[])
{
    init();
    while(true)
    {
        int type;
        msg_t msg;
        receive_message(server_qdesc, &msg, &type);

//        printf("List msg params:\n");
//        printf("type: %ld\n", msg.type);
//        printf("id: %d\n", msg.id);
//        printf("qname: %s\n", msg.text);
//        printf("\n");

        update_log(&msg);
        switch(type)
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