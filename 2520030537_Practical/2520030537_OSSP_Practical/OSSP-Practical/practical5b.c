#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (pid1 == 0) {
        /* First child: execute ls -l */

        close(fd[0]);

        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(fd[1]);

        execlp("ls", "ls", "-l", (char *)NULL);

        perror("execlp ls");
        _exit(127);
    }

    pid_t pid2 = fork();

    if (pid2 < 0) {
        perror("fork");

        close(fd[0]);
        close(fd[1]);

        waitpid(pid1, NULL, 0);

        return 1;
    }

    if (pid2 == 0) {
        /* Second child: execute grep ".c" */

        close(fd[1]);

        if (dup2(fd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(fd[0]);

        execlp("grep", "grep", ".c", (char *)NULL);

        perror("execlp grep");
        _exit(127);
    }

    /* Parent closes the pipe and waits. */

    close(fd[0]);
    close(fd[1]);

    int status1;
    int status2;

    if (waitpid(pid1, &status1, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (waitpid(pid2, &status2, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    printf("\nPipeline execution completed.\n");

    if (WIFEXITED(status1) &&
        WIFEXITED(status2) &&
        WEXITSTATUS(status1) == 0 &&
        WEXITSTATUS(status2) == 0) {
        printf("Both commands executed successfully.\n");
        return 0;
    }

    printf("One or more commands returned a nonzero status.\n");

    return 1;
}
