#define _GNU_SOURCE
#include "shell.h"
#include "job.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    opterr = 0;
    int s = 0;
    int j = 0;
    int l = 0;
    int opt;
    
    while ((opt = getopt(argc, argv, "s:j:l:")) != -1) {
        switch (opt) {
            case 's': {
                int val;
                if (sscanf(optarg, "%d", &val) != 1) {
                    printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                    return 1;
                }
                if (val <= 0) {
                    printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                    return 1;
                }
                s = val;
                break;
            }
            case 'j': {
                int val;
                if (sscanf(optarg, "%d", &val) != 1) {
                    printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                    return 1;
                }
                if (val <= 0) {
                    printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                    return 1;
                }
                j = val;
                break;
            }
            case 'l': {
                int val;
                if (sscanf(optarg, "%d", &val) != 1) {
                    printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                    return 1;
                }
                if (val <= 0) {
                    printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                    return 1;
                }
                l = val;
                break;
            }
            default:
                printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                return 1;
        }
    }
    
    if (optind < argc) {
        printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
        return 1;
    }
    
    shell = alloc_shell(j, l, s);
    if (shell == NULL) {
        return 1;
    }
    char *line = NULL;
    size_t len = 0;
    long read;
    read = getline(&line, &len, stdin);
    
    while (read != -1) {
        printf("msh>");
        fflush(stdout);
        
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        
        if (strcmp(line, "exit") == 0) {
            printf(" \n");
            free(line);
            line=NULL;
            exit_shell(shell);
            return 0;
        }
        evaluate(shell, line);
        free(line);
        line = NULL;
        read = getline(&line, &len, stdin);
    }
    
    printf(" \n");
    free(line);
    exit_shell(shell);
    return 0;
}