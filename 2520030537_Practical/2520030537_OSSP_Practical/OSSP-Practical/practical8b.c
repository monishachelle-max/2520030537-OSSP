#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define SIZE (20 * 1024 * 1024)

void show_memory(const char *label)
{
    char path[100];
    char line[256];

    snprintf(path, sizeof(path),
             "/proc/%ld/status", (long)getpid());

    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        perror("fopen");
        return;
    }

    printf("\n%s (PID=%ld)\n", label, (long)getpid());

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "VmRSS:", 6) == 0 ||
            strncmp(line, "VmSize:", 7) == 0)
            printf("%s", line);
    }

    fclose(fp);
}

int main(void)
{
    char *data = malloc(SIZE);

    if (data == NULL) {
        perror("malloc");
        return 1;
    }

    /* Touch each page to allocate physical memory. */
    for (size_t i = 0; i < SIZE; i += 4096)
        data[i] = 'A';

    printf("PRACTICAL 8B: COPY-ON-WRITE\n");

    show_memory("Parent before fork");

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        free(data);
        return 1;
    }

    if (pid == 0) {
        show_memory("Child before modification");

        /* Modify every page to trigger Copy-on-Write. */
        for (size_t i = 0; i < SIZE; i += 4096)
            data[i] = 'B';

        show_memory("Child after modification");

        printf("\nChild data[0] = %c\n", data[0]);

        fflush(stdout);
        free(data);
        _exit(0);
    }

    waitpid(pid, NULL, 0);

    printf("\nParent data[0] = %c\n", data[0]);

    show_memory("Parent after child modification");

    free(data);

    return 0;
}
