#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INPUT_SIZE 200
#define MAX_ARGS 20

typedef void (*builtin_func)(char **);

void builtin_pwd(char **args)
{
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s\n", cwd);
    else
        perror("pwd");
}

void builtin_cd(char **args)
{
    if (args[1] == NULL)
    {
        printf("cd: missing argument\n");
        return;
    }

    if (chdir(args[1]) != 0)
        perror("cd");
}

void builtin_help(char **args)
{
    printf("Built-in commands:\n");
    printf("  pwd   - show current directory\n");
    printf("  cd    - change directory\n");
    printf("  help  - show available commands\n");
    printf("  exit  - exit the shell\n");
}

struct Builtin
{
    char *name;
    builtin_func function;
};

struct Builtin builtins[] =
{
    {"pwd", builtin_pwd},
    {"cd", builtin_cd},
    {"help", builtin_help}
};

int builtin_count = sizeof(builtins) / sizeof(builtins[0]);

int main()
{
    char input[INPUT_SIZE];
    char *args[MAX_ARGS];
    int running = 1;

    printf("Built-in Dispatch Shell\n");

    while (running)
    {
        printf("dispatch> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
            continue;

        int argc = 0;

        char *token = strtok(input, " \t");

        while (token != NULL && argc < MAX_ARGS - 1)
        {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }

        args[argc] = NULL;

        if (strcmp(args[0], "exit") == 0)
        {
            running = 0;
            continue;
        }

        int found = 0;

        for (int i = 0; i < builtin_count; i++)
        {
            if (strcmp(args[0], builtins[i].name) == 0)
            {
                builtins[i].function(args);
                found = 1;
                break;
            }
        }

        if (!found)
        {
            printf("Invalid command: %s\n", args[0]);
        }
    }

    printf("Dispatch shell exited.\n");

    return 0;
}
