#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define INPUT_SIZE 200

int main()
{
    char input[INPUT_SIZE];
    int saved_stdout;
    int saved_stderr;
    int fd;

    printf("Combined Redirection Program\n");
    printf("Enter: test > output.txt 2>&1\n");
    printf("Type exit to quit.\n");

    while (1)
    {
        printf("combined> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        char *redirect = strstr(input, "2>&1");

        if (redirect == NULL)
        {
            printf("Error: Combined redirection '2>&1' not found.\n");
            continue;
        }

        *redirect = '\0';

        char *output_redirect = strchr(input, '>');

        if (output_redirect == NULL)
        {
            printf("Error: Output redirection '>' not found.\n");
            continue;
        }

        *output_redirect = '\0';

        char *command = input;
        char *filename = output_redirect + 1;

        while (*command == ' ')
            command++;

        while (*filename == ' ')
            filename++;

        if (*command == '\0')
        {
            printf("Error: Command is missing.\n");
            continue;
        }

        if (*filename == '\0')
        {
            printf("Error: Output file name is missing.\n");
            continue;
        }

        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd == -1)
        {
            perror("open");
            printf("Error: Cannot open output file: %s\n", filename);
            continue;
        }

        saved_stdout = dup(STDOUT_FILENO);
        saved_stderr = dup(STDERR_FILENO);

        if (saved_stdout == -1 || saved_stderr == -1)
        {
            perror("dup");
            close(fd);
            continue;
        }

        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("dup2 stdout");
            close(fd);
            close(saved_stdout);
            close(saved_stderr);
            continue;
        }

        /*
         * stderr is duplicated from stdout.
         * Therefore stdout and stderr now use the same file.
         */
        if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
        {
            perror("dup2 stderr");
            close(fd);
            close(saved_stdout);
            close(saved_stderr);
            continue;
        }

        close(fd);

        printf("Command output: %s\n", command);
        fprintf(stderr, "Error output: simulated error message\n");

        fflush(stdout);
        fflush(stderr);

        if (dup2(saved_stdout, STDOUT_FILENO) == -1)
        {
            perror("restore stdout");
        }

        if (dup2(saved_stderr, STDERR_FILENO) == -1)
        {
            perror("restore stderr");
        }

        close(saved_stdout);
        close(saved_stderr);

        printf("Both stdout and stderr were redirected.\n");
        printf("Streams restored successfully.\n");
    }

    printf("Program exited successfully.\n");

    return 0;
}

