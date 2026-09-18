#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        printf("Child process started\n");
        printf("Child PID: %d\n", getpid());

        sleep(2);

        printf("Child process completed\n");
        exit(0);
    }
    else
    {
        printf("Parent process started\n");
        printf("Parent PID: %d\n", getpid());

        printf("Parent waiting for child...\n");

        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            printf("Child exited normally\n");
            printf("Child exit status: %d\n",
                   WEXITSTATUS(status));
        }

        printf("Parent process completed\n");
    }

    return 0;
}
