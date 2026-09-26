#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_JOBS 10

typedef enum
{
    EMPTY,
    RUNNING,
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

Job table[MAX_JOBS];
int next_id = 1;

/* Convert a state to readable text */
const char *state_name(JobState state)
{
    switch (state)
    {
        case RUNNING:
            return "RUNNING";

        case COMPLETED:
            return "COMPLETED";

        default:
            return "EMPTY";
    }
}

/* Add a job to the table */
int add_job(
    pid_t pid,
    pid_t pgid,
    const char *command
)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (table[i].state == EMPTY)
        {
            table[i].id = next_id++;
            table[i].pid = pid;
            table[i].pgid = pgid;
            table[i].state = RUNNING;

            snprintf(
                table[i].command,
                sizeof(table[i].command),
                "%s",
                command
            );

            return table[i].id;
        }
    }

    return -1;
}

/* Display the job table */
void display_jobs(void)
{
    int found = 0;

    printf("\nJOB TABLE\n");
    printf("ID\tPID\tPGID\tSTATE\t\tCOMMAND\n");

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (table[i].state != EMPTY)
        {
            printf(
                "%d\t%ld\t%ld\t%-10s\t%s\n",
                table[i].id,
                (long)table[i].pid,
                (long)table[i].pgid,
                state_name(table[i].state),
                table[i].command
            );

            found = 1;
        }
    }

    if (!found)
        printf("No job records found.\n");
}

/* Update completed jobs without blocking */
void update_jobs(void)
{
    int status;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (table[i].state != RUNNING)
            continue;

        pid_t result = waitpid(
            table[i].pid,
            &status,
            WNOHANG
        );

        if (result == table[i].pid)
        {
            table[i].state = COMPLETED;

            printf(
                "Job %d completed. PID=%ld\n",
                table[i].id,
                (long)table[i].pid
            );
        }
        else if (result == -1)
        {
            perror("waitpid");
        }
    }
}

/* Remove completed records */
void remove_completed(void)
{
    int removed = 0;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (table[i].state == COMPLETED)
        {
            printf(
                "Removed completed job %d.\n",
                table[i].id
            );

            memset(&table[i], 0, sizeof(table[i]));
            table[i].state = EMPTY;

            removed++;
        }
    }

    if (removed == 0)
        printf("No completed jobs to remove.\n");
}

/* Validate job records */
void validate_jobs(void)
{
    int valid = 1;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (table[i].state == EMPTY)
            continue;

        if (table[i].id <= 0 ||
            table[i].pid <= 0 ||
            table[i].pgid <= 0 ||
            table[i].command[0] == '\0')
        {
            printf(
                "Invalid record in slot %d.\n",
                i
            );

            valid = 0;
        }

        for (int j = i + 1; j < MAX_JOBS; j++)
        {
            if (table[j].state != EMPTY &&
                table[i].id == table[j].id)
            {
                printf("Duplicate job ID found.\n");
                valid = 0;
            }
        }
    }

    if (valid)
        printf("All job records are valid.\n");
}

/* Create a process group and launch a test job */
void launch_job(int seconds)
{
    if (seconds < 0)
    {
        printf("Invalid duration.\n");
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

        snprintf(
            duration,
            sizeof(duration),
            "%d",
            seconds
        );

        execlp(
            "sleep",
            "sleep",
            duration,
            (char *)NULL
        );

        perror("execlp");
        _exit(127);
    }

    /*
     * The child creates its own process group.
     * Its PGID is its PID.
     */
    if (setpgid(pid, pid) == -1 &&
        errno != EACCES)
    {
        perror("setpgid");
    }

    char command[100];

    snprintf(
        command,
        sizeof(command),
        "sleep %d",
        seconds
    );

    int id = add_job(pid, pid, command);

    if (id == -1)
    {
        printf("Job table is full.\n");
        waitpid(pid, NULL, 0);
        return;
    }

    printf(
        "Job added: ID=%d PID=%ld PGID=%ld\n",
        id,
        (long)pid,
        (long)pid
    );
}

int main(void)
{
    char input[100];
    int seconds;
    char extra;

    printf("Job Table Management Program\n");

    printf("\nAvailable commands:\n");
    printf("add NUMBER\n");
    printf("list\n");
    printf("update\n");
    printf("remove\n");
    printf("validate\n");
    printf("exit\n");

    while (1)
    {
        printf("\njob> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (sscanf(input, "add %d %c",
                   &seconds, &extra) == 1)
        {
            launch_job(seconds);
        }
        else if (strcmp(input, "list") == 0)
        {
            display_jobs();
        }
        else if (strcmp(input, "update") == 0)
        {
            update_jobs();
        }
        else if (strcmp(input, "remove") == 0)
        {
            update_jobs();
            remove_completed();
        }
        else if (strcmp(input, "validate") == 0)
        {
            validate_jobs();
        }
        else
        {
            printf("Invalid command.\n");
        }
    }

    /* Reap any remaining child processes */
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (table[i].state == RUNNING)
        {
            waitpid(table[i].pid, NULL, 0);
            table[i].state = COMPLETED;
        }
    }

    printf("Job table program exited.\n");

    return 0;
}

