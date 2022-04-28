#define _XOPEN_SOURCE 500

#include "utils.h"

#define A 'a'
#define Z 'z'

char *random_string(int length)
{
    static int ctr = 0;
    if(ctr == 0)
    {
        srand(time(NULL));
        ctr++;
    }
    char *str = calloc(length + 1, sizeof(char));
    for (int i = 0; i < length; i++)
    {
        char randomletter = A + (rand() % (Z - A));
        str[i] = randomletter;
    }

    str[length] = '\0';
    return str;
}

char *concat(const char *s1, const char *s2)
{
    int len = strlen(s1) + strlen(s2) + 1;
    char *result = (char *)malloc(len);

    strcpy(result, s1);
    strcat(result, s2);

    return result;
}

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

void delete_queue(char *name) { mq_unlink(name); }
void close_queue(mqd_t descr) { mq_close(descr); }
int create_queue(char *name)
{
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MESSAGE_SIZE - 1;
    attr.mq_curmsgs = 0;

    return mq_open(name, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
}
int get_queue(char *name) { return mq_open(name, O_WRONLY); }

void send_message(mqd_t desc, msg_t *msg, int type) {
//    printf("Send_message params:\n");
//    printf("type: %ld\n", msg->type);
//    printf("pid: %d\n", msg->pid);
//    printf("qname: %s\n", msg->text);
    mq_send(desc, (char *)msg, sizeof(*msg), type);
}

void receive_message(mqd_t desc, msg_t *msg, int *typePointer) {
    mq_receive(desc, (char *)msg, MAX_MESSAGE_SIZE, (unsigned int *) typePointer);
}

void register_notification(mqd_t desc, struct sigevent *s){
    mq_notify(desc, s);
}