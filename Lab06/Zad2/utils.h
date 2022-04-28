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
#include "mqueue.h"

#define T_STOP 6
#define T_LIST 5
#define T_INIT 4
#define T_2ALL 3
#define T_2ONE 2
#define T_SERVER_SHUTDOWN 1

#define MAX_MESSAGE_SIZE 300
#define MESSAGE_BUFFER_SIZE 200

#define QUEUE_PREFIX ("/queue_")
#define SERVER_NAME (concat(QUEUE_PREFIX, "server"))
#define CLIENT_NAME concat(QUEUE_PREFIX, concat("client_", random_string(6)))
#define CLIENT_NAME_LEN 30

typedef struct msg_t
{
    long type;
    time_t timestamp;
    int id;
    int to_id;
    int pid;
    char text[MESSAGE_BUFFER_SIZE];
} msg_t;

char *random_string(int length);
char *concat(const char *s1, const char *s2);

void get_cmd(char *line, char *cmd, char *text);
void delete_queue(char *name);
void close_queue(mqd_t descr);
int create_queue(char *name);
int get_queue(char *name);
void send_message(mqd_t desc, msg_t *msg, int type);
void receive_message(mqd_t desc, msg_t *msg, int *typePointer);
void register_notification(mqd_t desc, struct sigevent *s);
int equals(char *str1, char *str2);
char *random_string(int length);
char *concat(const char *s1, const char *s2);

#endif