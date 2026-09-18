#include <stdio.h>
#include <stdlib.h>

int main() {
    int size = 5;

    int *buffer = (int *)malloc(size * sizeof(int));

    if (buffer == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    printf("Dynamic buffer allocated successfully.\n");

    free(buffer);

    return 0;
}
