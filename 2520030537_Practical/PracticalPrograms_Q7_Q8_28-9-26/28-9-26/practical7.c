#include <stdio.h>
#include <stdlib.h>

int global = 100;
static int s = 50;

int main()
{
    int stack = 10;
    int *heap = (int*)malloc(sizeof(int));

    printf("Code Segment   : %p\n", main);
    printf("Global Segment : %p\n", &global);
    printf("Static Segment : %p\n", &s);
    printf("Heap Segment   : %p\n", heap);
    printf("Stack Segment  : %p\n", &stack);

    free(heap);
    return 0;
}
