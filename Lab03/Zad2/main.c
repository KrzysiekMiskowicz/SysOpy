#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"
#include "time.h"
#include <sys/times.h>

struct tms s_init_timer, s_end_timer;
clock_t start_time, end_time;

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        perror("Invalid number of arguments given for main program");
        exit(EXIT_FAILURE);
    }
    int n = atoi(argv[1]);
    double dx = 1.0 / n;
    pid_t child_pid;

    start_time = times(&s_init_timer);

    for(int i = 0; i < n; i++)
    {
        child_pid = fork();
        if(child_pid == 0)
        {
            const int args_nr = 4;
            char args_arr[args_nr][256];
            snprintf(args_arr[0], 256, "%d", 4);
            snprintf(args_arr[1], 256, "%f", i*dx);
            snprintf(args_arr[2], 256, "%f", dx);
            snprintf(args_arr[3], 256, "%d", i+1);

            char *args[args_nr+1];
            for(int arg_ctr = 0; arg_ctr < args_nr; arg_ctr++)
                args[arg_ctr] = args_arr[arg_ctr];
            args[args_nr] = NULL;

            execvp("./child", args);
            break;
        }
    }

    for(int i = 0; i < n; i++)
        wait(NULL);

    double sum = 0;
    char fname[256];
    char buf[256];
    FILE *f;
    for(int i = 1; i <= n; i++)
    {
        snprintf(fname, 256, "w%d.txt", i);
        f = fopen(fname, "r");
        fread(buf, sizeof(double), 1, f);
        sum += strtod(buf, NULL);
        fclose(f);
    }

    end_time = times(&s_end_timer);
    double real_time = (double)(end_time - start_time) / (int)sysconf(_SC_CLK_TCK);

    snprintf(fname, 256, "raport.txt");
    f = fopen(fname, "a");
    fprintf(f, "Czas wykonania programu dla n = %d wyniósł: %fs\n", n, real_time);
    fclose(f);
    printf("Sum -> %f\n", sum);
    return 0;
}
