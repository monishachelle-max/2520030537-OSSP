#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int pipe1[2];
    int pipe2[2];

    pid_t pid1;
    pid_t pid2;
    pid_t pid3;

    if (pipe(pipe1) == -1)
    {
        perror("pipe1");
        return 1;
    }

    if (pipe(pipe2) == -1)
    {
        perror("pipe2");
        return 1;
    }

    printf("Two pipes created successfully.\n");

    pid1 = fork();

    if (pid1 < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid1 == 0)
    {
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]);

        dup2(pipe1[1], STDOUT_FILENO);

        close(pipe1[1]);

        execlp("ls", "ls", NULL);

        perror("execlp ls");
        exit(1);
    }

    pid2 = fork();

    if (pid2 < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid2 == 0)
    {
        close(pipe1[1]);
        close(pipe2[0]);

        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);

        close(pipe1[0]);
        close(pipe2[1]);

        execlp("sort", "sort", NULL);

        perror("execlp sort");
        exit(1);
    }

    pid3 = fork();

    if (pid3 < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid3 == 0)
    {
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);

        dup2(pipe2[0], STDIN_FILENO);

        close(pipe2[0]);

        execlp("wc", "wc", "-l", NULL);

        perror("execlp wc");
        exit(1);
    }

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    printf("All pipeline processes completed.\n");
    printf("Pipeline resources cleaned up successfully.\n");

    return 0;
}
