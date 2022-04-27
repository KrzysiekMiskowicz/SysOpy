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

#define MAX_COMMANDS 10
#define MAX_ARGS 20

int countChars( char* s, char c )
{
    return *s == '\0'
           ? 0
           : countChars( s + 1, c ) + (*s == c);
}

long get_file_size(FILE *f)
{
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return file_size;
}

void get_command_arguments(char *command, char *args[])
{
    char delim[] = " ";
    char *saveptr = args[0];
    char *arg = strtok_r(command, delim, &saveptr);
    int arg_ctr = 0;
    while(arg != NULL)
    {
        args[arg_ctr++] = arg;
        arg = strtok_r(NULL, delim, &saveptr);
    }
}

void execute_line(char *line)
{
    char *tokens[MAX_COMMANDS][MAX_ARGS];
    for(int i = 0; i < MAX_COMMANDS; i++)
    {
        for(int j = 0; j < MAX_ARGS; j++)
        {
            tokens[i][j] = NULL;
        }
    }

    char delim[] = "|";
    char *saveptr = line;
    char *command = strtok_r(line, delim, &saveptr);
    int command_ctr = 0;
    while(command != NULL)
    {
        get_command_arguments(command, tokens[command_ctr++]);
        command = strtok_r(NULL, delim, &saveptr);
    }

    int pipes[MAX_COMMANDS][2];
    for(int i = 0; i < command_ctr - 1; i++)
    {
        if(pipe(pipes[i]) < 0)
        {
            perror("Unable to create a pipe\n");
            exit(EXIT_FAILURE);
        }
    }

    for(int child_ctr = 0; child_ctr < command_ctr; child_ctr++)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            if(child_ctr > 0)
            {
                dup2(pipes[child_ctr - 1][0], STDIN_FILENO);
            }
            if(child_ctr < command_ctr - 1)
            {
                dup2(pipes[child_ctr][1], STDOUT_FILENO);
            }
            for (int pipe_ctr = 0; pipe_ctr < command_ctr - 1; pipe_ctr++)
            {
                close(pipes[pipe_ctr][0]);
                close(pipes[pipe_ctr][1]);
            }
            execvp(tokens[child_ctr][0], tokens[child_ctr]);
            exit(EXIT_SUCCESS);
        }
    }

    for(int pipe_ctr = 0; pipe_ctr < command_ctr - 1; pipe_ctr++)
    {
        close(pipes[pipe_ctr][0]);
        close(pipes[pipe_ctr][1]);
    }

    for(int child_ctr = 0; child_ctr < command_ctr; child_ctr++)
    {
        wait(0);
    }
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        perror("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }
    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL)
    {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }
    long buffer_size = get_file_size(fp);
    char *buffer = malloc(sizeof(char) * buffer_size);
    if(fread(buffer, sizeof(char), buffer_size, fp) != buffer_size)
    {
        perror("Unable to read from file\n");
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    int lines_nr = countChars(buffer, '\n') + 1;
    char **lines = malloc(sizeof(char*) * lines_nr);

    char *line;
    char delim[] = "\n";
    line = strtok(buffer, delim);
    int line_ctr = 0;
    while(line != NULL)
    {
        lines[line_ctr++] = line;
        line = strtok(NULL, delim);
    }

    for(int i = 0; i < lines_nr; i++)
    {
        execute_line(lines[i]);
    }

    free(lines);
    free(buffer);
    return 0;
}