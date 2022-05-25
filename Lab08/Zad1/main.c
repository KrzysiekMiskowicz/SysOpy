#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#define MAX_PIXEL_VAL 255
#define MAX_LINE_LEN 3000
#define WHITE_SIGNS " \t\r\n"
#define REPORT_FILE "Times.txt"

int W, H;
int **image;
int **negative;

int threads_nr;
typedef struct arg_struct
{
    int id;
    char *mode;
} arg_struct;

int count_negative(int p)
{
    return MAX_PIXEL_VAL - p;
}

void read_W_and_H(char *line)
{
    char *backup = line;
    W = atoi(strtok_r(backup, WHITE_SIGNS, &backup));
    H = atoi(strtok_r(backup, WHITE_SIGNS, &backup));
}

void load_row(char *line, int r)
{
    char *backup = line;
    for(int c = 0; c < W; c++)
    {
        image[r][c] = atoi(strtok_r(backup, WHITE_SIGNS, &backup));
    }
}

void load_image(char *fname)
{
    char COMMENT_CHAR = '#';
    FILE *f = fopen(fname, "r");
    if (f == NULL)
    {
        fprintf(stderr, "Unable to open file\n");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LEN + 1];
    fgets(line, MAX_LINE_LEN, f);

    do
    {
        fgets(line, MAX_LINE_LEN, f);
    } while(line[0] == COMMENT_CHAR);

    read_W_and_H(line);
    fgets(line, MAX_LINE_LEN, f);

    image = calloc(H, sizeof(int *));
    for(int r = 0; r < H; r++)
        image[r] = calloc(W, sizeof(int));

    for(int r = 0; r < H; r++)
    {
        fgets(line, MAX_LINE_LEN, f);
        load_row(line, r);
    }
    fclose(f);
}

void* calculate_time(struct timespec st, struct timespec end)
{
    double *time = malloc(sizeof(double));
    *time = (end.tv_sec - st.tv_sec) * 1000000 + (end.tv_nsec - st.tv_nsec) / 1000.0;
    return (void *)time;
}

void *value_method(int id)
{
    struct timespec st, end;
    clock_gettime(CLOCK_REALTIME, &st);
    int range = MAX_PIXEL_VAL / threads_nr;
    for(int r = 0; r < H; r++)
    {
        for (int c = 0; c < W; c++)
        {
            if (image[r][c] / range == id)
            {
                negative[r][c] = count_negative(image[r][c]);
            }
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);
    return calculate_time(st, end);
}

void *block_method(int id)
{
    struct timespec st, end;
    clock_gettime(CLOCK_REALTIME, &st);
    int range = W / threads_nr;
    for(int c = id * range; c < (id + 1) * range; c++)
    {
        for (int r = 0; r < H; r++)
        {
            negative[r][c] = count_negative(image[r][c]);
        }
    }

    clock_gettime(CLOCK_REALTIME, &end);
    return calculate_time(st, end);
}

void *calculate_negative(void *arg)
{
    arg_struct *converted_arg = arg;
    if (strcmp(converted_arg->mode, "value") == 0)
    {
        return value_method(converted_arg->id);
    }
    else if (strcmp(converted_arg->mode, "block") == 0)
    {
        return block_method(converted_arg->id);
    }
    else
    {
        printf("Invalid mode\n");
        exit(EXIT_FAILURE);
    }
}

void save_negative(char *fname)
{
    FILE *f = fopen(fname, "w");
    fprintf(f, "P2\n");
    fprintf(f, "# negative.pgm\n");
    fprintf(f, "%d %d\n", W, H);
    fprintf(f, "%d\n", MAX_PIXEL_VAL);
    for (int r = 0; r < H; r++)
    {
        for (int c = 0; c < W; c++)
        {
            fprintf(f, "%d ", negative[r][c]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void clean()
{
    for(int r = 0; r < H; r++)
    {
        free(image[r]);
        free(negative[r]);
    }
    free(image);
    free(negative);
}

int main(int argc, char *argv[])
{
    atexit(clean);
    threads_nr = atoi(argv[1]);
    char *mode = argv[2];
    char *input_file = argv[3];
    char *output_file = argv[4];

    load_image(input_file);

    FILE *f_report = fopen(REPORT_FILE, "a");
    fprintf(f_report, "Mode: %s | threads: %d\n", mode, threads_nr);

    negative = calloc(H, sizeof(int *));
    for (int r = 0; r < H; r++)
        negative[r] = calloc(W, sizeof(int));

    struct timespec st, end;
    clock_gettime(CLOCK_REALTIME, &st);

    pthread_t *thread_ids = calloc(threads_nr, sizeof(pthread_t));
    struct arg_struct *args = calloc(threads_nr, sizeof(struct arg_struct));
    for(int thread_id = 0; thread_id < threads_nr; thread_id++)
    {
        arg_struct arg;
        arg.id = thread_id;
        arg.mode = mode;

        args[thread_id] = arg;
        pthread_create(&thread_ids[thread_id], NULL, calculate_negative, (void *)&args[thread_id]);
    }

    for (int thread_id = 0; thread_id < threads_nr; thread_id++)
    {
        double *time;
        pthread_join(thread_ids[thread_id], (void *)&time);
        fprintf(f_report, "Thread id: %2d. Time elapsed: %.3lf us\n", thread_id, *time);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    double *time = (double *)calculate_time(st, end);

    fprintf(f_report, "Total time elapsed: %f\n\n", *time);
    fclose(f_report);
    save_negative(output_file);

    return EXIT_SUCCESS;
}