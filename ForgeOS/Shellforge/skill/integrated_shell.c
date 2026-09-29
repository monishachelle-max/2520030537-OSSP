#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_ARGS 20
#define MAX_JOBS 10
#define INPUT_SIZE 256

typedef struct {
    int id;
    pid_t pid;
    char command[INPUT_SIZE];
    int active;
} Job;

static Job jobs[MAX_JOBS];
static int next_id = 1;

static void update_jobs(void)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].active && jobs[i].pid == pid) {
                printf("\n[%d] Job completed: %s\n",
                       jobs[i].id, jobs[i].command);
                jobs[i].active = 0;
                break;
            }
        }
    }
}

static void show_jobs(void)
{
    int found = 0;
    update_jobs();

    printf("\nID\tPID\tCOMMAND\n");

    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active) {
            printf("%d\t%ld\t%s\n",
                   jobs[i].id, (long)jobs[i].pid,
                   jobs[i].command);
            found = 1;
        }
    }

    if (!found)
        printf("No active background jobs.\n");
}

static int tokenize(char *line, char *args[])
{
    int count = 0;
    char *token = strtok(line, " \t");

    while (token != NULL) {
        if (count >= MAX_ARGS - 1) {
            fprintf(stderr, "Error: Too many arguments.\n");
            return -1;
        }

        args[count++] = token;
        token = strtok(NULL, " \t");
    }

    args[count] = NULL;
    return count;
}

static void execute_command(char *input)
{
    char original[INPUT_SIZE];
    char *args[MAX_ARGS];

    snprintf(original, sizeof(original), "%s", input);

    int count = tokenize(input, args);

    if (count <= 0)
        return;

    int background = 0;

    if (strcmp(args[count - 1], "&") == 0) {
        background = 1;
        args[--count] = NULL;

        if (count == 0) {
            fprintf(stderr, "Error: Missing command before &.\n");
            return;
        }
    }

    if (strcmp(args[0], "pwd") == 0) {
        char cwd[INPUT_SIZE];

        if (count != 1 || background) {
            fprintf(stderr, "Usage: pwd\n");
            return;
        }

        if (getcwd(cwd, sizeof(cwd)))
            printf("%s\n", cwd);
        else
            perror("getcwd");

        return;
    }

    if (strcmp(args[0], "cd") == 0) {
        if (count != 2 || background) {
            fprintf(stderr, "Usage: cd DIRECTORY\n");
            return;
        }

        if (chdir(args[1]) == -1)
            perror("cd");

        return;
    }

    if (strcmp(args[0], "jobs") == 0) {
        if (count != 1 || background) {
            fprintf(stderr, "Usage: jobs\n");
            return;
        }

        show_jobs();
        return;
    }

    if (background) {
        int free_slot = -1;

        update_jobs();

        for (int i = 0; i < MAX_JOBS; i++) {
            if (!jobs[i].active) {
                free_slot = i;
                break;
            }
        }

        if (free_slot == -1) {
            fprintf(stderr, "Error: Job table full.\n");
            return;
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return;
        }

        if (pid == 0) {
            execvp(args[0], args);
            perror(args[0]);
            _exit(127);
        }

        jobs[free_slot].id = next_id++;
        jobs[free_slot].pid = pid;
        jobs[free_slot].active = 1;

        snprintf(jobs[free_slot].command,
                 sizeof(jobs[free_slot].command),
                 "%s", original);

        printf("[%d] Background PID=%ld\n",
               jobs[free_slot].id, (long)pid);

        return;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        execvp(args[0], args);
        perror(args[0]);
        _exit(127);
    }

    int status;
    pid_t result;

    do {
        result = waitpid(pid, &status, 0);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
        perror("waitpid");
    } else if (WIFEXITED(status) &&
               WEXITSTATUS(status) != 0) {
        printf("Command exited with status %d.\n",
               WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Command terminated by signal %d.\n",
               WTERMSIG(status));
    }
}

int main(void)
{
    char input[INPUT_SIZE];

    printf("Q21 - Integrated Shell\n");
    printf("Commands: pwd, cd DIRECTORY, jobs,\n");
    printf("          echo TEXT, sleep NUMBER, exit\n");
    printf("Add & for background execution.\n");

    while (1) {
        update_jobs();

        printf("\nq21> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        if (!strchr(input, '\n') && !feof(stdin)) {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            fprintf(stderr, "Error: Input too long.\n");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        execute_command(input);
    }

    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active)
            waitpid(jobs[i].pid, NULL, 0);
    }

    printf("Integrated shell exited.\n");
    return 0;
}
