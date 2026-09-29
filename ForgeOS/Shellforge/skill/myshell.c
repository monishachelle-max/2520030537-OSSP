#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 64

int main(void)
{
    char *line = NULL;
    size_t capacity = 0;

    printf("================================\n");
    printf(" Welcome to 2520030537 SHELLFORGE\n");
    printf("================================\n");

    while (1) {
        printf("2520030537_SHELLFORGE$ ");
        fflush(stdout);

        if (getline(&line, &capacity, stdin) == -1) {
            printf("\n");
            break;
        }

        char *args[MAX_ARGS];
        int count = 0;

        char *token = strtok(line, " \t\n");

        while (token != NULL && count < MAX_ARGS - 1) {
            args[count++] = token;
            token = strtok(NULL, " \t\n");
        }

        args[count] = NULL;

        if (count == 0)
            continue;

        if (strcmp(args[0], "exit") == 0) {
            printf("Exiting SHELLFORGE...\n");
            break;
        }

        if (strcmp(args[0], "help") == 0) {
            printf("Supported commands:\n");
            printf("  ls, pwd, date, cal, ps\n");
            printf("  gcc, cat, mkdir and other Linux commands\n");
            printf("  cd <directory> - change directory\n");
            printf("  ./program       - run a compiled program\n");
            printf("  exit            - leave the shell\n");
            continue;
        }

        if (strcmp(args[0], "cd") == 0) {
            const char *path =
                count > 1 ? args[1] : getenv("HOME");

            if (path == NULL || chdir(path) != 0)
                perror("cd");

            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            execvp(args[0], args);
            perror("Command failed");
            _exit(127);
        }

        int status;

        if (waitpid(pid, &status, 0) == -1)
            perror("waitpid");
    }

    free(line);
    return 0;
}
