#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <sys/types.h>
#include <unistd.h>

void count(double (*f_ptr)(double), double x, double dx, int process_nr)
{
    char fname[256];
    snprintf(fname, 256, "w%d.txt", process_nr);
    FILE* file = fopen(fname, "w");
    if(!file)
    {
        perror("Cannot open file\n");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%f", f_ptr(x) * dx);
    fclose(file);
}

double f(double x)
{
    return 4 / (x*x + 1);
}

int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        perror("Invalid number of arguments for exec.c\n");
        exit(EXIT_FAILURE);
    }
    double (*f_ptr)(double) = f;
    double x = strtod(argv[1], NULL);
    double dx = strtod(argv[2], NULL);
    int process_nr = atoi(argv[3]);
    count(f_ptr, x, dx, process_nr);
    return 0;
}
