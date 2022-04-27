#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H
    #include <stdlib.h>
    #include <stdio.h>
    #include <dlfcn.h>

    static void *handle = NULL;
    int (*_create_table)(unsigned int size);
    int (*_wc_files)(char *fnames);
    int (*_remove_block)(unsigned int index);
    int (*_delete_table)();
    char* (*_create_wc_command)(char *command, char *fname);
    int (*_wc_call)(char *fname);
    int (*_get_file_size)(FILE *fpath);
    int (*_load_file_to_array)(char **array, char *fpath);
    int (*_add_file_to_table)(char *fpath);
    int (*_add_result_to_table)();
    int (*_wc_file)(char *fname);

    void init_dynamic_library()
    {
        handle = dlopen("../Zad1/lib_wc.so", RTLD_LAZY);
        if (handle == NULL)
        {
            fprintf(stderr, "Problems meet during opening dynamic library\n");
            exit(-1);
        }

        _create_table = dlsym(handle, "create_table");
        _wc_files = dlsym(handle, "wc_files");
        _remove_block = dlsym(handle, "remove_block");
        _delete_table = dlsym(handle, "delete_table");

        _create_wc_command = dlsym(handle, "create_wc_command");
        _wc_call = dlsym(handle, "wc_call");
        _get_file_size = dlsym(handle, "get_file_size");
        _load_file_to_array = dlsym(handle, "load_file_to_array");
        _add_file_to_table = dlsym(handle, "add_file_to_table");
        _add_result_to_table = dlsym(handle, "add_result_to_table");
        _wc_file = dlsym(handle, "wc_file");

        char *error;
        if ((error = dlerror()) != NULL)
        {
            fprintf(stderr, "%s\n", error);
            printf("Problems meet during initializing dynamic library\n");
            return;
        }
    }

    int create_table(unsigned int size)
    {
        return (_create_table)(size);
    }

    int wc_files(char *fnames)
    {
        return (_wc_files)(fnames);
    }

    int remove_block(unsigned int index)
    {
        return (_remove_block)(index);
    }
    int delete_table()
    {
        return (_delete_table)();
    }

    char* create_wc_command(char *command, char *fname)
    {
        return (_create_wc_command)(command, fname);
    }

    int wc_call(char *fname)
    {
        return (_wc_call)(fname);
    }
    int get_file_size(FILE *fpath)
    {
        return (_get_file_size)(fpath);
    }
    int load_file_to_array(char **array, char *fpath)
    {
        return (_load_file_to_array)(array, fpath);
    }
    int add_file_to_table(char *fpath)
    {
        return (_add_file_to_table)(fpath);
    }
    int add_result_to_table()
    {
        return (_add_result_to_table)();
    }
    int wc_file(char *fname)
    {
        return (_wc_file)(fname);
    }

#endif //DYNAMIC_LIBRARY_H
