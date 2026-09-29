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
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        printf("Child process created\n");
        printf("Child PID: %d\n", getpid());

        /* Intentionally using an invalid command */
        execlp("invalid_command", "invalid_command", NULL);

        /* This executes only if execlp() fails */
        perror("Execution failed");
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
