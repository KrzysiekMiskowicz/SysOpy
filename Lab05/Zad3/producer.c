#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "string.h"
#include <time.h>

int main(int argc, char *argv[])
{
    if(argc != 5)
    {
        perror("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }
    char *pipe_path= argv[1];
    int row_nr = atoi(argv[2]);
    char *file_path = argv[3];
    int N = atoi(argv[4]);
    FILE *fp = fopen(file_path, "r");
    if(fp == NULL)
    {
        fprintf(stderr, "Unable to open a file: %s\n", file_path);
        exit(EXIT_FAILURE);
    }
    FILE *pp = fopen(pipe_path, "w");
    if(pp == NULL)
    {
        fprintf(stderr, "Unable to open pipe: %s\n", pipe_path);
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));

    char *buffer = malloc(sizeof(char) * N);
    size_t msg_overhead = 2;
    if(N >= 10)
        msg_overhead++;
    char *msg = malloc(sizeof(char) * (N + msg_overhead));
    size_t buf_len;
    while((buf_len = fread(buffer, sizeof(char), N, fp)) == N)
    {
        sprintf(msg, "|%d:%s", row_nr, buffer);
        fwrite(msg, sizeof(char), buf_len + msg_overhead, pp);
        sleep(rand() % 2);
    }
    sprintf(msg, "|%d:%s", row_nr, buffer);
    fwrite(msg, sizeof(char), buf_len + msg_overhead, pp);

    free(msg);
    free(buffer);
    fclose(pp);
    fclose(fp);
    printf("Producer has finished execution!\n");
    exit(EXIT_SUCCESS);
}
