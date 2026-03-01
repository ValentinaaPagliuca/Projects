#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *HISTORY_FILE_PATH = "../data/.msh_history";

history_t *alloc_history(int max_history) {
    history_t *history = malloc(sizeof(history_t));
    if (history == NULL) {
        return NULL;
    }
    history->max_history = max_history;
    history->next = 0;
    history->lines = malloc(sizeof(char *) * max_history);
    if (history->lines == NULL) {
        free(history);
        return NULL;
    }

    for (int i = 0; i < max_history; i++) {
        history->lines[i] = NULL;
    }
    FILE *fp = fopen(HISTORY_FILE_PATH, "r");
    if (fp == NULL) {
        return history;
    }
    char buffer[1024];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (count >= max_history) {
            break;
        }
        int len = strcspn(buffer, "\n");
        buffer[len] = '\0';
        history->lines[count] = malloc(len + 1);
        if (history->lines[count] == NULL) {
            break;
        }
        strcpy(history->lines[count], buffer);
        count++;
    }
    history->next = count;
    fclose(fp);
    return history;
}

void add_line_history(history_t *history, const char *cmd_line) {
    if (history == NULL || cmd_line == NULL || cmd_line[0] == '\0' || strcmp(cmd_line, "exit") == 0) {
        return;
    }

    if (history->next == history->max_history) {
        if (history->lines[0] != NULL) {
            free(history->lines[0]);
        }
        for (int i = 1; i < history->max_history; i++) {
            history->lines[i - 1] = history->lines[i];
        }
        history->lines[history->max_history - 1] = NULL;
        history->next = history->max_history - 1;
    }

    int len = strlen(cmd_line);
    history->lines[history->next] = malloc(len + 1);
    if (history->lines[history->next] == NULL) {
        return;
    }
    strcpy(history->lines[history->next], cmd_line);
    history->next++;
}

void print_history(history_t *history) {
    for (int i = 1; i <= history->next; i++) {
        printf("%5d\t%s\n", i, history->lines[i - 1]);
    }
}

char *find_line_history(history_t *history, int index) {
    if (history == NULL || index < 1 || index > history->next) {
        return NULL;
    }
    return history->lines[index - 1];
}

void free_history(history_t *history) {
    if (history == NULL) {
        return;
    }
    FILE *fp = fopen(HISTORY_FILE_PATH, "w");
    if (fp != NULL) {
        for (int i = 0; i < history->next; i++) {
            fprintf(fp, "%s\n", history->lines[i]);
        }
        fclose(fp);
    }
    for (int i = 0; i < history->max_history; i++) {
        if (history->lines[i] != NULL) {
            free(history->lines[i]);
        }
    }
    free(history->lines);
    free(history);
}
