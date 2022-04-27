#ifdef DYNAMIC_LINKING
    #include "dynamic_library.h"
#else
    #include "library.h"
#endif

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <sys/times.h>
#include <unistd.h>

struct tms s_init_timer, s_end_timer;
clock_t start_time, end_time;
FILE *report_file;

void write_header_to_report(FILE *report)
{
    fprintf(report, "%40s\t %20s \t %20s \t %20s", "Operation", "Real Time [s]", "User Time [s]", "System Time [s]\n");
    printf("%40s\t %20s \t %20s \t %20s", "Operation", "Real Time [s]", "User Time [s]", "System Time [s]\n");
}

void write_data_to_report(FILE *report, char *operation, double real_time, double user_time, double cpu_time)
{
    fprintf(report, "%40s\t %20f \t %20f \t %20f\n", operation, real_time, user_time, cpu_time);
    printf("%40s\t %20f \t %20f \t %20f\n", operation, real_time, user_time, cpu_time);
}

void start_timer()
{
    start_time = times(&s_init_timer);
}

void stop_timer()
{
    end_time = times(&s_end_timer);
}

void save_timer(char *operation, FILE *report)
{
    int tics = (int)sysconf(_SC_CLK_TCK);
    double real_time = (double)(end_time - start_time) / tics;
    double user_time = (double)(s_end_timer.tms_utime - s_init_timer.tms_utime) / tics;
    double cpu_time = (double)(s_end_timer.tms_stime - s_init_timer.tms_stime) / tics;
    write_data_to_report(report, operation, real_time, user_time, cpu_time);
}

int parser_stop_timer(char *argv[], int argc, int *ctr)
{
    stop_timer();
    end_time = times(&s_end_timer);
    if(*ctr + 1 >= argc){
        fprintf(stderr, "Invalid number of timer arguments\n");
        return -1;
    }
    *ctr += 1;
    char *operation = argv[*ctr];
    save_timer(operation, report_file);
    *ctr += 1;
    return 0;
}

int parser_create_table(char *argv[], int argc, int *ctr)
{
    if(*ctr + 1 >= argc){
        fprintf(stderr, "Invalid number of arguments for create_table function\n");
        return -1;
    }
    int size = atoi(argv[*ctr + 1]);
    *ctr += 2;
    return create_table(size);
}

int parser_remove_block(char *argv[], int argc, int *ctr)
{
    if(*ctr + 1 >= argc){
        fprintf(stderr, "Invalid number of arguments for remove_block function\n");
        return -1;
    }
    unsigned int index = atoi(argv[*ctr + 1]);
    *ctr += 2;
    return remove_block(index);
}

int parser_wc_files(char *argv[], int argc, int *ctr)
{
    if(*ctr + 1 >= argc || *ctr + 1 + atoi(argv[*ctr + 1]) >= argc){
        fprintf(stderr, "Invalid number of arguments for wc_files function\n");
        return -1;
    }
    *ctr += 2;
    int last_file_index = *ctr + atoi(argv[*ctr - 1]) - 1;
    char files[100] = {'\0'};
    while(*ctr <= last_file_index)
    {
        strcat(files, argv[*ctr]);
        strcat(files, " ");
        *ctr += 1;
    }
    return wc_files(files);
}

int main(int argc, char *argv[])
{
    #ifdef DYNAMIC_LINKING
        init_dynamic_library();
    #endif
    if(argc < 2){
        printf("Invalid numer of arguments delivered to main function\n");
        return -1;
    }
//    char report_name[] = "test_raport.txt";
    char report_name[50] = {"\0"};
    strcpy(report_name, argv[1]);
    strcat(report_name, "/raport.txt");
    report_file = fopen(report_name, "a");
    printf("%s\n", report_name);
    write_header_to_report(report_file);
    int ctr = 1;
    int status = 0;
    while(ctr < argc)
    {
        if(!strcmp(argv[ctr], "start"))
        {
            start_timer();
            ctr++;
        }
        else if(!strcmp(argv[ctr], "stop"))
        {
            status = parser_stop_timer(argv, argc, &ctr);
            if(status != 0){
                fprintf(stderr, "Program aborted due to errors in timer\n");
                break;
            }
        }
        else if(!strcmp(argv[ctr], "create_table"))
        {
            status = parser_create_table(argv, argc, &ctr);
            if(status != 0){
                fprintf(stderr, "Program aborted due to errors in create_table function\n");
                break;
            }
        }
        else if(!strcmp(argv[ctr], "remove_block"))
        {
            status = parser_remove_block(argv, argc, &ctr);
            if(status != 0){
                fprintf(stderr, "Program aborted due to errors in remove_block function\n");
                break;
            }
        }
        else if(!strcmp(argv[ctr], "wc_files"))
        {
            status = parser_wc_files(argv, argc, &ctr);
            if(status != 0){
                fprintf(stderr, "Program aborted due to errors in wc_files function\n");
                break;
            }
        }
        else
        {
            ctr++;
        }
    }
    delete_table();
    fclose(report_file);
    if(status != 0){
        exit(-1);
    }
    printf("Program executed succesfully\n");
    return 0;
}
