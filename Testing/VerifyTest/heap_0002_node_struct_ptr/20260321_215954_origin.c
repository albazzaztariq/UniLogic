/* test_heap_0002_node_struct_ptr.c
   Tests: struct definition, struct field access through pointer (->),
          linked-list traversal, NULL checks, pointer assignment.
   Extracted from llist.c: add_node, get_best_fit, get_last_node logic,
   using stack-allocated nodes (no malloc). */

#include <stdio.h>
#include <stddef.h>

typedef unsigned int uint;

typedef struct node_t {
    uint hole;
    uint size;
    struct node_t *next;
    struct node_t *prev;
} node_t;

typedef struct {
    node_t *head;
} bin_t;

/* Insert in sorted order by size */
void add_node(bin_t *bin, node_t *node) {
    node->next = NULL;
    node->prev = NULL;

    if (bin->head == NULL) {
        bin->head = node;
        return;
    }

    node_t *current = bin->head;
    node_t *previous = NULL;

    while (current != NULL && current->size <= node->size) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        previous->next = node;
        node->prev = previous;
    } else {
        if (previous != NULL) {
            node->next = current;
            previous->next = node;
            node->prev = previous;
            current->prev = node;
        } else {
            node->next = bin->head;
            bin->head->prev = node;
            bin->head = node;
        }
    }
}

node_t *get_best_fit(bin_t *bin, size_t size) {
    if (bin->head == NULL) return NULL;
    node_t *temp = bin->head;
    while (temp != NULL) {
        if (temp->size >= size) return temp;
        temp = temp->next;
    }
    return NULL;
}

node_t *get_last_node(bin_t *bin) {
    node_t *temp = bin->head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    return temp;
}

int main(void) {
    /* Build 4 stack nodes, insert into a bin, then query */
    node_t n1, n2, n3, n4;
    bin_t  bin;

    bin.head = NULL;

    n1.hole = 0; n1.size = 32;  n1.next = NULL; n1.prev = NULL;
    n2.hole = 0; n2.size = 8;   n2.next = NULL; n2.prev = NULL;
    n3.hole = 0; n3.size = 128; n3.next = NULL; n3.prev = NULL;
    n4.hole = 0; n4.size = 64;  n4.next = NULL; n4.prev = NULL;

    add_node(&bin, &n1);
    add_node(&bin, &n2);
    add_node(&bin, &n3);
    add_node(&bin, &n4);

    /* Print sorted order */
    printf("Sorted list (ascending size):\n");
    node_t *cur = bin.head;
    while (cur != NULL) {
        printf("  size=%u hole=%u\n", cur->size, cur->hole);
        cur = cur->next;
    }

    /* Best fit queries */
    node_t *fit;
    fit = get_best_fit(&bin, 10);
    printf("best_fit(10): size=%u\n", fit ? fit->size : 0);
    fit = get_best_fit(&bin, 64);
    printf("best_fit(64): size=%u\n", fit ? fit->size : 0);
    fit = get_best_fit(&bin, 200);
    printf("best_fit(200): %s\n", fit ? "found" : "NULL");

    /* Last node */
    node_t *last = get_last_node(&bin);
    printf("last_node: size=%u\n", last->size);

    return 0;
}
