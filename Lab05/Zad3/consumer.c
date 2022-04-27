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
#include "stdbool.h"
#include "math.h"

#define MAX_LINE_NR 20
#define MAX_LINE_LENGTH 100

int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        perror("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }
    char *pipe_path = argv[1];
    char *file_path = argv[2];
    int N = atoi(argv[3]);

    FILE *pp = fopen(pipe_path, "r");
    if(pp == NULL)
    {
        fprintf(stderr, "Unable to open pipe: %s\n", pipe_path);
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(file_path, "w");
    if(fp == NULL)
    {
        fprintf(stderr, "Unable to open a file: %s\n", file_path);
        exit(EXIT_FAILURE);
    }

//    char *path = argv[1];
//    FILE *f = fopen(path, "r");
//    int N = 5;

    char lines[MAX_LINE_NR][MAX_LINE_LENGTH];
    for(int i = 0; i < MAX_LINE_NR; i++)
    {
        strcpy(lines[i], "");
    }

    char *buffer = malloc(sizeof(char) * N);
    int active_line = 0;
    int possible_active_line = 0;
    char *buffer_line;
    bool active_msg = true;
    bool force_active = false;
    bool force_inactive = false;
    char delim[] = "|:";
    while(fread(buffer, sizeof(char), N, pp) == N)
    {
        if(*(buffer + N - 1) == ':')
            force_active = true;
        else if(*(buffer + N - 1) == '|')
            force_inactive = true;

        int shift = 0;
        if(*buffer == ':')
        {
            possible_active_line = 0;
            active_msg = true;
            shift = 1;
            buffer_line = strtok(buffer + shift, "|");
        }
        if(*(buffer + shift) == '|')
        {
            shift++;
            possible_active_line = 0;
            active_msg = false;
            buffer_line = strtok(buffer + shift, ":");
        }
        else
            buffer_line = strtok(buffer, delim);
        while(buffer_line != NULL)
        {
            if(active_msg)
            {
                possible_active_line = 0;
                strcat(lines[active_line], buffer_line);
            }
            else
            {
                if(strlen(buffer_line) == 0)
                {
                    possible_active_line = 0;
                }
                else
                {
                    active_line = atoi(buffer_line) + possible_active_line * pow(10, (double)strlen(buffer_line));
                    possible_active_line = active_line;
                }
            }
            active_msg = !active_msg;
            if(*buffer_line == '|')
            {
                possible_active_line = 0;
                active_msg = !active_msg;
                buffer_line = strtok(buffer_line+1, ":");
            }
            else if(*buffer_line == ':')
            {
                possible_active_line = 0;
                active_msg = !active_msg;
                buffer_line = strtok(buffer_line+1, "|");
            }
            else
                buffer_line = strtok(NULL, delim);
        }
        active_msg = !active_msg;
        if(*(buffer + N - 1) == '\0')
            possible_active_line = 0;

        if(force_active)
            active_msg = true;
        else if(force_inactive)
            active_msg = false;
        force_active = false;
        force_inactive = false;
    }

    for(int i = 0; i < MAX_LINE_NR; i++)
    {
        fprintf(fp, "\n");
        fprintf(fp, lines[i], strlen(lines[i]));
    }

    free(buffer);
    fclose(pp);
    fclose(fp);
    printf("Consumer has finished execution!\n");
    return 0;
}
