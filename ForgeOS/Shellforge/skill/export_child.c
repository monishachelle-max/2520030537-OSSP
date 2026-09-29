#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;

    setenv("NAME", "Monisha", 1);

    printf("Parent process\n");
    printf("NAME=%s\n", getenv("NAME"));

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        printf("Child process created\n");
        printf("Child received NAME=%s\n", getenv("NAME"));

        exit(0);
    }
    else
    {
        waitpid(pid, NULL, 0);
        printf("Parent process completed child test\n");
    }

    return 0;
}
