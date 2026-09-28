#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#define SIZE (10 * 1024 * 1024)
#define BUFFER_SIZE 8192

double elapsed(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(void)
{
    struct timespec start, end;
    char *message = "OSSP Practical 10 - mmap Test\n";

    /* Create a 10 MB test file. */
    int fd = open("practical10_mmap.dat",
                  O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (ftruncate(fd, SIZE) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    /* Memory-mapped I/O */
    clock_gettime(CLOCK_MONOTONIC, &start);

    char *mapped = mmap(NULL, SIZE,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    memcpy(mapped, message, strlen(message));

    printf("mmap read: %s", mapped);

    if (msync(mapped, SIZE, MS_SYNC) == -1)
        perror("msync");

    if (munmap(mapped, SIZE) == -1)
        perror("munmap");

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("mmap time: %.6f seconds\n",
           elapsed(start, end));

    close(fd);

    /* Traditional read/write I/O */
    fd = open("practical10_mmap.dat", O_RDONLY);

    if (fd == -1) {
        perror("open input");
        return 1;
    }

    int out = open("practical10_copy.dat",
                   O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (out == -1) {
        perror("open output");
        close(fd);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t n;

    clock_gettime(CLOCK_MONOTONIC, &start);

    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        ssize_t written = 0;

        while (written < n) {
            ssize_t result =
                write(out, buffer + written,
                      (size_t)(n - written));

            if (result == -1) {
                if (errno == EINTR)
                    continue;

                perror("write");
                close(fd);
                close(out);
                return 1;
            }

            if (result == 0) {
                fprintf(stderr, "Write made no progress.\n");
                close(fd);
                close(out);
                return 1;
            }

            written += result;
        }
    }

    if (n == -1) {
        perror("read");
        close(fd);
        close(out);
        return 1;
    }

    close(fd);
    close(out);

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("read/write copy time: %.6f seconds\n",
           elapsed(start, end));

    printf("Both methods executed successfully.\n");

    return 0;
}
