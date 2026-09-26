#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

static pid_t shell_pgid;
static int next_job_id = 1;

typedef struct {
    int id;
    pid_t pid;
    pid_t pgid;
    int active;
    int stopped;
} Job;

static Job job = {0};

void update_job(void)
{
    int status;

    if (!job.active)
        return;

    pid_t result = waitpid(
        job.pid, &status,
        WNOHANG | WUNTRACED | WCONTINUED
    );

    if (result == job.pid) {
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            printf("\nJob [%d] completed.\n", job.id);
            job.active = 0;
            job.stopped = 0;
        }
        else if (WIFSTOPPED(status)) {
            job.stopped = 1;
        }
        else if (WIFCONTINUED(status)) {
            job.stopped = 0;
        }
    }
}

void start_job(int seconds)
{
    if (job.active) {
        printf("Only one active job is supported.\n");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        setpgid(0, 0);

        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);

        char duration[20];
        snprintf(duration, sizeof(duration), "%d", seconds);

        execlp("sleep", "sleep", duration, (char *)NULL);
        perror("execlp");
        _exit(127);
    }

    if (setpgid(pid, pid) == -1 && errno != EACCES)
        perror("setpgid");

    job.id = next_job_id++;
    job.pid = pid;
    job.pgid = pid;
    job.active = 1;
    job.stopped = 0;

    printf("Started background job [%d], PID=%ld, PGID=%ld\n",
           job.id, (long)job.pid, (long)job.pgid);
}

void show_job(void)
{
    update_job();

    if (!job.active) {
        printf("No active job.\n");
        return;
    }

    printf("[%d] PID=%ld PGID=%ld STATUS=%s\n",
           job.id,
           (long)job.pid,
           (long)job.pgid,
           job.stopped ? "STOPPED" : "RUNNING");
}

void foreground_job(int id)
{
    int status;

    update_job();

    if (!job.active || job.id != id) {
        printf("Job not found.\n");
        return;
    }

    printf("Moving job [%d] to foreground...\n", job.id);
    fflush(stdout);

    if (tcsetpgrp(STDIN_FILENO, job.pgid) == -1) {
        perror("tcsetpgrp");
        return;
    }

    if (job.stopped) {
        if (kill(-job.pgid, SIGCONT) == -1) {
            perror("SIGCONT");
            tcsetpgrp(STDIN_FILENO, shell_pgid);
            return;
        }

        job.stopped = 0;
        printf("Job resumed.\n");
        fflush(stdout);
    }

    pid_t result;

    do {
        result = waitpid(job.pid, &status, WUNTRACED);
    } while (result == -1 && errno == EINTR);

    /* Return terminal control to this program. */
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) == -1)
        perror("restore terminal");

    if (result == -1) {
        perror("waitpid");
        return;
    }

    if (WIFSTOPPED(status)) {
        job.stopped = 1;
        printf("Job [%d] stopped.\n", job.id);
    }
    else {
        printf("Job [%d] completed in foreground.\n", job.id);
        job.active = 0;
        job.stopped = 0;
    }
}

int main(void)
{
    char input[100];
    int number;
    char extra;

    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Run in an interactive terminal.\n");
        return 1;
    }

    shell_pgid = getpgrp();

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    printf("Q17 - Foreground Switching\n");
    printf("Commands: add NUMBER, jobs, fg JOB_ID, exit\n");

    while (1) {
        update_job();

        printf("\nfgshell> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strcmp(input, "jobs") == 0) {
            show_job();
        }
        else if (sscanf(input, "add %d %c",
                        &number, &extra) == 1 &&
                 number >= 0) {
            start_job(number);
        }
        else if (sscanf(input, "fg %d %c",
                        &number, &extra) == 1) {
            foreground_job(number);
        }
        else {
            printf("Invalid command.\n");
        }
    }

    if (job.active) {
        if (job.stopped)
            kill(-job.pgid, SIGCONT);

        waitpid(job.pid, NULL, 0);
    }

    printf("Foreground switching program exited.\n");
    return 0;
}
