#include <stdio.h>
#include <unistd.h>

int main() {
    printf("My Process ID (PID) = %d\n", getpid());
    printf("My Parent Process ID (PPID) = %d\n", getppid());
    sleep(30); // keeps process alive for 30 seconds
    return 0;
}
