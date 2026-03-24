/* Linked List
   Implements a singly linked list with insert, search, and print
   Concepts: structs, pointers, memory allocation, data structures */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A single node in the linked list */
typedef struct Node {
    int value;
    struct Node *next;
} Node;

/* The linked list with head pointer and size */
typedef struct {
    Node *head;
    int size;
} LinkedList;

/* Initialize an empty list */
void list_init(LinkedList *ll) {
    ll->head = NULL;
    ll->size = 0;
}

/* Insert a new node at the front */
void push_front(LinkedList *ll, int value) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->value = value;
    node->next = ll->head;
    ll->head = node;
    ll->size++;
}

/* Search for a value, return 1 if found */
int find(LinkedList *ll, int value) {
    Node *current = ll->head;
    while (current != NULL) {
        if (current->value == value) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

/* Print all elements: [a -> b -> c] */
void print_list(LinkedList *ll) {
    char result[256] = "[";
    Node *current = ll->head;
    int first = 1;
    while (current != NULL) {
        if (!first) {
            strcat(result, " -> ");
        }
        char num[16];
        sprintf(num, "%d", current->value);
        strcat(result, num);
        first = 0;
        current = current->next;
    }
    strcat(result, "]");
    printf("%s\n", result);
}

/* Free all nodes in the list */
void list_free(LinkedList *ll) {
    Node *current = ll->head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    ll->head = NULL;
    ll->size = 0;
}

int main(void) {
    printf("=== Linked List ===\n");
    printf("\n");

    /* Build a list by pushing to front */
    LinkedList ll;
    list_init(&ll);

    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        push_front(&ll, values[i]);
        printf("Pushed %d, size = %d\n", values[i], ll.size);
    }

    printf("\n");
    printf("List contents:\n");
    print_list(&ll);

    printf("\n");

    /* Search for values */
    int search_for[] = {30, 99, 10};
    for (int i = 0; i < 3; i++) {
        if (find(&ll, search_for[i])) {
            printf("Found %d in list\n", search_for[i]);
        } else {
            printf("%d not in list\n", search_for[i]);
        }
    }

    /* Clean up allocated memory */
    list_free(&ll);

    return 0;
}
