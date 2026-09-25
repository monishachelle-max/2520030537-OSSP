#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pid_t pid;

    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    printf("Pipe created successfully.\n");

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        close(fd[1]);

        printf("Child process waiting for data...\n");

        char buffer[100];

        ssize_t bytes_read = read(fd[0], buffer, sizeof(buffer) - 1);

        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            printf("Child received: %s\n", buffer);
        }

        close(fd[0]);

        exit(0);
    }
    else
    {
        close(fd[0]);

        printf("Parent process sending data...\n");

        char message[] = "Data transferred through pipe";

        write(fd[1], message, sizeof(message));

        close(fd[1]);

        waitpid(pid, NULL, 0);

        printf("Parent synchronized with child.\n");
        printf("Data flow completed successfully.\n");
    }

    return 0;
}

