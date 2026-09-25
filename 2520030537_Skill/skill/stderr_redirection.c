#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define INPUT_SIZE 200

int main()
{
    char input[INPUT_SIZE];
    int saved_stderr;
    int fd;

    printf("stderr Redirection Program\n");
    printf("Enter a command in the form: ls missing_file 2> error.txt\n");
    printf("Type exit to quit.\n");

    while (1)
    {
        printf("stderr> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        char *redirect = strstr(input, "2>");

        if (redirect == NULL)
        {
            printf("Error: stderr redirection operator '2>' not found.\n");
            continue;
        }

        *redirect = '\0';

        char *command = input;
        char *filename = redirect + 2;

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
            printf("Error: Error output file name is missing.\n");
            continue;
        }

        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd == -1)
        {
            perror("open");
            printf("Error: Cannot open error output file: %s\n", filename);
            continue;
        }

        saved_stderr = dup(STDERR_FILENO);

        if (saved_stderr == -1)
        {
            perror("dup");
            close(fd);
            continue;
        }

        if (dup2(fd, STDERR_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            close(saved_stderr);
            continue;
        }

        close(fd);

        if (strncmp(command, "ls ", 3) == 0)
        {
            char *filename_to_open = command + 3;

            FILE *file = fopen(filename_to_open, "r");

            if (file == NULL)
            {
                perror("ls");
            }
            else
            {
                printf("File exists: %s\n", filename_to_open);
                fclose(file);
            }
        }
        else
        {
            fprintf(stderr, "Command failed: %s\n", command);
        }

        fflush(stderr);

        if (dup2(saved_stderr, STDERR_FILENO) == -1)
        {
            perror("dup2 restore");
        }

        close(saved_stderr);

        printf("Error output redirected to: %s\n", filename);
        printf("stderr stream restored successfully.\n");
    }

    printf("Program exited successfully.\n");

    return 0;
}
