#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define INPUT_SIZE 200
#define MAX_JOBS 20

typedef struct
{
    int id;
    pid_t pid;
    char command[INPUT_SIZE];
    int active;
} Job;

Job jobs[MAX_JOBS];
int next_id = 1;

/* Remove leading and trailing whitespace */
char *trim(char *str)
{
    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return str;

    char *end = str + strlen(str) - 1;

    while (end > str &&
           isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }

    return str;
}

/* Check whether a command ends with & */
int detect_background(char *command)
{
    char *clean = trim(command);
    size_t length = strlen(clean);

    if (length > 0 && clean[length - 1] == '&')
    {
        clean[length - 1] = '\0';
        return 1;
    }

    return 0;
}

/* Store information about a background job */
int add_job(pid_t pid, const char *command)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (!jobs[i].active)
        {
            jobs[i].id = next_id++;
            jobs[i].pid = pid;

            snprintf(
                jobs[i].command,
                sizeof(jobs[i].command),
                "%s",
                command
            );

            jobs[i].active = 1;

            return jobs[i].id;
        }
    }

    return -1;
}

/* Display currently running jobs */
void show_jobs(void)
{
    int found = 0;

    printf("\nBackground Jobs:\n");

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active)
        {
            printf(
                "[%d] PID=%ld RUNNING %s\n",
                jobs[i].id,
                (long)jobs[i].pid,
                jobs[i].command
            );

            found = 1;
        }
    }

    if (!found)
        printf("No active background jobs.\n");
}

/* Check completed background processes */
void monitor_jobs(void)
{
    int status;
    pid_t pid;

    while (1)
    {
        pid = waitpid(-1, &status, WNOHANG);

        if (pid <= 0)
            break;

        for (int i = 0; i < MAX_JOBS; i++)
        {
            if (jobs[i].active &&
                jobs[i].pid == pid)
            {
                printf(
                    "\n[%d] Background job completed. PID=%ld\n",
                    jobs[i].id,
                    (long)pid
                );

                if (WIFEXITED(status))
                {
                    printf(
                        "Exit status: %d\n",
                        WEXITSTATUS(status)
                    );
                }
                else if (WIFSIGNALED(status))
                {
                    printf(
                        "Terminated by signal: %d\n",
                        WTERMSIG(status)
                    );
                }

                jobs[i].active = 0;
                break;
            }
        }
    }
}

/* Execute sleep N in a child process */
void launch_job(char *command, int background)
{
    int seconds;
    char extra;

    if (sscanf(command, "sleep %d %c",
               &seconds, &extra) != 1 ||
        seconds < 0)
    {
        printf("Error: Use sleep NUMBER.\n");
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

    if (background)
    {
        int id = add_job(pid, command);

        if (id == -1)
        {
            printf("Job table full.\n");
            waitpid(pid, NULL, 0);
            return;
        }

        printf(
            "[%d] Background job started. PID=%ld\n",
            id,
            (long)pid
        );

        printf("Prompt returned immediately.\n");
    }
    else
    {
        printf("Waiting for foreground process...\n");

        if (waitpid(pid, NULL, 0) == -1)
            perror("waitpid");

        printf("Foreground process completed.\n");
    }
}

int main(void)
{
    char input[INPUT_SIZE];

    printf("Background Jobs Program\n");
    printf("Supported commands:\n");
    printf("sleep 5 &\n");
    printf("sleep 2\n");
    printf("jobs\n");
    printf("check\n");
    printf("exit\n");

    while (1)
    {
        monitor_jobs();

        printf("\nbg> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        char *command = trim(input);

        if (strcmp(command, "exit") == 0)
            break;

        if (strcmp(command, "jobs") == 0)
        {
            monitor_jobs();
            show_jobs();
            continue;
        }

        if (strcmp(command, "check") == 0)
        {
            monitor_jobs();
            show_jobs();
            continue;
        }

        int background = detect_background(command);
        command = trim(command);

        if (*command == '\0')
        {
            printf("Error: Empty command.\n");
            continue;
        }

        if (strncmp(command, "sleep ", 6) == 0)
        {
            launch_job(command, background);
        }
        else
        {
            printf("Unsupported command.\n");
            printf("Use sleep NUMBER or sleep NUMBER &.\n");
        }
    }

    /* Wait for any jobs still running before exiting */
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active)
        {
            waitpid(jobs[i].pid, NULL, 0);
            jobs[i].active = 0;
        }
    }

    printf("Background jobs program exited.\n");

    return 0;
}
