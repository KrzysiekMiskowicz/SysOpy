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

int server_qid, client_qid;
int id;

void send_to_server(msg_t *msg)
{
    msg->timestamp = time(NULL);
    if(send(server_qid, msg) == -1)
    {
        perror("Unable to send message to server");
    }
}
void clean()
{
    msg_t msg;
    msg.type = T_STOP;
    msg.id = id;
    sprintf(msg.text, "");
    printf("Client disconnected!\n");

    send_to_server(&msg);
    close_queue(server_qid);
    delete_queue(client_qid, get_client_key());
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
    sprintf(msg.text, "");
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
        handler_stop();
    }
    else if(strcmp("LIST", command) == 0)
    {
        handler_list();
    }
    else if(strcmp("2ALL", command) == 0)
    {
        handler_2all_send(text);
    }
    else if(strcmp("2ONE", command) == 0)
    {
        handler_2one_send(text);
    }
}

void sender()
{
    char line[MAX_COMMAND_LEN];
    char command[MAX_COMMAND_LEN];
    char text[MAX_COMMAND_LEN];
    printf("Type command to go into read mode\n");
    fgets(line, MAX_COMMAND_LEN, stdin);
    get_cmd(line, command, text);
    sender_handler_cmd(command, text);
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

void catcher()
{
    while (!is_empty(client_qid))
    {
        msg_t message;
        if(receive_no_wait(client_qid, &message) != -1)
        {
            switch(message.type)
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
                    printf("Client received invalid message type: %ld\n", message.type);
                    exit(EXIT_FAILURE);
            }
        }
    }
}

void init()
{
    atexit(clean);
    signal(SOMETHING_HAPPEND, catcher);
    signal(SIGINT, handler_sigint);

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = catcher;
    sigaction(SOMETHING_HAPPEND, &sa, NULL);

    if((server_qid = get_queue(get_server_key())) == -1)
    {
        perror("Unable to connect to server server_qid");
        exit(EXIT_FAILURE);
    }

    key_t private_key = get_client_key();
    if((client_qid = create_queue(private_key)) == -1)
    {
        perror("Unable to establish client server_qid");
        exit(EXIT_FAILURE);
    }

    msg_t msg;
    msg.type = T_INIT;
    sprintf(msg.text, "%d", private_key);
    msg.pid = getpid();
    send_to_server(&msg);

    if(receive(client_qid, &msg) == -1)
    {
        perror("Unable to receive id from server");
        exit(EXIT_FAILURE);
    }

    id = msg.id;
    printf("Client initialized: id -> %d\n", id);
}

int main(int argc, char *argv[])
{
    init();
    while(true)
    {
        sender();
        catcher();
    }
    return 0;
}