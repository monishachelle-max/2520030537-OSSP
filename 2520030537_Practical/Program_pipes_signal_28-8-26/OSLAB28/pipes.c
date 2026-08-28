#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main()
{
    int fd[2];
    pid_t pid;
    char message[] = "Hello from Parent";
    char buffer[100];

    pipe(fd);

    pid = fork();

    if (pid == 0)
    {
        // Child
        close(fd[1]);

        read(fd[0], buffer, sizeof(buffer));

        printf("Child received: %s\n", buffer);

        close(fd[0]);
    }
    else
    {
        // Parent
        close(fd[0]);

        write(fd[1], message, strlen(message) + 1);

        close(fd[1]);

        wait(NULL);
    }

    return 0;
}
