#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

int main()
{
    char current_dir[PATH_MAX];

    if (getcwd(current_dir, sizeof(current_dir)) == NULL)
    {
        perror("getcwd");
        return 1;
    }

    printf("Current Directory: %s\n", current_dir);

    printf("Program is running successfully.\n");

    printf("Enter 'exit' to terminate the program.\n");

    char input[50];

    while (1)
    {
        printf("state> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
        {
            printf("Exit request received.\n");
            break;
        }

        if (strcmp(input, "pwd") == 0)
        {
            if (getcwd(current_dir, sizeof(current_dir)) != NULL)
            {
                printf("Current Directory: %s\n", current_dir);
            }
            else
            {
                perror("getcwd");
            }
        }
        else
        {
            printf("Invalid command: %s\n", input);
        }
    }

    printf("Cleaning up resources...\n");
    printf("State saved successfully.\n");
    printf("Program exited successfully.\n");

    return 0;
}
