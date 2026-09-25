#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pid_t pid;
    char message[] = "Hello from parent process";
    char buffer[100];

    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    printf("Pipeline created successfully.\n");

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        close(fd[1]);

        read(fd[0], buffer, sizeof(buffer));

        printf("Child process received: %s\n", buffer);

        close(fd[0]);

        exit(0);
    }
    else
    {
        close(fd[0]);

        printf("Parent process sending: %s\n", message);

        write(fd[1], message, strlen(message) + 1);

        close(fd[1]);

        waitpid(pid, NULL, 0);

        printf("Parent process completed.\n");
    }

    return 0;
}
