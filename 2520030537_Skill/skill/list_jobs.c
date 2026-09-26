#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_JOBS 10

typedef struct {
    int id;
    pid_t pid;
    int active;
    char command[100];
} Job;

Job jobs[MAX_JOBS];
int next_id = 1;

void update_jobs(void)
{
    int status;

    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active)
            continue;

        pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);

        if (result == jobs[i].pid) {
            jobs[i].active = 0;
            printf("Job [%d] completed.\n", jobs[i].id);
        }
    }
}

void list_jobs(void)
{
    int found = 0;

    update_jobs();

    printf("\nJOB ID\tPID\tSTATUS\tCOMMAND\n");

    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active) {
            printf("[%d]\t%ld\tRUNNING\t%s\n",
                   jobs[i].id,
                   (long)jobs[i].pid,
                   jobs[i].command);
            found = 1;
        }
    }

    if (!found)
        printf("No active jobs.\n");
}

void start_job(int seconds)
{
    int slot = -1;

    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        printf("Job table full.\n");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        char duration[20];
        snprintf(duration, sizeof(duration), "%d", seconds);
        execlp("sleep", "sleep", duration, (char *)NULL);
        perror("execlp");
        _exit(127);
    }

    jobs[slot].id = next_id++;
    jobs[slot].pid = pid;
    jobs[slot].active = 1;

    snprintf(jobs[slot].command,
             sizeof(jobs[slot].command),
             "sleep %d", seconds);

    printf("Started job [%d], PID=%ld\n",
           jobs[slot].id, (long)pid);
}

int main(void)
{
    char input[100];
    int seconds;
    char extra;

    printf("Q17 - List Active Jobs\n");
    printf("Commands: add NUMBER, jobs, exit\n");

    while (1) {
        update_jobs();

        printf("\njoblist> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strcmp(input, "jobs") == 0) {
            list_jobs();
        }
        else if (sscanf(input, "add %d %c",
                        &seconds, &extra) == 1 &&
                 seconds >= 0) {
            start_job(seconds);
        }
        else {
            printf("Invalid command.\n");
        }
    }

    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active)
            waitpid(jobs[i].pid, NULL, 0);
    }

    printf("Job listing program exited.\n");
    return 0;
}



