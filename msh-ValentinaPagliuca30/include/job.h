
  #ifndef _JOB_H_
  #define _JOB_H_ 

  #include <sys/types.h>
  #include <stdbool.h>

  typedef enum job_state{FOREGROUND, BACKGROUND, SUSPENDED, UNDEFINED} job_state_t; 

  // Represents a job in a shell. 
  typedef struct job {
      char *cmd_line;     // The command line for this specific job. 
      job_state_t state;  // The current state for this job 
      pid_t pid;          // The process id for this job 
      int jid;            // The job number for this job 
  }job_t;
  
/*
 * add_job: adds a new job to the jobs array
 *
 * jobs: array of job_t
 * 
 * max_jobs: max number of job in jobs[].
 * 
 * pid: process id 
 * 
 * state: initial job state
 * 
 * cmd_line: command string to store
 *
 * Returns: success --> true , false if no space
 */
  bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line);
  
  
  /*
 * delete_job: removes a job from the array by pid.
 *
 * jobs: array of job_t elements.
 * 
 * pid: pid of the job to remove.
 *
 * Returns: success -> true, false if pid does not exist.
 */
  
  bool delete_job(job_t *jobs, pid_t pid);

  /*
 * free_jobs: frees all job memory , every cmd_line and then frees the entire array.
 *
 * jobs: array of job_t.
 * 
 * max_jobs: max number of jobs allocated. 
 */
  void free_jobs(job_t *jobs, int max_jobs);

/*
 * get_fg_job: returns the job currently running in the foreground.
 *
 * jobs: array of job_t elements.
 *
 * max_jobs: maximum number of jobs allowed in the array.
 *
 * Returns: pointer to the foreground job if it exists, otherwise NULL.
 */
  job_t *get_fg_job(job_t *jobs, int max_jobs);

   /*
 * get_job_pid: finds and returns a job by its process id.
 *
 * jobs: array of job_t elements.
 *
 * max_jobs: maximum number of jobs allowed in the array.
 *
 * pid: process id of the job being searched for.
 *
 * Returns: pointer to the matching job_t if found, otherwise NULL.
 */
  job_t *get_job_pid(job_t *jobs, int max_jobs, pid_t pid);
  #endif 
  