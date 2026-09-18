#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        printf("Child Process Executing . . . \n");
        sleep(3);  // simulate some work
        printf("Child Finished\n");
    } else {
        // Parent process waits for child
        wait(NULL);
        printf("Parent Process Continues\n");
    }

    return 0;
}
