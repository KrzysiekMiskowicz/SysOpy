#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include <dirent.h>
#include <ftw.h>
#include "time.h"

#define SLASH "/"

unsigned int file_ctr = 0;
unsigned int dir_ctr = 0;
unsigned int chr_ctr = 0;
unsigned int block_ctr = 0;
unsigned int fifo_ctr = 0;
unsigned int link_ctr = 0;
unsigned int socket_ctr = 0;

char* get_dir_object_type_name(int st_mode)
{
    if(S_ISREG(st_mode))
        return("file");
    else if(S_ISDIR(st_mode))
        return("dir");
    else if(S_ISCHR(st_mode))
        return("char dev");
    else if(S_ISBLK(st_mode))
        return("block dev");
    else if(S_ISFIFO(st_mode))
        return("fifo");
    else if(S_ISLNK(st_mode))
        return("slink");
    else if(S_ISSOCK(st_mode))
        return("sock");
    else
    {
        fprintf(stderr, "Invalid file type\n");
        exit(EXIT_FAILURE);
    }
}

void update_files_ctr(int st_mode, unsigned int *file, unsigned int *dir, unsigned int *chr, unsigned int *block,
                      unsigned int *fifo, unsigned int *link, unsigned int *socket)
{
    if(strcmp(get_dir_object_type_name(st_mode), "file") == 0)
        *file += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "dir") == 0)
        *dir += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "char dev") == 0)
        *chr += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "block dev") == 0)
        *block += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "fifo") == 0)
        *fifo += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "slink") == 0)
        *link += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "sock") == 0)
        *socket += 1;
}

void nfwt_update_files(int st_mode)
{
    if(strcmp(get_dir_object_type_name(st_mode), "file") == 0)
        file_ctr += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "dir") == 0)
        dir_ctr += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "char dev") == 0)
        chr_ctr += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "block dev") == 0)
        block_ctr += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "fifo") == 0)
        fifo_ctr += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "slink") == 0)
        link_ctr += 1;
    else if(strcmp(get_dir_object_type_name(st_mode), "sock") == 0)
        socket_ctr += 1;
}

void display_dir_object_info(const char *path, const struct stat *pstat)
{
    char *object_type = get_dir_object_type_name(pstat->st_mode);
    printf("%s -> links: %u; type: %s; size: %lldB; last access: %s; last modification: %s",
           path, pstat->st_nlink, object_type, pstat->st_size, ctime(&pstat->st_atime), ctime(&pstat->st_mtime));
}

void read_dir(char *path, unsigned int *file_ctr, unsigned int *dir_ctr, unsigned int *chr_ctr, unsigned int *block_ctr,
              unsigned int *fifo_ctr, unsigned int *link_ctr, unsigned int *socket_ctr)
{
    if(path == NULL)
        return;
    DIR *dir = opendir(path);
    if(dir == NULL)
    {
        perror("Cannot open directory");
        exit(EXIT_FAILURE);
    }
    char new_path[256] = "";
    struct dirent *object;
    while((object = readdir(dir)) != NULL)
    {
        strcpy(new_path, path);
        strcat(new_path, SLASH);
        strcat(new_path, object->d_name);

        struct stat s_object;
        if (lstat(new_path, &s_object) < 0)
        {
            perror("Problems meet during executing lstat function");
            exit(EXIT_FAILURE);
        }
        if(S_ISDIR(s_object.st_mode) && strcmp(object->d_name, ".") != 0 && strcmp(object->d_name, "..") != 0)
        {
            read_dir(new_path, file_ctr, dir_ctr, chr_ctr, block_ctr, fifo_ctr, link_ctr, socket_ctr);
        }
        display_dir_object_info(new_path, &s_object);
        update_files_ctr(s_object.st_mode, file_ctr, dir_ctr, chr_ctr, block_ctr, fifo_ctr, link_ctr, socket_ctr);
    }
    closedir(dir);
}

void stat_main(char *path)
{
    unsigned int file_ctr = 0;
    unsigned int dir_ctr = 0;
    unsigned int chr_ctr = 0;
    unsigned int block_ctr = 0;
    unsigned int fifo_ctr = 0;
    unsigned int link_ctr = 0;
    unsigned int socket_ctr = 0;
    read_dir(path, &file_ctr, &dir_ctr, &chr_ctr, &block_ctr, &fifo_ctr, &link_ctr, &socket_ctr);
    printf("Number of file types:\nfile: %u\ndir: %u\nchar dev: %u\nblock dev: %u\nfifo: %u\nlink: %u\nsocket: %u\n",
           file_ctr, dir_ctr, chr_ctr, block_ctr, fifo_ctr, link_ctr, socket_ctr);
}

int get_info(const char *path, const struct stat *statptr, int flags, struct FTW *pfwt)
{
    display_dir_object_info(path, statptr);
    nfwt_update_files(statptr->st_mode);
    return 0;
}

void nftw_main(char *path)
{
    int flags = FTW_PHYS;
    int fd_limit = 2;
    nftw(path, get_info, fd_limit, flags);
    printf("Number of file types:\nfile: %u\ndir: %u\nchar dev: %u\nblock dev: %u\nfifo: %u\nlink: %u\nsocket: %u\n",
           file_ctr, dir_ctr, chr_ctr, block_ctr, fifo_ctr, link_ctr, socket_ctr);
}

void run_program(char *dir_path)
{
    char report_name[] = "pomiar_zad_3.txt";
    FILE* freport = fopen(report_name, "w");

    printf("Stat functions execution\n");
    void (*fptr)(char*) = &stat_main;
    clock_t start = clock();
    fptr(dir_path);
    clock_t end = clock();
    float sec = (end - start) / CLOCKS_PER_SEC;
    fprintf(freport, "Stat functions execution time: %fs\n", sec);

    printf("Nftw functions execution\n");
    fptr = &nftw_main;
    start = clock();
    fptr(dir_path);
    end = clock();
    sec = (end - start) / CLOCKS_PER_SEC;

    fprintf(freport, "Nftw function execution time: %fs\n", sec);
    fclose(freport);
}

int main(int argc, char *argv[])
{
    char path[100] = "";
    char absoulte_path[100] = "";
    if(argc == 2)
    {
        strcpy(path, argv[1]);
    }
    else if(argc == 1)
    {
        printf("Enter searched path: ");
        scanf("%s", path);
    }
    else
    {
        fprintf(stderr, "Invalid number of arguments given to program\n");
        exit(EXIT_FAILURE);
    }
    if(path[0] == '/')
        strcpy(absoulte_path, path);
    else
        strcpy(absoulte_path, realpath(path, NULL));
    printf("%s\n", absoulte_path);
    run_program(absoulte_path);
    return 0;
}
