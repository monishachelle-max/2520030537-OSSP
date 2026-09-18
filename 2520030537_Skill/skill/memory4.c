#include <stdio.h>
#include <stdlib.h>

struct Node {
    int data;
    struct Node *next;
};

int main() {

    struct Node *head = NULL;
    struct Node *second = NULL;
    struct Node *third = NULL;

    /* Allocate memory for three nodes */
    head = malloc(sizeof(struct Node));
    second = malloc(sizeof(struct Node));
    third = malloc(sizeof(struct Node));

    /* Check memory allocation */
    if (head == NULL || second == NULL || third == NULL) {
        printf("Memory allocation failed.\n");

        free(head);
        free(second);
        free(third);

        return 1;
    }

    /* Store data in the nodes */
    head->data = 10;
    head->next = second;

    second->data = 20;
    second->next = third;

    third->data = 30;
    third->next = NULL;

    /* Display the linked list */
    printf("Linked List: ");

    struct Node *current = head;

    while (current != NULL) {
        printf("%d ", current->data);
        current = current->next;
    }

    printf("\n");

    /* Release dynamically allocated memory */
    free(head);
    free(second);
    free(third);

    printf("Memory released successfully.\n");

    return 0;
}
