#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>

#define T_STOP 1
#define T_LIST 2
#define T_INIT 3
#define T_2ALL 4
#define T_2ONE 5
#define T_SERVER_SHUTDOWN 6

#define MESSAGE_BUFFER_SIZE 1024
#define MAX_MESSAGE_SIZE (sizeof(msg_t) - sizeof(long))
#define ALL_MESSAGE_SIZE sizeof(msg_t)
#define MAX_MESSAGES 10

typedef struct msg_t
{
    long type;
    time_t timestamp;
    int id;
    int to_id;
    char text[MESSAGE_BUFFER_SIZE];
    int pid;
} msg_t;

key_t get_server_key();
key_t get_client_key();

int send(int qid, msg_t *msg);
int receive(int qid, msg_t *msg);
int create_queue(int key);
int delete_queue(int qid, int key);
int get_queue(int key);
int close_queue(int qid);
int is_empty(int qid);
int receive_no_wait(int qid, msg_t *message);

void get_cmd(char *line, char *cmd, char *text);

#endif