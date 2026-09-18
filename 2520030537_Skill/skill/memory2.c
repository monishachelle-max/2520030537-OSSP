#include <stdio.h>
#include <stdlib.h>

int main() {
    int size = 5;

    int *buffer = malloc(size * sizeof(int));

    if (buffer == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    int count = 0;

    for (int i = 1; i <= 7; i++) {

        if (count >= size) {
            printf("Buffer full. Resizing buffer...\n");

            int new_size = size * 2;

            int *temp = realloc(buffer, new_size * sizeof(int));

            if (temp == NULL) {
                printf("Memory reallocation failed.\n");
                free(buffer);
                return 1;
            }

            buffer = temp;
            size = new_size;
        }

        buffer[count] = i;
        count++;
    }

    printf("Final buffer size: %d\n", size);
    printf("Number of elements: %d\n", count);

    printf("Buffer contents: ");

    for (int i = 0; i < count; i++) {
        printf("%d ", buffer[i]);
    }

    printf("\n");

    free(buffer);

    return 0;
}
