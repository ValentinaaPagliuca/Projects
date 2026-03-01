#include "shell.h"
#include "job.h"
#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    
#include <sys/types.h> 
#include <sys/wait.h>  

const int MAX_LINE = 1024;
const int MAX_JOBS = 16;
const int MAX_HISTORY = 10;
msh_t *shell = NULL; 


msh_t *alloc_shell(int max_jobs, int max_line, int max_history) {
    shell = malloc(sizeof(msh_t));
    if (!shell) {
        return NULL; 
    }
    if (max_jobs == 0) {
        max_jobs = MAX_JOBS;
    }
    if (max_line == 0) {
        max_line = MAX_LINE;
    }
    if (max_history == 0) {
        max_history = MAX_HISTORY;
    }

    shell->max_jobs = max_jobs;
    shell->max_line = max_line;
    shell->max_history = max_history;
    
    shell->jobs = malloc(max_jobs * sizeof(job_t));
    for (int i = 0; i < max_jobs; i++) {
        shell->jobs[i].cmd_line = NULL; 
        shell->jobs[i].pid = 0;
        shell->jobs[i].jid = 0;
        shell->jobs[i].state = UNDEFINED;
    }
    initialize_signal_handlers();
    return shell;
}

char *parse_tok(char *line, int *job_type) {
    static char *ptr = NULL;   
    if (line != NULL) {
        ptr = line;
    }
    if (job_type == NULL) {
        return NULL;
    }

    if (ptr == NULL || *ptr == '\0') {
        *job_type = -1;
        return NULL;
    }

    char *tmp = ptr;
    int space = 1;                
    while (*tmp != '\0') {
        if (*tmp != ' ' && *tmp != '\t' && *tmp != '\n') {
            space = 0;            
            break;
        }
        tmp++;
    }
    if (space) {                 
        *job_type = -1;
        ptr = NULL;
        return NULL;
    }

    char *job_start = ptr;
    while (*ptr != '\0') {
        if (*ptr == '&' || *ptr == ';') {
            char sep = *ptr;
            *ptr = '\0';      
            ptr++;
            char *q = job_start;
            int only_space = 1; 
            while (*q != '\0') {
                if (*q != ' ' && *q != '\t' && *q != '\n') {
                    only_space = 0;
                    break;
                }
                q++;
            }

            if (only_space) {
                return parse_tok(NULL, job_type);
            }

            if (sep == '&') {
                *job_type = 0;  // BACKGROUND
            } else {
                *job_type = 1;  // FOREGROUND
            }
            return job_start;
        }
        ptr++;
    }

    *job_type = 1;  
    ptr = NULL;     
    char *q = job_start;
    int only_space = 1;
    while (*q != '\0') {
        if (*q != ' ' && *q != '\t' && *q != '\n') {
            only_space = 0;
            break;
        }
        q++;
    }
    if (only_space) {
        *job_type = -1;
        return NULL;
    }
    return job_start;
}

char **separate_args(char *line, int *argc, bool *is_builtin) {
    if (line == NULL) {
        *argc = 0;
        return NULL;
    }

    while (*line == ' ') line++;
    if (*line == '\0') {
        *argc = 0;
        return NULL;
    }
    
    int count = 0;
    int in_word = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] != ' ' && in_word == 0) {
            count++;
            in_word = 1;
        } else if (line[i] == ' ') {
            in_word = 0;
        }
    }
    
    *argc = count;
    char **argv = malloc((count + 1) * sizeof(char *));
    if (!argv) {
        *argc = 0;
        return NULL;
    }

    int arg_index = 0;
    int i = 0;
    while (line[i] != '\0' && arg_index < count) {
        while (line[i] == ' ') i++;
        if (line[i] == '\0') break;

        int start = i;
        while (line[i] != '\0' && line[i] != ' ') i++;
        int end = i;
        int len = end - start;
        
        argv[arg_index] = malloc(len + 1);
        for (int k = 0; k < len; k++) {
            argv[arg_index][k] = line[start + k];
        }
        argv[arg_index][len] = '\0';
        arg_index++;
    }
    argv[count] = NULL;  
    return argv;
}

