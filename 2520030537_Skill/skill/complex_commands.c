#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SIZE 300

/* Remove leading and trailing spaces */
char *trim(char *str)
{
    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return str;

    char *end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }

    return str;
}

int main()
{
    char input[SIZE];
    char *input_file;
    char *output_file;
    char *search_word;

    int pipefd[2];
    pid_t pid1, pid2;

    printf("Complex Commands Program\n");
    printf("Supported format:\n");
    printf("cat < input.txt | grep WORD > output.txt\n");

    printf("\nEnter command: ");
    fflush(stdout);

    if (fgets(input, sizeof(input), stdin) == NULL)
        return 1;

    input[strcspn(input, "\n")] = '\0';

    /* Locate the operators */
    char *in_op = strchr(input, '<');
    char *pipe_op = strchr(input, '|');
    char *out_op = strchr(input, '>');

    /* Validate operator presence and order */
    if (in_op == NULL || pipe_op == NULL || out_op == NULL ||
        !(in_op < pipe_op && pipe_op < out_op))
    {
        printf("Error: Invalid command syntax.\n");
        printf("Expected: cat < input.txt | grep WORD > output.txt\n");
        return 1;
    }

    /* Split command into sections */
    *in_op = '\0';
    *pipe_op = '\0';
    *out_op = '\0';

    char *first_command = trim(input);
    input_file = trim(in_op + 1);

    char *second_section = trim(pipe_op + 1);
    output_file = trim(out_op + 1);

    /* Validate the first command */
    if (strcmp(first_command, "cat") != 0)
    {
        printf("Error: First command must be cat.\n");
        return 1;
    }

    /* Validate the second command */
    if (strncmp(second_section, "grep ", 5) != 0)
    {
        printf("Error: Second command must be grep WORD.\n");
        return 1;
    }

    search_word = trim(second_section + 5);

    /* Validate filenames and search word */
    if (*input_file == '\0' ||
        *output_file == '\0' ||
        *search_word == '\0')
    {
        printf("Error: Missing filename or search word.\n");
        return 1;
    }

    /* Reject unsupported extra operators */
    if (strpbrk(input_file, "<>|") != NULL ||
        strpbrk(output_file, "<>|") != NULL ||
        strpbrk(search_word, "<>|") != NULL)
    {
        printf("Error: Unsupported extra redirection or pipe.\n");
        return 1;
    }

    /* Check the input file before creating the output file */
    int input_fd = open(input_file, O_RDONLY);

    if (input_fd == -1)
    {
        perror("Input file");
        return 1;
    }

    /* Create the output file */
    int output_fd = open(output_file,
                         O_WRONLY | O_CREAT | O_TRUNC,
                         0644);

    if (output_fd == -1)
    {
        perror("Output file");
        close(input_fd);
        return 1;
    }

    /* Display the execution plan */
    printf("\nExecution Plan:\n");
    printf("1. Open input file: %s\n", input_file);
    printf("2. Execute cat\n");
    printf("3. Send cat output through a pipe\n");
    printf("4. Execute grep with search word: %s\n", search_word);
    printf("5. Redirect grep output to: %s\n", output_file);

    fflush(stdout);

    /* Create a pipe */
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        close(input_fd);
        close(output_fd);
        return 1;
    }

    /* First child: cat */
    pid1 = fork();

    if (pid1 < 0)
    {
        perror("fork");
        close(input_fd);
        close(output_fd);
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    if (pid1 == 0)
    {
        /* Redirect input file to stdin */
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            perror("dup2 input");
            _exit(1);
        }

        /* Redirect stdout to the pipe */
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2 pipe");
            _exit(1);
        }

        close(input_fd);
        close(output_fd);
        close(pipefd[0]);
        close(pipefd[1]);

        execlp("cat", "cat", (char *)NULL);

        perror("cat");
        _exit(1);
    }

    /* Second child: grep */
    pid2 = fork();

    if (pid2 < 0)
    {
        perror("fork");

        close(input_fd);
        close(output_fd);
        close(pipefd[0]);
        close(pipefd[1]);

        waitpid(pid1, NULL, 0);
        return 1;
    }

    if (pid2 == 0)
    {
        /* Read input from the pipe */
        if (dup2(pipefd[0], STDIN_FILENO) == -1)
        {
            perror("dup2 pipe input");
            _exit(1);
        }

        /* Redirect output to the output file */
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2 output");
            _exit(1);
        }

        close(input_fd);
        close(output_fd);
        close(pipefd[0]);
        close(pipefd[1]);

        execlp("grep", "grep", search_word, (char *)NULL);

        perror("grep");
        _exit(1);
    }

    /* Parent closes unused descriptors */
    close(input_fd);
    close(output_fd);
    close(pipefd[0]);
    close(pipefd[1]);

    /* Wait for both children */
    int status1, status2;

    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    if (!WIFEXITED(status1) || WEXITSTATUS(status1) != 0)
    {
        printf("Error: cat failed.\n");
        return 1;
    }

    if (!WIFEXITED(status2))
    {
        printf("Error: grep failed.\n");
        return 1;
    }

    if (WEXITSTATUS(status2) == 1)
    {
        printf("Pipeline completed. No matching lines found.\n");
        return 0;
    }

    if (WEXITSTATUS(status2) != 0)
    {
        printf("Error: grep failed.\n");
        return 1;
    }

    printf("\nPipeline executed successfully.\n");
    printf("Output saved to: %s\n", output_file);

    return 0;
}
