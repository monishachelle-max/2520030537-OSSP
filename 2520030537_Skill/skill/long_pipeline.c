#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int pipe1[2];
    int pipe2[2];
    int pipe3[2];

    pid_t pid1, pid2, pid3, pid4;

    if (pipe(pipe1) == -1 ||
        pipe(pipe2) == -1 ||
        pipe(pipe3) == -1)
    {
        perror("pipe");
        return 1;
    }

    printf("Three pipes created successfully.\n");

    pid1 = fork();

    if (pid1 == 0)
    {
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(pipe3[0]);
        close(pipe3[1]);

        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);

        execlp("ls", "ls", NULL);

        perror("ls");
        exit(1);
    }

    pid2 = fork();

    if (pid2 == 0)
    {
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe3[0]);
        close(pipe3[1]);

        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);

        close(pipe1[0]);
        close(pipe2[1]);

        execlp("sort", "sort", NULL);

        perror("sort");
        exit(1);
    }

    pid3 = fork();

    if (pid3 == 0)
    {
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);
        close(pipe3[0]);

        dup2(pipe2[0], STDIN_FILENO);
        dup2(pipe3[1], STDOUT_FILENO);

        close(pipe2[0]);
        close(pipe3[1]);

        execlp("grep", "grep", ".c", NULL);

        perror("grep");
        exit(1);
    }

    pid4 = fork();

    if (pid4 == 0)
    {
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(pipe3[1]);

        dup2(pipe3[0], STDIN_FILENO);
        close(pipe3[0]);

        execlp("wc", "wc", "-l", NULL);

        perror("wc");
        exit(1);
    }

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);
    close(pipe3[0]);
    close(pipe3[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);
    waitpid(pid4, NULL, 0);

    printf("Long pipeline completed successfully.\n");
    printf("All descriptors closed and resources cleaned up.\n");

    return 0;
}
