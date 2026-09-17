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
        printf("Launching command: echo Hello OSSP\n");

        execlp("echo", "echo", "Hello OSSP", NULL);

        perror("Command execution failed");
        exit(1);
    }
    else
    {
        printf("Parent process waiting for child...\n");

        waitpid(pid, NULL, 0);

        printf("Command execution completed\n");
    }

    return 0;
}
