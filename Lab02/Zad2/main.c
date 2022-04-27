#include "stdlib.h"
#include "stdbool.h"
#include "stdio.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "time.h"
#include "string.h"

#define NEW_LINE '\n'
#define LINE_MAX_LEN 256

// lib functions

int lib_read_wrapper(char *buffer, size_t size, FILE *fp)
{
    int loaded_size = fread(buffer, sizeof(char), size, fp);
    if(size != loaded_size && !feof(fp))
    {
        perror("Problems meet during reading file");
        exit(EXIT_FAILURE);
    }
    return loaded_size;
}

FILE* lib_open_wrapper(char *fname, char *mode)
{
    FILE *fp = fopen(fname, mode);
    if(fp == NULL)
    {
        perror("Cannot read file:");
        exit(EXIT_FAILURE);
    }
    return fp;
}

// sys functions

int sys_read_wrapper(int fd, char *buffer, size_t size)
{
    int loaded_size = read(fd, buffer, size);
    if(loaded_size == -1)
    {
        perror("Problems meet during reading file");
        exit(EXIT_FAILURE);
    }
    return loaded_size;
}

int sys_open_wrapper(char *fname)
{
    int fd = open(fname, O_RDONLY | O_CREAT);;
    if(fd == -1)
    {
        perror("Cannot read file:");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void search(char token, char *fname, bool lib)
{
    FILE *fp = NULL;
    int fd = 0;
    if(lib)
        fp = fopen(fname, "r");
    else
        fd = sys_open_wrapper(fname);
    const size_t buffer_size = LINE_MAX_LEN;
    char *buffer = calloc(sizeof(char), buffer_size);
    unsigned long token_ctr = 0;
    unsigned long lines_with_token_ctr = 0;
    bool is_line_with_token = false;
    bool continue_coping = false;
    int loaded_size = 0;
    if(lib)
    {
        loaded_size = lib_read_wrapper(buffer, buffer_size, fp);
        continue_coping = (loaded_size == buffer_size || feof(fp));
    }
    else
    {
        loaded_size = sys_read_wrapper(fd, buffer, buffer_size);
        continue_coping = (loaded_size > 0);
    }
    while(continue_coping)
    {
        for(size_t char_ctr = 0; char_ctr < loaded_size; char_ctr++)
        {
            if(buffer[char_ctr] == NEW_LINE)
            {
                if(is_line_with_token)
                {
                    lines_with_token_ctr++;
                }
                is_line_with_token = false;
            }
            else if(buffer[char_ctr] == token)
            {
                is_line_with_token = true;
                token_ctr++;
            }
        }
        if((lib && feof(fp)) || (!lib && loaded_size < buffer_size))
        {
            break;
        }
        if(lib)
        {
            loaded_size = lib_read_wrapper(buffer, buffer_size, fp);
            continue_coping = (loaded_size == buffer_size || feof(fp));
        }
        else
        {
            loaded_size = sys_read_wrapper(fd, buffer, buffer_size);
            continue_coping = (loaded_size > 0);
        }
    }
    if(lib)
        fclose(fp);
    else
        close(fd);
    free(buffer);
    printf("Occurences of %c: %lu\nLines with %c: %lu\n", token, token_ctr, token, lines_with_token_ctr);
}

void lib_search(char token, char *fname)
{
    search(token, fname, true);
}

void sys_search(char token, char *fname)
{
    search(token, fname, false);
}

void run_program(char token, char *src_file)
{
    char report_name[] = "pomiar_zad_2.txt";
    FILE* freport = fopen(report_name, "w");
    printf("Library functions execution\n");
    void (*fptr)(char, char*) = &lib_search;
    clock_t start = clock();
    fptr(token, src_file);
    clock_t end = clock();
    float sec = (end - start) / CLOCKS_PER_SEC;
    fprintf(freport, "Library functions execution time: %fs\n", sec);

    printf("System functions execution\n");
    fptr = &sys_search;
    start = clock();
    fptr(token, src_file);
    end = clock();
    sec = (end - start) / CLOCKS_PER_SEC;

    fprintf(freport, "System functions execution time: %fs\n", sec);
    fclose(freport);
}

int main(int argc, char *argv[])
{
    char token;
    char src_name[256] = "";
    if(argc == 3)
    {
        token = *argv[1];
        strcpy(src_name, argv[2]);
    }
    else if(argc == 1)
    {
        printf("Enter searched character: ");
        scanf("%c", &token);
        printf("Enter source file path: ");
        scanf("%s", src_name);
    }
    else
    {
        fprintf(stderr, "Invalid number of arguments given to program\n");
        exit(EXIT_FAILURE);
    }
    run_program(token, src_name);
    return 0;
}
