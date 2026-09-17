#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 200
#define MAX_ARGS 20

void builtin_pwd()
{
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
    }
    else
    {
        perror("pwd");
    }
}

void builtin_cd(char *path)
{
    if (path == NULL)
    {
        fprintf(stderr, "cd: missing argument\n");
        return;
    }

    if (chdir(path) != 0)
    {
        perror("cd");
    }
}

void builtin_help()
{
    printf("Built-in commands:\n");
    printf("  pwd   - display current directory\n");
    printf("  cd    - change directory\n");
    printf("  help  - display built-in commands\n");
    printf("  exit  - exit the shell\n");
}

int main()
{
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    int argc;
    int running = 1;

    printf("Mini Shell Built-ins\n");
    printf("Type help to see available commands.\n");

    while (running)
    {
        printf("\nbuiltin> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
        {
            continue;
        }

        argc = 0;

        char *token = strtok(input, " \t");

        while (token != NULL && argc < MAX_ARGS - 1)
        {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }

        args[argc] = NULL;

        if (strcmp(args[0], "pwd") == 0)
        {
            builtin_pwd();
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            builtin_cd(args[1]);
        }
        else if (strcmp(args[0], "help") == 0)
        {
            builtin_help();
        }
        else if (strcmp(args[0], "exit") == 0)
        {
            running = 0;
        }
        else
        {
            printf("Invalid command: %s\n", args[0]);
        }
    }

    printf("Shell exited.\n");

    return 0;
}
