#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *a, *b;

    a = (int*)malloc(5 * sizeof(int));

    b = (int*)calloc(5, sizeof(int));

    a = (int*)realloc(a, 10 * sizeof(int));

    free(a);
    free(b);

    return 0;
}
