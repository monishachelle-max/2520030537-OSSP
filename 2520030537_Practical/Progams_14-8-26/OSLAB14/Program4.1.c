#include <stdio.h>
#include <unistd.h>

int main() {
    for (int i = 1; i <= 3; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Child process
            printf("Child %d PID = %d\n", i, getpid());
            return 0; // child exits after printing
        }
    }
    return 0;
}
