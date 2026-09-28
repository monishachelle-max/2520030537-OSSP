#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 8192

double elapsed(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

int copy_syscall(const char *source, const char *destination)
{
    char buffer[BUFFER_SIZE];

    int in = open(source, O_RDONLY);
    if (in == -1) {
        perror("open source");
        return -1;
    }

    int out = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1) {
        perror("open destination");
        close(in);
        return -1;
    }

    /* Demonstrate lseek by positioning at the file beginning. */
    if (lseek(in, 0, SEEK_SET) == -1) {
        perror("lseek");
        close(in);
        close(out);
        return -1;
    }

    ssize_t n;

    while ((n = read(in, buffer, sizeof(buffer))) > 0) {
        ssize_t written = 0;

        while (written < n) {
            ssize_t result =
                write(out, buffer + written, (size_t)(n - written));

            if (result == -1) {
                if (errno == EINTR)
                    continue;

                perror("write");
                close(in);
                close(out);
                return -1;
            }

            written += result;
        }
    }

    if (n == -1) {
        perror("read");
        close(in);
        close(out);
        return -1;
    }

    close(in);
    close(out);

    return 0;
}

int copy_stdio(const char *source, const char *destination)
{
    char buffer[BUFFER_SIZE];

    FILE *in = fopen(source, "rb");
    if (in == NULL) {
        perror("fopen source");
        return -1;
    }

 
   FILE *out = fopen(destination, "wb");
    if (out == NULL) {
        perror("fopen destination");
        fclose(in);
        return -1;
    }

    size_t n;

    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, n, out) != n) {
            perror("fwrite");
            fclose(in);
            fclose(out);
            return -1;
        }
    }

    if (ferror(in)) {
        perror("fread");
        fclose(in);
        fclose(out);
        return -1;
    }

    fclose(in);
    fclose(out);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s source_file\n", argv[0]);
        return 1;
    }

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    if (copy_syscall(argv[1], "copy_syscall.txt") != 0)
        return 1;

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("System call copy time: %.6f seconds\n",
           elapsed(start, end));

    clock_gettime(CLOCK_MONOTONIC, &start);

    if (copy_stdio(argv[1], "copy_stdio.txt") != 0)
        return 1;

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Standard library copy time: %.6f seconds\n",
           elapsed(start, end));

    printf("Both file copies completed successfully.\n");

    return 0;
}
