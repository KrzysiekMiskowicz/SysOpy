#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"
#include <dirent.h>
#include "sys/stat.h"
#include "stdbool.h"

#define SLASH "/"


bool is_name_valid(const char* filename)
{
    if (strlen(filename) >= 4 && strcmp(filename + strlen(filename) - 4, ".txt") == 0)
        return true;
    else
        return false;
}

bool does_file_contain_pattern(const char *fname, const char *pattern)
{
    FILE *f = fopen(fname, "r");
    const size_t buf_size = 256;
    size_t n_read = 0;
    char buf[buf_size];
    if(!f)
    {
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }
    while(true)
    {
        n_read = fread(buf, sizeof(char), buf_size, f);
        if(n_read == 0)
        {
            fclose(f);
            return false;
        }
        if (strstr(buf, pattern) != NULL)
        {
            fclose(f);
            return true;
        }
    }
}

void add_to_path(char *path, const char *file)
{
    strcat(path, SLASH);
    strcat(path, file);
}

void read_dir(const char *path, const char *rel_path, const char *pattern, int maxdepth)
{
    if(!path || maxdepth <= 0)
        return;
    DIR *dir = opendir(path);
    if(!dir)
    {
        perror("Cannot open directory");
        exit(EXIT_FAILURE);
    }
    char new_path[256] = "";
    char new_rel_path[256] = "";
    struct dirent *object;
    while((object = readdir(dir)) != NULL)
    {
//        snprintf(new_path, "%s%s%s", path, SLASH, object->d_name);
        strcpy(new_path, path);
        add_to_path(new_path, object->d_name);

        strcpy(new_rel_path, rel_path);
        add_to_path(new_rel_path, object->d_name);

        struct stat s_object;
        if (lstat(new_path, &s_object) < 0)
        {
            perror("Problems meet during executing lstat function");
            exit(EXIT_FAILURE);
        }
        if(S_ISDIR(s_object.st_mode) && strcmp(object->d_name, ".") != 0 && strcmp(object->d_name, "..") != 0)
        {
//            printf("+++\n");
            pid_t child_pid = fork();
            if(child_pid == 0) {
                const int args_nr = 5;
                const int arr_size = 256;
                char args_arr[args_nr][arr_size];
                snprintf(args_arr[0], arr_size, "%d", args_nr);
                snprintf(args_arr[1], arr_size, "%s", new_path);
                snprintf(args_arr[2], arr_size, "%s", pattern);
                snprintf(args_arr[3], arr_size, "%d", maxdepth - 1);
                snprintf(args_arr[4], arr_size, "%s", new_rel_path);

                char *args[args_nr + 1];
                for (int arg_ctr = 0; arg_ctr < args_nr; arg_ctr++)
                    args[arg_ctr] = args_arr[arg_ctr];
                args[args_nr] = NULL;
                execvp("./main", args);
            }
        }
        else if(S_ISREG(s_object.st_mode) && is_name_valid(object->d_name) && does_file_contain_pattern(new_path, pattern))
        {
            printf("Process -> %d; file -> %s\n", (int)getpid(), new_rel_path);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[])
{
    if(argc != 4 && argc != 5)
    {
        perror("Invalid number of arguments given for main program");
        exit(EXIT_FAILURE);
    }
    const int arr_size = 256;
    char path[arr_size];
    char pattern[arr_size];
    strcpy(path, argv[1]);
    strcpy(pattern, argv[2]);
    int maxdepth = atoi(argv[3]);
    char rel_path[arr_size];
    if(argc == 4)
        strcpy(rel_path, ".");
    else
        strcpy(rel_path, argv[4]);
    read_dir(path, rel_path, pattern, maxdepth);
    return 0;
}
