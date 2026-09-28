#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_data = 100;

int main(void)
{
    static int static_data = 200;
    int stack_data = 300;

    char *heap_data = malloc(1024 * 1024);

    if (heap_data == NULL) {
        perror("malloc");
        return 1;
    }

    heap_data[0] = 'A';

    printf("PRACTICAL 7B: VIRTUAL MEMORY MAPPING\n");
    printf("------------------------------------\n");

    printf("Process PID: %ld\n", (long)getpid());

    printf("Global address: %p\n", (void *)&global_data);
    printf("Static address: %p\n", (void *)&static_data);
    printf("Heap address  : %p\n", (void *)heap_data);
    printf("Stack address : %p\n", (void *)&stack_data);

    printf("\nProcess is running for 60 seconds.\n");
    printf("Examine /proc/%ld/maps now.\n", (long)getpid());

    fflush(stdout);

    sleep(60);

    free(heap_data);

    printf("Memory released. Program completed.\n");

    return 0;
}
