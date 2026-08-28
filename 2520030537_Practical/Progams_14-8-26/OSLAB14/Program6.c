#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Before exec()\n");

    // execlp replaces the current process with "ls -l"
    execlp("ls", "ls", "-l", NULL);

    // This line will not execute if execlp succeeds
    printf("This will not execute.\n");

    return 0;
}
