#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 200
#define MAX_OUTPUT 400

void expand_variables(char *input, char *output)
{
    int i = 0;
    int j = 0;

    while (input[i] != '\0' && j < MAX_OUTPUT - 1)
    {
        if (input[i] == '$')
        {
            char variable[100];
            int k = 0;
            char *value;

            i++;

            while (input[i] != '\0' &&
                   ((input[i] >= 'A' && input[i] <= 'Z') ||
                    (input[i] >= 'a' && input[i] <= 'z') ||
                    (input[i] >= '0' && input[i] <= '9') ||
                    input[i] == '_'))
            {
                if (k < 99)
                {
                    variable[k++] = input[i];
                }

                i++;
            }

            variable[k] = '\0';

            value = getenv(variable);

            if (value != NULL)
            {
                int len = strlen(value);

                if (j + len < MAX_OUTPUT - 1)
                {
                    strcpy(&output[j], value);
                    j += len;
                }
            }
        }
        else
        {
            output[j++] = input[i++];
        }
    }

    output[j] = '\0';
}

void parse_double_quotes(char *input, char *output)
{
    int i = 0;
    int j = 0;
    int in_double_quote = 0;

    while (input[i] != '\0' && j < MAX_OUTPUT - 1)
    {
        if (input[i] == '"')
        {
            in_double_quote = !in_double_quote;
            i++;
        }
        else
        {
            output[j++] = input[i++];
        }
    }

    output[j] = '\0';

    if (in_double_quote)
    {
        printf("Error: Unmatched double quote.\n");
        return;
    }

    char expanded[MAX_OUTPUT];

    expand_variables(output, expanded);

    strcpy(output, expanded);
}

int main()
{
    char input[MAX_INPUT];
    char output[MAX_OUTPUT];

    printf("Enter command: ");

    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        return 1;
    }

    input[strcspn(input, "\n")] = '\0';

    parse_double_quotes(input, output);

    printf("\nParsed output: [%s]\n", output);

    return 0;
}