int evaluate(msh_t *shell, char *line) {
    if (line == NULL || line[0] == '\0') {
        return 0;
    }
    if (strlen(line) > shell->max_line) {
        printf("error: reached the maximum line limit\n");
        return 0;
    }

    //history
    if (line[0] == '!') {
        int num = atoi(line + 1);
        char *cmd = find_line_history(shell->history, num);

        if (cmd != NULL) {
            printf("%s\n", cmd);   
            evaluate(shell, cmd); 
        } else {
            printf("error: invalid history reference\n");
        }
        return 0; 
    }

    if (shell->history != NULL) {
        add_line_history(shell->history, line);
    }

    int job_type;
    char *current_job = parse_tok(line, &job_type);
    while (current_job != NULL) {
        int argc = 0;
        bool is_builtin = false;
        char **argv = separate_args(current_job, &argc, &is_builtin);
        if (argv == NULL || argc == 0) {
            current_job = parse_tok(NULL, &job_type);
            continue;
        }
        
        char *builtin = builtin_cmd(argv);
        if (builtin != NULL) {
            for (int i = 0; i < argc; i++) {
                free(argv[i]);
            }
            free(argv);
            current_job = parse_tok(NULL, &job_type);
            continue;
        }


        sigset_t mask, prev;
        sigemptyset(&mask);
        sigaddset(&mask, SIGCHLD);
        sigprocmask(SIG_BLOCK, &mask, &prev);
        pid_t pid = fork();
        
        if (pid == 0) {         // CHILD PROCESS
            setpgid(0, 0);                 
            sigprocmask(SIG_SETMASK, &prev, NULL);
            execve(argv[0], argv, NULL);
            perror("execve");
            exit(1);
        } else if (pid > 0){    
            add_job(shell->jobs, shell->max_jobs, pid,
                    job_type == 0 ? BACKGROUND : FOREGROUND,
                    current_job);
            sigprocmask(SIG_SETMASK, &prev, NULL);
            job_t *j = get_job_pid(shell->jobs, shell->max_jobs, pid);
            if (job_type == 0) {
                printf("[%d] %d %s\n", j->jid, j->pid, j->cmd_line);
            }
            else {
                waitfg(pid);
                delete_job(shell->jobs, pid);
            }
        }
        
        for (int i = 0; i < argc; i++) {
            free(argv[i]);
        }
        free(argv);
        current_job = parse_tok(NULL, &job_type);
    }

    return 0;
}

void exit_shell(msh_t *shell) {
    if (shell == NULL) {
        return;
    }
    if (shell->jobs != NULL) {
        free_jobs(shell->jobs, shell->max_jobs);
    }
    free(shell);
}

void waitfg(pid_t pid) {
    while (1) {
        job_t *fg = get_fg_job(shell->jobs, shell->max_jobs);
        if (fg == NULL || fg->pid != pid) {
            break;
        }

        sleep(1); 
    }
}

char *builtin_cmd(char **argv) {
    if (argv == NULL || argv[0] == NULL) {
        return NULL;
    }

    if (strcmp(argv[0], "jobs") == 0) {
        for (int i = 0; i < shell->max_jobs; i++) {
            job_t *j = &shell->jobs[i];
            if (j->pid != 0 && j->state != UNDEFINED) {
                char *state;
                if (j->state == BACKGROUND) {
                    state = "RUNNING";
                } else if (j->state == SUSPENDED) {
                    state = "Stopped";
                } else {
                    continue;
                }
                printf("[%d] %d %s %s\n", j->jid, j->pid, state, j->cmd_line);
            }
        }
        return "";
    }

    if (strcmp(argv[0], "history") == 0) {
        print_history(shell->history);
        return "";
    }
    if (strcmp(argv[0], "bg") == 0) {
        if (argv[1] == NULL) {
            return "";
        }

        job_t *j = NULL;
        if (argv[1][0] == '%') {
            int jid = atoi(argv[1] + 1);
            for (int i = 0; i < shell->max_jobs; i++) {
                if (shell->jobs[i].jid == jid) {
                    j = &shell->jobs[i];
                    break;
                }
            }
        } else {
            pid_t pid = atoi(argv[1]);
            j = get_job_pid(shell->jobs, shell->max_jobs, pid);
        }

        if (j != NULL) {
            kill(-j->pid, SIGCONT);
            j->state = BACKGROUND;
            printf("[%d] %d %s\n", j->jid, j->pid, j->cmd_line);
        }
        return "";
    }

    if (strcmp(argv[0], "fg") == 0) {
        if (argv[1] == NULL) {
            return "";
        }
        job_t *j = NULL;
        if (argv[1][0] == '%') {
            int jid = atoi(argv[1] + 1);
            for (int i = 0; i < shell->max_jobs; i++) {
                if (shell->jobs[i].jid == jid) {
                    j = &shell->jobs[i];
                    break;
                }
            }
        } else {
            pid_t pid = atoi(argv[1]);
            j = get_job_pid(shell->jobs, shell->max_jobs, pid);
        }

        if (j != NULL) {
            kill(-j->pid, SIGCONT);
            j->state = FOREGROUND;
            waitfg(j->pid);
        }
        return "";
    }

    if (strcmp(argv[0], "kill") == 0) {
        if (argv[1] == NULL || argv[2] == NULL) {
            return "";
        }
        int sig = atoi(argv[1]);
        pid_t pid = atoi(argv[2]);
        if (sig != 2 && sig != 9 && sig != 18 && sig != 19) {
            printf("error: invalid signal number\n");
            return "";
        }
        kill(pid, sig);
        return "";
    }

    return NULL;
}
