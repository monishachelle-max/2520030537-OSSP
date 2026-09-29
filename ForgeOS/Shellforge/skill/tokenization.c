#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 200
#define MAX_TOKENS 50
#define MAX_TOKEN_LENGTH 100

struct Token
{
    char value[MAX_TOKEN_LENGTH];
};

void tokenize(char *input, struct Token tokens[], int *count)
{
    int i = 0;
    int j;

    *count = 0;

    while (input[i] != '\0')
    {
        /* Skip whitespace */
        while (isspace((unsigned char)input[i]))
        {
            i++;
        }

        if (input[i] == '\0')
        {
            break;
        }

        j = 0;

        /* Read one token */
        while (input[i] != '\0' &&
               !isspace((unsigned char)input[i]))
        {
            if (j < MAX_TOKEN_LENGTH - 1)
            {
                tokens[*count].value[j++] = input[i];
            }

            i++;
        }

        tokens[*count].value[j] = '\0';

        (*count)++;

        if (*count >= MAX_TOKENS)
        {
            break;
        }
    }
}

void display_tokens(struct Token tokens[], int count)
{
    printf("\nTokens:\n");

    for (int i = 0; i < count; i++)
    {
        printf("Token %d: [%s]\n",
               i + 1,
               tokens[i].value);
    }
}

int validate_tokens(struct Token tokens[], int count)
{
    if (count == 0)
    {
        printf("Error: Empty input.\n");
        return 0;
    }

    for (int i = 0; i < count; i++)
    {
        if (strlen(tokens[i].value) == 0)
        {
            printf("Error: Empty token found.\n");
            return 0;
        }
    }

    return 1;
}

int main()
{
    char input[MAX_INPUT];

    struct Token tokens[MAX_TOKENS];

    int count;

    printf("Enter input: ");

    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        return 1;
    }

    input[strcspn(input, "\n")] = '\0';

    tokenize(input, tokens, &count);

    display_tokens(tokens, count);

    if (validate_tokens(tokens, count))
    {
        printf("\nToken stream is valid.\n");
    }
    else
    {
        printf("\nToken stream is invalid.\n");
    }

    return 0;
}
