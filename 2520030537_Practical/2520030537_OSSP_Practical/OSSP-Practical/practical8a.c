#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *a, *b;

    /* malloc */
    a = malloc(5 * sizeof(int));

    if (a == NULL) {
        perror("malloc");
        return 1;
    }

    printf("malloc values: ");
    for (int i = 0; i < 5; i++) {
        a[i] = (i + 1) * 10;
        printf("%d ", a[i]);
    }

    /* calloc */
    b = calloc(5, sizeof(int));

    if (b == NULL) {
        perror("calloc");
        free(a);
        return 1;
    }

    printf("\ncalloc values: ");
    for (int i = 0; i < 5; i++)
        printf("%d ", b[i]);

    /* realloc */
    int *temp = realloc(a, 10 * sizeof(int));

    if (temp == NULL) {
        perror("realloc");
        free(a);
        free(b);
        return 1;
    }

    a = temp;

    printf("\nrealloc values: ");

    for (int i = 5; i < 10; i++)
        a[i] = (i + 1) * 10;

    for (int i = 0; i < 10; i++)
        printf("%d ", a[i]);

    free(a);
    free(b);

    printf("\nMemory released successfully.\n");

    return 0;
}
