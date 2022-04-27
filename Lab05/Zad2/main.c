#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "stdbool.h"

#define MAX 200

int main(int argc, char** argv) {

    if(argc == 4)
    {
        char* command = calloc(MAX, sizeof(char));
        char* email = argv[1];
        char* title = argv[2];
        char* content = argv[3];

        sprintf(command, "mail -s \"%s\" %s << %s", title, email, content);
        FILE* fp;

        if (!(fp = popen(command, "w")))
        {
            fprintf(stderr, "Unable to read from command: %s\n", command);
            exit(EXIT_FAILURE);
        }
        pclose(fp);
        free(command);
        printf("Email has been sent\n");
    }
    else if(argc == 2) {

        char* type = argv[1];
        bool is_date = false;
        if(strcmp(type, "nadawca") == 0)
            is_date = false;
        else if(strcmp(type, "data") == 0)
            is_date = true;
        else
        {
            fprintf(stderr, "Invalid argument: %s\n", type);
            exit(EXIT_FAILURE);
        }

        char* command = calloc(MAX, sizeof(char));

        if (is_date)
            strcpy(command,"mail -H | sort -k 3");
        else
            strcpy(command, "mail -H | sort -k 7 | sort -k 6 | sort -k 5");

        FILE *fp;
        fp = popen(command, "r");

        char* line = calloc(MAX,sizeof(char));
        size_t len = 0;


        if(is_date)
            printf("Mails sorted in date order:\n");
        else
            printf("Mails sorted in sender order:\n");

        while(getline(&line, &len, fp) != EOF ){
            printf("%s", line);
            line = calloc(MAX,sizeof(char));
        }
        pclose(fp);
        free(line);
        free(command);
    }
    else
    {
        fprintf(stderr, "Invalid nr of arguments\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}