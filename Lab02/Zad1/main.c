#include "stdlib.h"
#include "stdbool.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "time.h"

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

void lib_write_wrapper(char *buffer, size_t size, FILE *fp)
{
    if(size != fwrite(buffer, sizeof(char), size, fp))
    {
        perror("Problems meet during writing to file");
        exit(EXIT_FAILURE);
    }
}

void lib_write_from_buffers(char *buffer, char *backup, size_t size, FILE *dst_f, bool from_backup, size_t buffer_pos, bool is_first_line)
{
    size_t backup_size = 0;
    if(!is_first_line)
        lib_write_wrapper("\n", strlen("\n"), dst_f);
    if(from_backup)
    {
        backup_size = size - buffer_pos - 1; // -1
        lib_write_wrapper(backup, backup_size, dst_f);
    }
    size_t buffer_size = size - backup_size - 1;
    int start_buffer_pos = (int)buffer_pos - buffer_size;
    lib_write_wrapper(buffer + start_buffer_pos, buffer_size, dst_f);
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

void sys_write_wrapper(int fd, char *buffer, size_t size)
{
    if(write(fd, buffer, size) != size)
    {
        perror("Problems meet during writing to file");
        exit(EXIT_FAILURE);
    }
}

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

void sys_write_from_buffers(char *buffer, char *backup, size_t size, int fd, bool from_backup, size_t buffer_pos, bool is_first_line)
{
    size_t backup_size = 0;
    if(!is_first_line)
        sys_write_wrapper(fd, "\n", strlen("\n"));
    if(from_backup)
    {
        backup_size = size - buffer_pos - 1;
        sys_write_wrapper(fd, backup, backup_size);
    }
    size_t buffer_size = size - backup_size - 1;
    int start_buffer_pos = (int)buffer_pos - buffer_size;
    sys_write_wrapper(fd, buffer + start_buffer_pos, buffer_size);
}

int sys_open_wrapper(char *fname, bool for_read)
{
    int fd;
    if(for_read)
        fd = open(fname, O_RDONLY | O_CREAT);
    else
        fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC);

    if(fd == -1)
    {
        perror("Cannot read file:");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void copy(char *src_name, char *dst_name, bool lib)
{
    FILE *src_f = NULL;
    FILE *dst_f = NULL;
    int src_fd = 0;
    int dst_fd = 0;
    if(lib)
    {
        src_f = lib_open_wrapper(src_name, "r");
        dst_f = lib_open_wrapper(dst_name, "w");
    }
    else
    {
        src_fd = sys_open_wrapper(src_name, true);
        dst_fd = sys_open_wrapper(dst_name, false);
    }
    const size_t buffer_size = LINE_MAX_LEN;
    char *buffer = calloc(sizeof(char), buffer_size);
    char *backup = calloc(sizeof(char), buffer_size);
    char *line_beginning = NULL;
    size_t line_size = 0;
    bool all_white_spaces = true;
    bool load_from_backup = false;
    bool is_first_line = true;
    bool continue_coping = false;
    int loaded_size = 0;
    if(lib)
    {
        loaded_size = lib_read_wrapper(buffer, buffer_size, src_f);
        continue_coping = (loaded_size == buffer_size || feof(src_f));
    }
    else
    {
        loaded_size = sys_read_wrapper(src_fd, buffer, buffer_size);
        continue_coping = (loaded_size > 0);
    }
    while(continue_coping)
    {
        for(size_t char_ctr = 0; char_ctr < loaded_size; char_ctr++)
        {
            line_size++;
            if(line_size == 1)
            {
                line_beginning = buffer + char_ctr;
            }
            if(buffer[char_ctr] == NEW_LINE)
            {
                if(!all_white_spaces)
                {
                    if(lib)
                        lib_write_from_buffers(buffer, backup, line_size, dst_f, load_from_backup, char_ctr, is_first_line);
                    else
                        sys_write_from_buffers(buffer, backup, line_size, dst_fd, load_from_backup, char_ctr, is_first_line);
                }
                is_first_line = false;
                line_size = 0;
                all_white_spaces = true;
                load_from_backup = false;
            }
            else
            {
                all_white_spaces &= isspace((int)buffer[char_ctr]);
            }
        }
        if(lib && feof(src_f))
        {
            if(!all_white_spaces)
                lib_write_from_buffers(buffer, backup, line_size+1, dst_f, load_from_backup, loaded_size, is_first_line);
            break;
        }
        else if(!lib && loaded_size < buffer_size)
        {
            if(!all_white_spaces)
                sys_write_from_buffers(buffer, backup, line_size+1, dst_fd, load_from_backup, loaded_size, is_first_line);
            break;
        }
        memcpy(backup, line_beginning, line_size);
        line_beginning = backup;
        load_from_backup = true;
        if(lib)
        {
            loaded_size = lib_read_wrapper(buffer, buffer_size, src_f);
            continue_coping = (loaded_size == buffer_size || feof(src_f));
        }
        else
        {
            loaded_size = sys_read_wrapper(src_fd, buffer, buffer_size);
            //continue_coping = (loaded_size == buffer_size || loaded_size == 0);
            continue_coping = (loaded_size > 0);
        }
    }
    if(lib)
    {
        fclose(src_f);
        fclose(dst_f);
    }
    else
    {
        close(src_fd);
        close(dst_fd);
    }
    free(backup);
    free(buffer);
}

void lib_copy(char *src_name, char *dst_name)
{
    copy(src_name, dst_name, true);
}

void sys_copy(char *src_name, char *dst_name)
{
    copy(src_name, dst_name, false);
}

void run_program(char *src_name, char *dst_name)
{
    char report_name[] = "pomiar_zad_1.txt";
    FILE* freport = fopen(report_name, "w");

    void (*fptr)(char*, char*) = &lib_copy;
    clock_t start = clock();
    fptr(src_name, dst_name);
    clock_t end = clock();
    float sec = (end - start) / CLOCKS_PER_SEC;
    fprintf(freport, "Library functions execution time: %fs\n", sec);

    fptr = &sys_copy;
    start = clock();
    fptr(src_name, dst_name);
    end = clock();
    sec = (end - start) / CLOCKS_PER_SEC;

    fprintf(freport, "System functions execution time: %fs\n", sec);
    fclose(freport);
}

int main(int argc, char* argv[])
{
    char src_name[256] = "";
    char dst_name[256] = "";
    if(argc == 3)
    {
        strcpy(src_name, argv[1]);
        strcpy(dst_name, argv[2]);
    }
    else if(argc == 1)
    {
        printf("Enter source file path: ");
        scanf("%s", src_name);
        printf("Enter destination file path: ");
        scanf("%s", dst_name);
    }
    else
    {
        fprintf(stderr, "Invalid number of arguments given to program\n");
        exit(EXIT_FAILURE);
    }
    run_program(src_name, dst_name);
    return 0;
}
