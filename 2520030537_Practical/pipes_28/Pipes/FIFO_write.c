#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd;
    char message[] = "Hello from Writer";

    fd = open("myfifo", O_WRONLY);

    write(fd, message, strlen(message) + 1);

    close(fd);

    printf("Message sent\n");

    return 0;
}
