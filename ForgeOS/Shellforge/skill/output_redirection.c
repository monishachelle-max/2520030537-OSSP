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
    int fd;

    printf("Output Redirection Program\n");
    printf("Enter a command in the form: echo Hello > output.txt\n");
    printf("Type exit to quit.\n");

    while (1)
    {
        printf("redirect> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        char *redirect = strchr(input, '>');

        if (redirect == NULL)
        {
            printf("Error: Output redirection '>' not found.\n");
            continue;
        }

        *redirect = '\0';

        char *command = input;
        char *filename = redirect + 1;

        while (*filename == ' ')
            filename++;

        if (*filename == '\0')
        {
            printf("Error: Output file name is missing.\n");
            continue;
        }

        while (*command == ' ')
            command++;

        if (*command == '\0')
        {
            printf("Error: Command is missing.\n");
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

        if (saved_stdout == -1)
        {
            perror("dup");
            close(fd);
            continue;
        }

        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            close(saved_stdout);
            continue;
        }

        close(fd);

        if (strncmp(command, "echo ", 5) == 0)
        {
            printf("%s\n", command + 5);
        }
        else
        {
            printf("%s\n", command);
        }

        fflush(stdout);

        if (dup2(saved_stdout, STDOUT_FILENO) == -1)
        {
            perror("dup2 restore");
        }

        close(saved_stdout);

        printf("Output redirected to: %s\n", filename);
        printf("Output stream restored successfully.\n");
    }

    printf("Program exited successfully.\n");

    return 0;
}
