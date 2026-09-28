#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_initialized = 100;
int global_uninitialized;

void display(void)
{
    printf("Function executed successfully.\n");
}

int main(void)
{
    static int static_variable = 50;
    int stack_variable = 25;

    int *heap_variable = malloc(sizeof(int));

    if (heap_variable == NULL) {
        perror("malloc");
        return 1;
    }

    *heap_variable = 75;

    printf("PRACTICAL 7A: PROCESS MEMORY LAYOUT\n");
    printf("-----------------------------------\n");

    printf("Process ID: %ld\n\n", (long)getpid());

    printf("Code segment       : %p\n", (void *)display);
    printf("Initialized global : %p\n",
           (void *)&global_initialized);
    printf("Uninitialized global: %p\n",
           (void *)&global_uninitialized);
    printf("Static variable    : %p\n",
           (void *)&static_variable);
    printf("Heap variable      : %p\n",
           (void *)heap_variable);
    printf("Stack variable     : %p\n",
           (void *)&stack_variable);

    printf("\nValues:\n");
    printf("Global = %d\n", global_initialized);
    printf("Static = %d\n", static_variable);
    printf("Heap   = %d\n", *heap_variable);
    printf("Stack  = %d\n", stack_variable);

    display();

    free(heap_variable);

    return 0;
}
