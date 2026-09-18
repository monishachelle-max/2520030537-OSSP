#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 300
#define MAX_OUTPUT 600

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
                   (isalnum((unsigned char)input[i]) ||
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
            else
            {
                printf("Undefined variable: $%s\n", variable);
            }
        }
        else
        {
            output[j++] = input[i++];
        }
    }

    output[j] = '\0';
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

    expand_variables(input, output);

    printf("\nExpanded output: [%s]\n", output);

    return 0;
}
