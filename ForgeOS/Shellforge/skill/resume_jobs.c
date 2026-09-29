#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_JOBS 10

typedef enum
{
    EMPTY,
    RUNNING,
    STOPPED,
    COMPLETED
} JobState;

typedef struct
{
    int id;
    pid_t pid;
    pid_t pgid;
    JobState state;
    char command[100];
} Job;

Job jobs[MAX_JOBS];
int next_id = 1;

const char *state_name(JobState state)
{
    switch (state)
    {
        case RUNNING: return "RUNNING";
        case STOPPED: return "STOPPED";
        case COMPLETED: return "COMPLETED";
        default: return "EMPTY";
    }
}

void update_jobs(void)
{
    int status;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].state != RUNNING &&
            jobs[i].state != STOPPED)
            continue;

        pid_t result;

        do
        {
            result = waitpid(
                jobs[i].pid,
                &status,
                WNOHANG | WUNTRACED | WCONTINUED
            );
        }
        while (result == -1 && errno == EINTR);

        if (result == jobs[i].pid)
        {
            if (WIFSTOPPED(status))
                jobs[i].state = STOPPED;

            else if (WIFCONTINUED(status))
                jobs[i].state = RUNNING;

            else if (WIFEXITED(status) ||
                     WIFSIGNALED(status))
                jobs[i].state = COMPLETED;
        }
    }
}

void list_jobs(void)
{
    update_jobs();

    printf("\nJOB TABLE\n");
    printf("ID\tPID\tPGID\tSTATE\t\tCOMMAND\n");

    int found = 0;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].state != EMPTY)
        {
            printf(
                "%d\t%ld\t%ld\t%-10s\t%s\n",
                jobs[i].id,
                (long)jobs[i].pid,
                (long)jobs[i].pgid,
                state_name(jobs[i].state),
                jobs[i].command
            );

            found = 1;
        }
    }

    if (!found)
        printf("No jobs found.\n");
}

int find_job(int id)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].state != EMPTY &&
            jobs[i].id == id)
            return i;
    }

    return -1;
}

void start_job(int seconds)
{
    update_jobs();

    int slot = -1;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].state == EMPTY ||
            jobs[i].state == COMPLETED)
        {
            slot = i;
            break;
        }
    }

    if (slot == -1)
    {
        printf("Job table is full.\n");
        return;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        if (setpgid(0, 0) == -1)
        {
            perror("setpgid");
            _exit(1);
        }

        char duration[20];

        snprintf(duration, sizeof(duration),
                 "%d", seconds);

        execlp("sleep", "sleep",
               duration, (char *)NULL);

        perror("execlp");
        _exit(127);
    }

    if (setpgid(pid, pid) == -1 &&
        errno != EACCES)
    {
        perror("setpgid");
    }

    jobs[slot].id = next_id++;
    jobs[slot].pid = pid;
    jobs[slot].pgid = pid;
    jobs[slot].state = RUNNING;

    snprintf(
        jobs[slot].command,
        sizeof(jobs[slot].command),
        "sleep %d",
        seconds
    );

    printf(
        "Started job [%d], PID=%ld, PGID=%ld\n",
        jobs[slot].id,
        (long)pid,
        (long)pid
    );
}

void stop_job(int id)
{
    update_jobs();

    int index = find_job(id);

    if (index == -1 ||
        jobs[index].state != RUNNING)
    {
        printf("Running job not found.\n");
        return;
    }

    if (kill(-jobs[index].pgid, SIGSTOP) == -1)
    {
        perror("SIGSTOP");
        return;
    }

    int status;

    if (waitpid(jobs[index].pid,
                &status, WUNTRACED) == -1)
    {
        perror("waitpid");
        return;
    }

    if (WIFSTOPPED(status))
    {
        jobs[index].state = STOPPED;
        printf("Job [%d] stopped.\n", id);
    }
}

void resume_job(int id)
{
    update_jobs();

    int index = find_job(id);

    if (index == -1 ||
        jobs[index].state != STOPPED)
    {
        printf("Stopped job not found.\n");
        return;
    }

    if (kill(-jobs[index].pgid, SIGCONT) == -1)
    {
        perror("SIGCONT");
        return;
    }

    jobs[index].state = RUNNING;

    printf(
        "SIGCONT sent to job [%d].\n",
        id
    );

    printf("Job resumed in background.\n");
}

void remove_completed(void)
{
    update_jobs();

    int removed = 0;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].state == COMPLETED)
        {
            printf(
                "Removing completed job [%d].\n",
                jobs[i].id
            );

            memset(&jobs[i], 0,
                   sizeof(jobs[i]));

            removed++;
        }
    }

    if (!removed)
        printf("No completed jobs to remove.\n");
}

int main(void)
{
    char input[100];
    int number;
    char extra;

    printf("Q18 - Background Job Recovery\n");
    printf("Commands:\n");
    printf("add NUMBER\n");
    printf("stop JOB_ID\n");
    printf("bg JOB_ID\n");
    printf("jobs\n");
    printf("remove\n");
    printf("exit\n");

    while (1)
    {
        update_jobs();

        printf("\nrecovery> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strcmp(input, "jobs") == 0)
        {
            list_jobs();
        }
        else if (strcmp(input, "remove") == 0)
        {
            remove_completed();
        }
        else if (sscanf(input, "add %d %c",
                        &number, &extra) == 1 &&
                 number >= 0)
        {
            start_job(number);
        }
        else if (sscanf(input, "stop %d %c",
                        &number, &extra) == 1)
        {
            stop_job(number);
        }
        else if (sscanf(input, "bg %d %c",
                        &number, &extra) == 1)
        {
            resume_job(number);
        }
        else
        {
            printf("Invalid command.\n");
        }
    }

    /* Reap all remaining children before exiting */
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].state == STOPPED)
        {
            kill(-jobs[i].pgid, SIGCONT);
        }

        if (jobs[i].state == RUNNING ||
            jobs[i].state == STOPPED)
        {
            waitpid(jobs[i].pid, NULL, 0);
        }
    }

    printf("Background recovery program exited.\n");

    return 0;
}
