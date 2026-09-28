#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        int input_fd = open("practical9_input.txt", O_RDONLY);

        if (input_fd == -1) {
            perror("open input");
            _exit(1);
        }

        int output_fd = open("practical9_output.txt",
                             O_WRONLY | O_CREAT | O_TRUNC,
                             0644);

        if (output_fd == -1) {
            perror("open output");
            close(input_fd);
            _exit(1);
        }

        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("dup2 stdin");
            _exit(1);
        }

        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("dup2 stdout");
            _exit(1);
        }

        close(input_fd);
        close(output_fd);

        /* Equivalent to: cat < input.txt > output.txt */
        execlp("cat", "cat", (char *)NULL);

        perror("execlp");
        _exit(127);
    }

    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Input and output redirection successful.\n");
        printf("Output saved to practical9_output.txt\n");
        return 0;
    }

    printf("Redirection failed.\n");
    return 1;
}
