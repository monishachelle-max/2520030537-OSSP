#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define INPUT_SIZE 200

int main()
{
    char input[INPUT_SIZE];
    int saved_stdin;
    int fd;

    printf("Input Redirection Program\n");
    printf("Enter a command in the form: cat < filename\n");
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

        char *redirect = strchr(input, '<');

        if (redirect == NULL)
        {
            printf("Error: Input redirection '<' not found.\n");
            continue;
        }

        *redirect = '\0';

        char *filename = redirect + 1;

        while (*filename == ' ')
            filename++;

        if (*filename == '\0')
        {
            printf("Error: Input file name is missing.\n");
            continue;
        }

        fd = open(filename, O_RDONLY);

        if (fd == -1)
        {
            perror("open");
            printf("Error: Cannot open input file: %s\n", filename);
            continue;
        }

        saved_stdin = dup(STDIN_FILENO);

        if (saved_stdin == -1)
        {
            perror("dup");
            close(fd);
            continue;
        }

        if (dup2(fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            close(saved_stdin);
            continue;
        }

        close(fd);

        printf("Input redirected from: %s\n", filename);

        char buffer[100];

        while (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            printf("Read: %s", buffer);
        }

        if (dup2(saved_stdin, STDIN_FILENO) == -1)
        {
            perror("dup2 restore");
        }

        close(saved_stdin);

        printf("\nInput stream restored successfully.\n");
    }

    printf("Program exited successfully.\n");

    return 0;
}
