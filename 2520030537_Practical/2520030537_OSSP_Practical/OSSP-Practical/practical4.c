#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid1, pid2, pid3;

    printf("Parent process started.\n");
    printf("Parent PID: %d\n", getpid());

    pid1 = fork();

    if (pid1 == 0) {
        printf("Child 1: PID = %d, PPID = %d\n", getpid(), getppid());
        sleep(2);
        printf("Child 1 terminated.\n");
        return 10;
    }

    pid2 = fork();

    if (pid2 == 0) {
        printf("Child 2: PID = %d, PPID = %d\n", getpid(), getppid());
        sleep(4);
        printf("Child 2 terminated.\n");
        return 20;
    }

    pid3 = fork();

    if (pid3 == 0) {
        printf("Child 3: PID = %d, PPID = %d\n", getpid(), getppid());
        sleep(6);
        printf("Child 3 terminated.\n");
        return 30;
    }

    printf("\nParent is waiting for children...\n");

    wait(NULL);

    printf("One child completed using wait().\n");

    waitpid(pid2, NULL, 0);

    printf("Child 2 completed using waitpid().\n");

    waitpid(pid3, NULL, 0);

    printf("Child 3 completed using waitpid().\n");

    printf("Parent process terminating.\n");

    return 0;
}

