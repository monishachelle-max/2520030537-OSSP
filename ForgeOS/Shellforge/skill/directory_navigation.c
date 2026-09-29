#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define INPUT_SIZE 200

int main()
{
    char input[INPUT_SIZE];
    char previous_dir[PATH_MAX];
    char current_dir[PATH_MAX];

    if (getcwd(current_dir, sizeof(current_dir)) == NULL)
    {
        perror("getcwd");
        return 1;
    }

    strcpy(previous_dir, current_dir);

    while (1)
    {
        printf("nav> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
        {
            break;
        }

        if (strcmp(input, "pwd") == 0)
        {
            if (getcwd(current_dir, sizeof(current_dir)) != NULL)
            {
                printf("Current directory: %s\n", current_dir);
            }
            else
            {
                perror("getcwd");
            }

            continue;
        }

        if (strncmp(input, "cd ", 3) == 0)
        {
            char *path = input + 3;

            if (strlen(path) == 0)
            {
                printf("Error: Path is empty.\n");
                continue;
            }

            if (getcwd(current_dir, sizeof(current_dir)) == NULL)
            {
                perror("getcwd");
                continue;
            }

            strcpy(previous_dir, current_dir);

            if (chdir(path) != 0)
            {
                perror("cd");
                continue;
            }

            if (getcwd(current_dir, sizeof(current_dir)) != NULL)
            {
                printf("Directory changed to: %s\n", current_dir);
            }

            continue;
        }

        if (strcmp(input, "cd -") == 0)
        {
            char temp[PATH_MAX];

            if (getcwd(current_dir, sizeof(current_dir)) == NULL)
            {
                perror("getcwd");
                continue;
            }

            strcpy(temp, current_dir);

            if (chdir(previous_dir) != 0)
            {
                perror("cd");
                continue;
            }

            strcpy(previous_dir, temp);

            if (getcwd(current_dir, sizeof(current_dir)) != NULL)
            {
                printf("Changed to previous directory: %s\n",
                       current_dir);
            }

            continue;
        }

        printf("Invalid command: %s\n", input);
    }

    printf("Navigation program exited.\n");

    return 0;
}
