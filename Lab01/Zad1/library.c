#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool is_initialized = false;
unsigned int table_size = 0;
char **table = NULL;
char *tmp_file = NULL;

int create_table(unsigned int size)
{
    if(is_initialized){
        fprintf(stderr, "Error: Table is already initialized!\n");
        return -1;
    }
    is_initialized = true;
    table_size = size;
    table = calloc(table_size, sizeof(char *));
    tmp_file = "tmp.txt";
    return 0;
}

char* create_wc_command(char *command, char *fname)
{
    strcpy(command, "");
    strcat(command, "wc ");
    strcat(command, fname);
    strcat(command, " > ");
    strcat(command, tmp_file);
    return command;
}

int wc_call(char *fname)
{
    char command[100];
    create_wc_command( command, fname);
    int wc_status = system(command);
    if(wc_status != 0){
        fprintf(stderr, "Problems meet during executing system command: %s\n", command);
    }
    return wc_status;
}

int get_file_size(FILE *fpath)
{
    fseek(fpath, 0L, SEEK_END);
    int size = (int)ftell(fpath);
    rewind(fpath);
    return size;
}

int load_file_to_array(char **array, char *fpath)
{
    FILE *f = fopen(fpath, "r");
    if(!f){
        fprintf(stderr, "Cannot read file %s", fpath);
        return -1;
    }
    int file_size = get_file_size(f);
    *array = calloc(file_size, sizeof(char));
    fread(*array, sizeof(char), file_size, f);
    fclose(f);
    return 0;
}

int add_file_to_table(char *fpath)
{
    for(int block_ctr = 0; block_ctr < table_size; block_ctr++)
    {
        if(table[block_ctr] == NULL){
            int load_status = load_file_to_array(&table[block_ctr], fpath);
            if(load_status){
               fprintf(stderr, "Problems meet during adding file %s to table", fpath);
               return -1;
            }
            return 0;
        }
    }
    fprintf(stderr, "Table is full - cannot add anoter file to it\n");
    return -1;
}

int add_result_to_table()
{
    return add_file_to_table(tmp_file);
}

int wc_file(char *fname)
{
    int prog_status = wc_call(fname);
    if(prog_status){
        return -1;
    }
    prog_status = add_result_to_table();
    if(prog_status){
        return -1;
    }
    return 0;
}

int wc_files(char *fnames)
{
    char *fnames_ = strdup(fnames);
    char *delimeter = " ";
    char *fname = strtok(fnames_, delimeter);
    while(fname)
    {
        int prog_status = wc_file(fname);
        if(prog_status){
            free(fnames_);
            return -1;
        }
        fname = strtok(NULL, delimeter);
    }
    free(fnames_);
    return 0;
}

int remove_block(unsigned int index)
{
    if(index >= table_size){
        fprintf(stderr, "Program tried to remove block that was out of table. Operation aborted.\n");
        return -1;
    }
    free(table[index]);
    table[index] = NULL;
    return 0;
}

int delete_table()
{
    for(int block_ctr = 0; block_ctr < table_size; block_ctr++)
    {
        remove_block(block_ctr);
    }
    is_initialized = false;
    table_size = 0;
    free(table);
    return 0;
}
