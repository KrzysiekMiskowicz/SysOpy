#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "utils.h"

void get_cmd(char *line, char *command, char *text)
{
    char line_cpy[256];
    strcpy(line_cpy, line);

    char *line_tmp = line_cpy;
    char *cmd = strtok_r(line_cpy, " \n", &line_tmp);

    char *text_tmp = strtok_r(NULL, "\n", &line_tmp);
    if (text_tmp == NULL)
    {
        text[0] = '\0';
    }
    else
    {
        strcpy(text, text_tmp);
    }
    if (cmd == NULL)
    {
        command[0] = '\0';
    }
    else
    {
        strcpy(command, cmd);
    }
}

key_t get_server_key()
{
    key_t key;
    if ((key = ftok(getenv("HOME"), 1)) == -1)
    {
        perror("Unable to generate server key");
        exit(EXIT_FAILURE);
    }
    return key;
}

key_t get_client_key()
{
    key_t key;
    if ((key = ftok(getenv("HOME"), getpid())) == -1)
    {
        perror("Unable to generate client key");
        exit(EXIT_FAILURE);
    }
    return key;
}

int send(int qid, msg_t *msg)
{
    return msgsnd(qid, msg, MAX_MESSAGE_SIZE, 0);
}

int receive(int qid, msg_t *msg)
{
    return msgrcv(qid, msg, MAX_MESSAGE_SIZE, -100, 0);
}

int receive_no_wait(int qid, msg_t *msg)
{
    return msgrcv(qid, msg, MAX_MESSAGE_SIZE, -100, IPC_NOWAIT);
}

int create_queue(int key)
{
    return msgget(key, IPC_CREAT | IPC_EXCL | 0777);
}

int delete_queue(int qid, int key)
{
    return msgctl(qid, IPC_RMID, NULL);
}

int get_queue(int key)
{
    return msgget(key, 0);
}

int close_queue(int qid)
{
    return 0;
}

int is_empty(int qid)
{
    struct msqid_ds buf;
    msgctl(qid, IPC_STAT, &buf);

    return buf.msg_qnum == 0;
}