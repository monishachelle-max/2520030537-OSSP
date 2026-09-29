#include <stdio.h>
#include <string.h>

#define MAX_INPUT 200
#define MAX_OUTPUT 200

void parse_single_quotes(char *input, char *output)
{
    int i = 0;
    int j = 0;
    int in_single_quote = 0;

    while (input[i] != '\0' && j < MAX_OUTPUT - 1)
    {
        if (input[i] == '\'')
        {
            in_single_quote = !in_single_quote;
            i++;
        }
        else
        {
            output[j++] = input[i++];
        }
    }

    output[j] = '\0';

    if (in_single_quote)
    {
        printf("Error: Unmatched single quote.\n");
    }
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

    parse_single_quotes(input, output);

    printf("\nParsed output: [%s]\n", output);

    return 0;
}

