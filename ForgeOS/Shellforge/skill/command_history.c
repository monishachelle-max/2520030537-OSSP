#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HISTORY 5
#define MAX_COMMAND 100

char history[MAX_HISTORY][MAX_COMMAND];
int history_count = 0;

void store_command(const char *command)
{
    if (history_count < MAX_HISTORY)
    {
        strcpy(history[history_count], command);
        history_count++;
    }
    else
    {
        for (int i = 1; i < MAX_HISTORY; i++)
        {
            strcpy(history[i - 1], history[i]);
        }

        strcpy(history[MAX_HISTORY - 1], command);
    }
}

void display_history()
{
    printf("\nCommand History:\n");

    if (history_count == 0)
    {
        printf("History is empty.\n");
        return;
    }

    for (int i = 0; i < history_count; i++)
    {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

void retrieve_entry(int number)
{
    if (number < 1 || number > history_count)
    {
        printf("Error: Invalid history entry.\n");
        return;
    }

    printf("Retrieved command: %s\n", history[number - 1]);
}

int validate_history()
{
    for (int i = 0; i < history_count; i++)
    {
        if (strlen(history[i]) == 0)
        {
            return 0;
        }
    }

    return 1;
}

int main()
{
    char input[MAX_COMMAND];

    printf("Command History Manager\n");
    printf("Maximum history capacity: %d\n", MAX_HISTORY);

    while (1)
    {
        printf("\nhistory> ");
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

        if (strcmp(input, "history") == 0)
        {
            display_history();
            continue;
        }

        if (strncmp(input, "retrieve ", 9) == 0)
        {
            int number = atoi(input + 9);
            retrieve_entry(number);
            continue;
        }

        if (strlen(input) == 0)
        {
            printf("Error: Empty command.\n");
            continue;
        }

        store_command(input);
        printf("Command stored: %s\n", input);
    }

    printf("\nFinal History:\n");
    display_history();

    if (validate_history())
    {
        printf("History consistency: VALID\n");
    }
    else
    {
        printf("History consistency: INVALID\n");
    }

    printf("History manager exited.\n");

    return 0;
}
