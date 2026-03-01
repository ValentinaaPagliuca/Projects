#include "job.h"
#include "shell.h"
#include <stdlib.h>
#include <string.h>

static int next_jid = 1;

bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line) {
    if (jobs == NULL || cmd_line == NULL) {
        return false;
    }

    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].cmd_line == NULL) {
            jobs[i].cmd_line = malloc(strlen(cmd_line) + 1);
            if (jobs[i].cmd_line == NULL) {
                return false;
            }
            strcpy(jobs[i].cmd_line, cmd_line);
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = next_jid++;
            return true;
        }
    }
    return false;
}

bool delete_job(job_t *jobs, pid_t pid) {
    if (jobs == NULL) {
        return false;
    }
    
    for (int i = 0; i < 16; i++) {  
        if (jobs[i].pid == pid) {
            if (jobs[i].cmd_line != NULL) {
                free(jobs[i].cmd_line);
                jobs[i].cmd_line = NULL;
            }
            jobs[i].pid = 0;
            jobs[i].jid = 0;
            jobs[i].state = UNDEFINED;
            return true;
        }
    }
    return false;
}
void free_jobs(job_t *jobs, int max_jobs) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].cmd_line != NULL) {
            free(jobs[i].cmd_line);
            jobs[i].cmd_line = NULL;
        }
        jobs[i].pid = 0;
        jobs[i].jid = 0;
        jobs[i].state = UNDEFINED;
    }

    free(jobs);
}

job_t *get_fg_job(job_t *jobs, int max_jobs) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid != 0 && jobs[i].state == FOREGROUND) {
            return &jobs[i];
        }
    }
    return NULL;
}

job_t *get_job_pid(job_t *jobs, int max_jobs, pid_t pid) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid == pid) {
            return &jobs[i];
        }
    }
    return NULL;
}


