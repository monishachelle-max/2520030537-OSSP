#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMANDS 10
#define MAX_LENGTH 100

struct Pipeline
{
    char commands[MAX_COMMANDS][MAX_LENGTH];
    int count;
};

void add_command(struct Pipeline *pipeline, const char *command)
{
    if (pipeline->count >= MAX_COMMANDS)
    {
        printf("Error: Pipeline capacity reached.\n");
        return;
    }

    strcpy(pipeline->commands[pipeline->count], command);
    pipeline->count++;

    printf("Command stored: %s\n", command);
}

void display_pipeline(struct Pipeline *pipeline)
{
    printf("\nPipeline Structure:\n");

    for (int i = 0; i < pipeline->count; i++)
    {
        printf("Process %d: %s\n",
               i + 1,
               pipeline->commands[i]);

        if (i < pipeline->count - 1)
        {
            printf("        |\n");
            printf("        v\n");
        }
    }
}

int validate_pipeline(struct Pipeline *pipeline)
{
    if (pipeline->count == 0)
    {
        printf("Error: Pipeline is empty.\n");
        return 0;
    }

    for (int i = 0; i < pipeline->count; i++)
    {
        if (strlen(pipeline->commands[i]) == 0)
        {
            printf("Error: Empty command at position %d.\n",
                   i + 1);
            return 0;
        }
    }

    return 1;
}

int main()
{
    struct Pipeline pipeline;
    char input[MAX_LENGTH];

    pipeline.count = 0;

    printf("Pipeline Structure Manager\n");
    printf("Enter commands one by one.\n");
    printf("Type 'done' when finished.\n");

    while (pipeline.count < MAX_COMMANDS)
    {
        printf("\npipeline> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "done") == 0)
        {
            break;
        }

        if (strlen(input) == 0)
        {
            printf("Error: Empty command.\n");
            continue;
        }

        add_command(&pipeline, input);
    }

    display_pipeline(&pipeline);

    if (validate_pipeline(&pipeline))
    {
        printf("\nPipeline layout is VALID.\n");
        printf("Execution order maintained successfully.\n");
    }
    else
    {
        printf("\nPipeline layout is INVALID.\n");
    }

    return 0;
}
