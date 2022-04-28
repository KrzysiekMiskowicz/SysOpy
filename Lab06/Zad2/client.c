#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "stdbool.h"

#include "utils.h"

#define MAX_COMMAND_LEN 256

int server_qdesc, client_qdesc;
int id;

void send_to_server(msg_t *msg)
{
    msg->timestamp = time(NULL);
    send_message(server_qdesc, msg, msg->type);
//    printf("Send msg params:\n");
//    printf("type: %ld\n", msg->type);
//    printf("pid: %d\n", msg->pid);
//    printf("qname: %s\n", msg->text);
}

void enable_queue()
{
//    printf("Registering queue\n");
    struct sigevent ev;
    ev.sigev_notify = SIGEV_SIGNAL;
    ev.sigev_signo = SIGUSR1;
    register_notification(client_qdesc, &ev);
}

void clean()
{
    msg_t msg;
    msg.type = T_STOP;
    msg.id = id;
    strcpy(msg.text, "");
    printf("Client disconnected!\n");

    send_to_server(&msg);
    close_queue(client_qdesc);
}

void handler_sigint(int sig)
{
    exit(EXIT_SUCCESS);
}

void handler_stop()
{
    raise(SIGINT);
}

void handler_list()
{
    msg_t msg;
    msg.type = T_LIST;
    msg.id = id;
    strcpy(msg.text, "");
    send_to_server(&msg);
}

void handler_2all_send(char *text)
{
    printf("Client wants to send broadcast message!\n");
    printf("----------\n%s\n----------\n", text);
    msg_t msg;
    msg.type = T_2ALL;
    msg.id = id;
    sprintf(msg.text, "%s", text);
    send_to_server(&msg);
}

void handler_2one_send(char *id_and_text)
{
    msg_t msg;
    msg.type = T_2ONE;
    msg.id = id;
    
    char delim[] = " ";
    char *text = strpbrk(id_and_text, delim);
    bool empty_text = text == NULL;
    char *id_and_text_tmp = id_and_text;
    strtok_r(id_and_text, delim, &id_and_text_tmp);
    
    msg.to_id = atoi(id_and_text);
    if(!empty_text)
        sprintf(msg.text, "%s", text+1);

    printf("Client wants to send direct message to id -> %d\n", msg.to_id);
    printf("----------\n%s\n----------\n", msg.text);

    send_to_server(&msg);
}

void sender_handler_cmd(char *command, char *text)
{

    if(strcmp("STOP", command) == 0)
    {
//        printf("STOP\n");
        handler_stop();
    }
    else if(strcmp("LIST", command) == 0)
    {
//        printf("LIST\n");
        handler_list();
    }
    else if(strcmp("2ALL", command) == 0)
    {
//        printf("2ALL\n");
        handler_2all_send(text);
    }
    else if(strcmp("2ONE", command) == 0)
    {
//        printf("2ONE\n");
        handler_2one_send(text);
    }
}

void sender()
{
    char line[MAX_COMMAND_LEN];
    char command[MAX_COMMAND_LEN];
    char text[MAX_COMMAND_LEN];
    strcpy(line, "");
    strcpy(command, "");
    strcpy(text, "");
//    printf("Client read input to get message\n");
//    scanf("%s", line);
    fgets(line, MAX_COMMAND_LEN, stdin);
    get_cmd(line, command, text);
    sender_handler_cmd(command, text);
//    printf("Client send message %s\n", command);
//    enable_queue();
}

void handler_2all_receive(msg_t *msg)
{
    printf("Client received broadcast message from id -> %d!\n", msg->id);
    printf("----------\n%s\n----------\n", msg->text);
}

void handler_2one_receive(msg_t *msg)
{
    printf("Client received direct message from id -> %d!\n", msg->id);
    printf("----------\n%s\n----------\n", msg->text);
}

void handler_server_shutdown()
{
    printf("Client receive server shutdown command\n");
    exit(EXIT_SUCCESS);
}

void catcher(int sig)
{
//    printf("Inside catcher!\n");
    int type;
    msg_t message;
    receive_message(client_qdesc, &message, &type);
    switch(type)
    {
        case T_2ALL:
            handler_2all_receive(&message);
            break;
        case T_2ONE:
            handler_2one_receive(&message);
            break;
        case T_SERVER_SHUTDOWN:
            handler_server_shutdown();
            break;
        default:
            printf("Client received invalid message type: %d\n", type);
            exit(EXIT_FAILURE);
    }
    enable_queue();
    signal(SIGUSR1, catcher);
}

void init()
{
    atexit(clean);
    signal(SIGINT, handler_sigint);
    signal(SIGUSR1, catcher);

    if((server_qdesc = get_queue(SERVER_NAME)) == -1)
    {
        perror("Unable to connect to server queue");
        exit(EXIT_FAILURE);
    }

    char client_qname[CLIENT_NAME_LEN];
    strcpy(client_qname, CLIENT_NAME);
    
    if((client_qdesc = create_queue(client_qname)) == -1)
    {
        fprintf(stderr, "qname: %s\n", client_qname);
        perror("Unable to establish client queue");
        exit(EXIT_FAILURE);
    }

    msg_t msg;
    msg.type = T_INIT;
    sprintf(msg.text, "%s", client_qname);
    msg.pid = getpid();
    send_to_server(&msg);

//    printf("List msg params:\n");
//    printf("type: %ld\n", msg.type);
//    printf("pid: %d\n", msg.pid);
//    printf("qname: %s\n", msg.text);

    receive_message(client_qdesc, &msg, NULL);
    enable_queue();

    id = msg.id;
    printf("Client initialized: id -> %d\n", id);
}

int main(int argc, char *argv[])
{
    init();
    while(true)
    {
        sender();
    }
    return 0;
}