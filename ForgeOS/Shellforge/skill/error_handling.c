#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_SIZE 256
#define MAX_ARGS 20

static FILE *log_file;

static void report_error(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);

    if (log_file) {
        va_start(args, format);
        fprintf(log_file, "ERROR: ");
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        va_end(args);

        fflush(log_file);
    }
}

static int parse_command(char *input, char *args[])
{
    int count = 0;
    char *token = strtok(input, " \t");

    while (token) {
        if (count >= MAX_ARGS - 1) {
            report_error("Too many arguments.");
            return -1;
        }

        if (strcmp(token, "|") == 0 ||
            strcmp(token, "<") == 0 ||
            strcmp(token, ">") == 0 ||
            strcmp(token, "&") == 0) {
            report_error("Unsupported syntax: %s", token);
            return -1;
        }

        args[count++] = token;
        token = strtok(NULL, " \t");
    }

    args[count] = NULL;
    return count;
}

static void run_command(char *args[])
{
    pid_t pid = fork();

    if (pid == -1) {
        report_error("fork failed: %s", strerror(errno));
        return;
    }

    if (pid == 0) {
        execvp(args[0], args);

        fprintf(stderr, "ERROR: %s: %s\n",
                args[0], strerror(errno));

        _exit(127);
    }

    int status;
    pid_t result;

    do {
        result = waitpid(pid, &status, 0);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
        report_error("waitpid failed: %s",
                     strerror(errno));
    } else if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);

        if (exit_code != 0)
            report_error("Command '%s' failed with exit status %d.",
                         args[0], exit_code);
        else
            printf("Command executed successfully.\n");
    } else if (WIFSIGNALED(status)) {
        report_error("Command '%s' terminated by signal %d.",
                     args[0], WTERMSIG(status));
    }
}

int main(void)
{
    char input[INPUT_SIZE];
    char *args[MAX_ARGS];

    log_file = fopen("q21_errors.log", "a");

    if (!log_file) {
        perror("Cannot open log file");
        return 1;
    }

    printf("Q21 - Runtime Error Handling\n");
    printf("Commands: pwd, ls, echo TEXT, cat FILE, exit\n");
    printf("Failures are logged in q21_errors.log.\n");

    while (1) {
        printf("\nerror> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            if (ferror(stdin))
                report_error("Input failed: %s",
                             strerror(errno));
            break;
        }

        if (!strchr(input, '\n') && !feof(stdin)) {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            report_error("Input exceeds %d characters.",
                         INPUT_SIZE - 2);
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (input[0] == '\0') {
            report_error("Empty command.");
            continue;
        }

        int count = parse_command(input, args);

        if (count < 0)
            continue;

        if (count == 0) {
            report_error("Missing command.");
            continue;
        }

        run_command(args);
    }

    if (fclose(log_file) == EOF)
        perror("Closing log file");

    printf("Error handling program exited normally.\n");
    return 0;
}
