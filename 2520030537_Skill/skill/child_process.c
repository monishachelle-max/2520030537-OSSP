#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        printf("Child process created\n");
        printf("Child PID: %d\n", getpid());

        execlp("ls", "ls", "-l", NULL);

        perror("exec");
        exit(1);
    }
    else
    {
        printf("Parent process\n");
        printf("Parent PID: %d\n", getpid());

        wait(NULL);

        printf("Child process completed\n");
    }

    return 0;
}
