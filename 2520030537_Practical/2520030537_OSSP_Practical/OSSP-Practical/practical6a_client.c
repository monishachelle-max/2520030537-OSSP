#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define SERVER_FIFO "/tmp/ossp_q6_server"
#define MAX_MSG 200

typedef struct {
    pid_t pid;
    char message[MAX_MSG];
} Request;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"message\"\n", argv[0]);
        return 1;
    }

    Request req = {0};
    req.pid = getpid();
    snprintf(req.message, sizeof(req.message), "%s", argv[1]);

    char response_fifo[100];

    snprintf(response_fifo, sizeof(response_fifo),
             "/tmp/ossp_q6_client_%ld", (long)req.pid);

    if (mkfifo(response_fifo, 0600) == -1) {
        perror("mkfifo");
        return 1;
    }

    int server_fd = open(SERVER_FIFO, O_WRONLY);

    if (server_fd == -1) {
        perror("Server not running");
        unlink(response_fifo);
        return 1;
    }

    if (write(server_fd, &req, sizeof(req)) != sizeof(req)) {
        perror("write");
        close(server_fd);
        unlink(response_fifo);
        return 1;
    }

    close(server_fd);

    int client_fd = open(response_fifo, O_RDONLY);

    if (client_fd == -1) {
        perror("open");
        unlink(response_fifo);
        return 1;
    }

    char response[MAX_MSG];
    ssize_t n = read(client_fd, response, sizeof(response) - 1);

    if (n > 0) {
        response[n] = '\0';
        printf("Server response: %s\n", response);
    } else {
        printf("No response received.\n");
    }

    close(client_fd);
    unlink(response_fifo);

    return 0;
}
