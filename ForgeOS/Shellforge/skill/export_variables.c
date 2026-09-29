#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

#define INPUT_SIZE 200

int valid_name(const char *name)
{
    if (name == NULL || name[0] == '\0')
    {
        return 0;
    }

    if (!isalpha((unsigned char)name[0]) && name[0] != '_')
    {
        return 0;
    }

    for (int i = 1; name[i] != '\0'; i++)
    {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_')
        {
            return 0;
        }
    }

    return 1;
}

void process_export(char *input)
{
    char *name;
    char *value;
    char *equals;

    if (strncmp(input, "export ", 7) != 0)
    {
        printf("Invalid command.\n");
        return;
    }

    char *assignment = input + 7;

    equals = strchr(assignment, '=');

    if (equals == NULL)
    {
        printf("Error: Export syntax must be NAME=VALUE.\n");
        return;
    }

    *equals = '\0';

    name = assignment;
    value = equals + 1;

    if (!valid_name(name))
    {
        printf("Error: Invalid variable name: %s\n", name);
        return;
    }

    if (setenv(name, value, 1) != 0)
    {
        perror("setenv");
        return;
    }

    printf("Environment variable updated.\n");
    printf("%s=%s\n", name, value);
}

int main()
{
    char input[INPUT_SIZE];

    printf("Export Environment Shell\n");

    while (1)
    {
        printf("export> ");
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

        if (strcmp(input, "show") == 0)
        {
            char *path = getenv("PATH");

            if (path != NULL)
            {
                printf("PATH=%s\n", path);
            }

            continue;
        }

        process_export(input);
    }

    printf("Export shell exited.\n");

    return 0;
}
