#include <stdio.h>
#include <stdlib.h>

int global = 10;
int bss_variable;

int main() {

    int local = 20;

    int *heap = malloc(sizeof(int));
    *heap = 30;

    printf("Code address  : %p\n", (void *)main);
    printf("Data address  : %p\n", (void *)&global);
    printf("BSS address   : %p\n", (void *)&bss_variable);
    printf("Heap address  : %p\n", (void *)heap);
    printf("Stack address : %p\n", (void *)&local);

    free(heap);

    return 0;
}
