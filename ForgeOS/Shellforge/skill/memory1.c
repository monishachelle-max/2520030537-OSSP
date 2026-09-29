#include <stdio.h>
#include <stdlib.h>

int main() {
    int size = 5;

    int *buffer = (int *)malloc(size * sizeof(int));

    if (buffer == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < size; i++) {
        buffer[i] = i + 1;
    }

    printf("Original buffer size: %d\n", size);

    size = 10;

    int *temp = (int *)realloc(buffer, size * sizeof(int));

    if (temp == NULL) {
        printf("Memory reallocation failed.\n");
        free(buffer);
        return 1;
    }

    buffer = temp;

    for (int i = 5; i < size; i++) {
        buffer[i] = i + 1;
    }

    printf("Resized buffer size: %d\n", size);

    printf("Buffer contents: ");

    for (int i = 0; i < size; i++) {
        printf("%d ", buffer[i]);
    }

    printf("\n");

    free(buffer);

    return 0;
}
