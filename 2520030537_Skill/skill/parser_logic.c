#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 200
#define MAX_TOKENS 50
#define MAX_TOKEN_LENGTH 100

struct Node
{
    char value[MAX_TOKEN_LENGTH];
    struct Node *left;
    struct Node *right;
};

struct Node *create_node(const char *value)
{
    struct Node *node = malloc(sizeof(struct Node));

    if (node == NULL)
    {
        perror("malloc");
        exit(1);
    }

    strcpy(node->value, value);
    node->left = NULL;
    node->right = NULL;

    return node;
}

int tokenize(char *input, char tokens[][MAX_TOKEN_LENGTH])
{
    int count = 0;
    char *token;

    token = strtok(input, " \t\n");

    while (token != NULL && count < MAX_TOKENS)
    {
        strcpy(tokens[count], token);
        count++;

        token = strtok(NULL, " \t\n");
    }

    return count;
}

struct Node *build_parse_tree(char tokens[][MAX_TOKEN_LENGTH], int count)
{
    struct Node *root;

    if (count == 0)
    {
        return NULL;
    }

    root = create_node(tokens[0]);

    if (count > 1)
    {
        root->right = create_node(tokens[1]);
    }

    for (int i = 2; i < count; i++)
    {
        struct Node *new_node = create_node(tokens[i]);
        struct Node *current = root;

        while (current->right != NULL)
        {
            current = current->right;
        }

        current->right = new_node;
    }

    return root;
}

void display_tree(struct Node *root, int level)
{
    if (root == NULL)
    {
        return;
    }

    for (int i = 0; i < level; i++)
    {
        printf("  ");
    }

    printf("|-- %s\n", root->value);

    display_tree(root->left, level + 1);
    display_tree(root->right, level + 1);
}

int validate_syntax(char tokens[][MAX_TOKEN_LENGTH], int count)
{
    if (count == 0)
    {
        printf("Error: Empty command.\n");
        return 0;
    }

    if (strcmp(tokens[0], "|") == 0)
    {
        printf("Error: Command cannot start with '|'.\n");
        return 0;
    }

    if (strcmp(tokens[count - 1], "|") == 0)
    {
        printf("Error: Command cannot end with '|'.\n");
        return 0;
    }

    for (int i = 0; i < count - 1; i++)
    {
        if (strcmp(tokens[i], "|") == 0 &&
            strcmp(tokens[i + 1], "|") == 0)
        {
            printf("Error: Consecutive '|' operators.\n");
            return 0;
        }
    }

    return 1;
}

void free_tree(struct Node *root)
{
    if (root == NULL)
    {
        return;
    }

    free_tree(root->left);
    free_tree(root->right);

    free(root);
}

int main()
{
    char input[MAX_INPUT];
    char tokens[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int count;
    struct Node *root;

    printf("Enter command: ");

    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        return 1;
    }

    input[strcspn(input, "\n")] = '\0';

    count = tokenize(input, tokens);

    printf("\nTokens:\n");

    for (int i = 0; i < count; i++)
    {
        printf("Token %d: [%s]\n", i + 1, tokens[i]);
    }

    if (!validate_syntax(tokens, count))
    {
        printf("\nSyntax validation failed.\n");
        return 1;
    }

    root = build_parse_tree(tokens, count);

    printf("\nParse Tree:\n");
    display_tree(root, 0);

    printf("\nSyntax is valid.\n");
    printf("Execution structure generated successfully.\n");

    free_tree(root);

    return 0;
}
