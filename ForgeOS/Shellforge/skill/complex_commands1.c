#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SIZE 300

/* Remove leading and trailing whitespace */
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

    printf("Complex Commands Program\n");
    printf("Supported command:\n");
    printf("cat < input.txt | grep WORD > output.txt\n\n");
    printf("Enter command: ");
    fflush(stdout);

    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        printf("Error: Unable to read command.\n");
        return 1;
    }

    input[strcspn(input, "\n")] = '\0';

    /* Identify redirection and pipe operators */
    char *in_op = strchr(input, '<');
    char *pipe_op = strchr(input, '|');
    char *out_op = strchr(input, '>');

    /* Validate the required operators and their order */
    if (in_op == NULL ||
        pipe_op == NULL ||
        out_op == NULL ||
        !(in_op < pipe_op && pipe_op < out_op))
    {
        printf("Error: Invalid command syntax.\n");
        printf("Expected: cat < input.txt | grep WORD > output.txt\n");
        return 1;
    }

    /* Separate the command into sections */
    *in_op = '\0';
    *pipe_op = '\0';
    *out_op = '\0';

    char *first_command = trim(input);
    char *input_file = trim(in_op + 1);
    char *second_command = trim(pipe_op + 1);
    char *output_file = trim(out_op + 1);

    /* Validate the first command */
    if (strcmp(first_command, "cat") != 0)
    {
        printf("Error: First command must be cat.\n");
        return 1;
    }

    /* Validate the second command */
    if (strncmp(second_command, "grep ", 5) != 0)
    {
        printf("Error: Second command must be grep WORD.\n");
        return 1;
    }

    char *search_word = trim(second_command + 5);

    /* Check for missing filenames or search words */
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

    /* Open the input file */
    int input_fd = open(input_file, O_RDONLY);

    if (input_fd == -1)
    {
        perror("Input file");
        return 1;
    }

    /* Open or create the output file */
    int output_fd = open(
        output_file,
        O_WRONLY | O_CREAT | O_TRUNC,
        0644
    );

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

    /* Create the pipe */
    int pipefd[2];

    if (pipe(pipefd) == -1)
    {
        perror("pipe");

        close(input_fd);
        close(output_fd);

        return 1;
    }

    /* Create the first child process */
    pid_t pid1 = fork();

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
        /* Redirect stdin from the input file */
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            perror("dup2 input");
            _exit(1);
        }

        /* Redirect stdout to the pipe */
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2 pipe output");
            _exit(1);
        }

        close(input_fd);
        close(output_fd);
        close(pipefd[0]);
        close(pipefd[1]);

        /* Execute cat */
        execlp("cat", "cat", (char *)NULL);

        perror("cat");
        _exit(1);
    }

    /* Create the second child process */
    pid_t pid2 = fork();

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
        /* Redirect stdin from the pipe */
        if (dup2(pipefd[0], STDIN_FILENO) == -1)
        {
            perror("dup2 pipe input");
            _exit(1);
        }

        /* Redirect stdout to the output file */
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2 output file");
            _exit(1);
        }

        close(input_fd);
        close(output_fd);
        close(pipefd[0]);
        close(pipefd[1]);

        /* Execute grep */
        execlp(
            "grep",
            "grep",
            search_word,
            (char *)NULL
        );

        perror("grep");
        _exit(1);
    }

    /* Parent closes all file descriptors */
    close(input_fd);
    close(output_fd);
    close(pipefd[0]);
    close(pipefd[1]);

    /* Wait for both child processes */
    int status1;
    int status2;

    if (waitpid(pid1, &status1, 0) == -1)
    {
        perror("waitpid cat");
        return 1;
    }

    if (waitpid(pid2, &status2, 0) == -1)
    {
        perror("waitpid grep");
        return 1;
    }

    /* Verify execution results */
    if (!WIFEXITED(status1) ||
        WEXITSTATUS(status1) != 0)
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
        printf("\nPipeline completed.\n");
        printf("No matching lines found.\n");
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
