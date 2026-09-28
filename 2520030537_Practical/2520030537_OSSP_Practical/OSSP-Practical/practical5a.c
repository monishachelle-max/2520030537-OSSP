#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#define COUNT 100000

static double elapsed_seconds(struct timespec start,
                              struct timespec end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

/* Read exactly the requested number of bytes. */
static int read_all(int fd, void *buffer, size_t size)
{
    size_t received = 0;
    char *ptr = buffer;

    while (received < size) {
        ssize_t n = read(fd, ptr + received, size - received);

        if (n == 0)
            return 0;

        if (n == -1) {
            if (errno == EINTR)
                continue;

            perror("read");
            return -1;
        }

        received += (size_t)n;
    }

    return 1;
}

/* Write exactly the requested number of bytes. */
static int write_all(int fd, const void *buffer, size_t size)
{
    size_t sent = 0;
    const char *ptr = buffer;

    while (sent < size) {
        ssize_t n = write(fd, ptr + sent, size - sent);

        if (n == -1) {
            if (errno == EINTR)
                continue;

            perror("write");
            return -1;
        }

        if (n == 0)
            return -1;

        sent += (size_t)n;
    }

    return 0;
}

int main(void)
{
    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    struct timespec start, end;

    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        perror("clock_gettime");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (pid == 0) {
        /* Child: consumer */
        close(fd[1]);

        int value;
        int received = 0;
        long long sum = 0;

        while (received < COUNT) {
            int result = read_all(fd[0], &value, sizeof(value));

            if (result == 0)
                break;

            if (result == -1) {
                close(fd[0]);
                _exit(1);
            }

            sum += value;
            received++;
        }

        close(fd[0]);

        printf("CHILD PROCESS - CONSUMER\n");
        printf("Child PID: %ld\n", (long)getpid());
        printf("Child consumed %d integers.\n", received);
        printf("Sum of received data: %lld\n", sum);

        fflush(stdout);

        _exit(received == COUNT ? 0 : 1);
    }

    /* Parent: producer */
    close(fd[0]);

    int sent = 0;
    int write_failed = 0;

    for (int i = 1; i <= COUNT; i++) {
        if (write_all(fd[1], &i, sizeof(i)) == -1) {
            write_failed = 1;
            break;
        }

        sent++;
    }

    close(fd[1]);

    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
        perror("clock_gettime");
        return 1;
    }

    double seconds = elapsed_seconds(start, end);
    double bytes = (double)sent * sizeof(int);

    printf("\nPARENT PROCESS - PRODUCER\n");
    printf("Parent PID: %ld\n", (long)getpid());
    printf("Parent produced %d integers.\n", sent);
    printf("Data transferred: %.0f bytes\n", bytes);
    printf("Communication time: %.6f seconds\n", seconds);

    printf("Throughput: %.2f MB/s\n",
           seconds > 0 ? bytes / seconds / 1000000.0 : 0.0);

    if (write_failed ||
        !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Communication failed.\n");
        return 1;
    }

    printf("Communication completed successfully.\n");

    return 0;
}
