#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        printf("Fork failed.\n");
        return 1;
    }

    if (pid == 0) {
        printf("Child process: PID = %d\n", getpid());
        printf("Child terminating...\n");
        return 0;
    }

    else {
        printf("Parent process: PID = %d\n", getpid());
        printf("Child PID = %d\n", pid);

        printf("Parent waiting for child using wait()...\n");

        wait(NULL);

        printf("Child has been collected by parent.\n");
        printf("No zombie process remains.\n");

        printf("Parent process terminating.\n");
    }

    return 0;
}
