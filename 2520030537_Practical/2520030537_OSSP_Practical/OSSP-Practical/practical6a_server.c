#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

#define SERVER_FIFO "/tmp/ossp_q6_server"
#define MAX_MSG 200

typedef struct {
    pid_t pid;
    char message[MAX_MSG];
} Request;

int main(void)
{
    Request req;
    char response_fifo[100];
    char response[MAX_MSG];

    if (mkfifo(SERVER_FIFO, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

    printf("FIFO server started.\n");
    printf("Waiting for clients...\n");
    fflush(stdout);

    int server_fd = open(SERVER_FIFO, O_RDWR);

    if (server_fd == -1) {
        perror("open");
        return 1;
    }

    while (1) {
        ssize_t n = read(server_fd, &req, sizeof(req));

        if (n == -1) {
            if (errno == EINTR)
                continue;
            perror("read");
            break;
        }

        if (n != sizeof(req))
            continue;

        printf("Received from client %ld: %s\n",
               (long)req.pid, req.message);
        fflush(stdout);

        snprintf(response, sizeof(response), "%s", req.message);

        for (int i = 0; response[i]; i++) {
            response[i] =
                (char)toupper((unsigned char)response[i]);
        }

        snprintf(response_fifo, sizeof(response_fifo),
                 "/tmp/ossp_q6_client_%ld", (long)req.pid);

        int client_fd = open(response_fifo, O_WRONLY);

        if (client_fd == -1) {
            perror("open client FIFO");
            continue;
        }

        if (write(client_fd, response, strlen(response) + 1) == -1)
            perror("write");

        close(client_fd);
    }

    close(server_fd);
    unlink(SERVER_FIFO);

    return 0;
}
