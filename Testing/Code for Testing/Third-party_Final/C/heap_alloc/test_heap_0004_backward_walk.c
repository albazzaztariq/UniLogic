/* test_heap_0004_backward_walk.c
   Tests: backward pointer walk (prev pointer chain), remove_node,
          struct pointer through prev chain, NULL-terminated list.
   Extracted from llist.c::remove_node and the heap_free coalescing logic. */

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

void add_node(bin_t *bin, node_t *node) {
    node->next = NULL;
    node->prev = NULL;
    if (bin->head == NULL) { bin->head = node; return; }
    node_t *current = bin->head;
    node_t *previous = NULL;
    while (current != NULL && current->size <= node->size) {
        previous = current;
        current = current->next;
    }
    if (current == NULL) {
        previous->next = node; node->prev = previous;
    } else {
        if (previous != NULL) {
            node->next = current; previous->next = node;
            node->prev = previous; current->prev = node;
        } else {
            node->next = bin->head; bin->head->prev = node; bin->head = node;
        }
    }
}

void remove_node(bin_t *bin, node_t *node) {
    if (bin->head == NULL) return;
    if (bin->head == node) { bin->head = bin->head->next; return; }
    node_t *temp = bin->head->next;
    while (temp != NULL) {
        if (temp == node) {
            if (temp->next == NULL) {
                temp->prev->next = NULL;
            } else {
                temp->prev->next = temp->next;
                temp->next->prev = temp->prev;
            }
            return;
        }
        temp = temp->next;
    }
}

/* Walk backward from last node using prev pointers */
void print_backward(bin_t *bin) {
    if (bin->head == NULL) { printf("(empty)\n"); return; }
    /* Find last */
    node_t *cur = bin->head;
    while (cur->next != NULL) cur = cur->next;
    /* Walk back */
    printf("backward:");
    while (cur != NULL) {
        printf(" %u", cur->size);
        cur = cur->prev;
    }
    printf("\n");
}

void print_forward(bin_t *bin) {
    printf("forward:");
    node_t *cur = bin->head;
    while (cur != NULL) {
        printf(" %u", cur->size);
        cur = cur->next;
    }
    printf("\n");
}

int main(void) {
    node_t n1, n2, n3, n4, n5;
    bin_t bin; bin.head = NULL;

    n1.size=16;  n1.hole=1;
    n2.size=32;  n2.hole=1;
    n3.size=64;  n3.hole=1;
    n4.size=128; n4.hole=1;
    n5.size=256; n5.hole=1;

    add_node(&bin, &n3);
    add_node(&bin, &n1);
    add_node(&bin, &n5);
    add_node(&bin, &n2);
    add_node(&bin, &n4);

    printf("Initial state:\n");
    print_forward(&bin);
    print_backward(&bin);

    /* Remove head */
    printf("\nAfter remove head (size=16):\n");
    remove_node(&bin, &n1);
    print_forward(&bin);
    print_backward(&bin);

    /* Remove middle */
    printf("\nAfter remove middle (size=64):\n");
    remove_node(&bin, &n3);
    print_forward(&bin);
    print_backward(&bin);

    /* Remove tail */
    printf("\nAfter remove tail (size=256):\n");
    remove_node(&bin, &n5);
    print_forward(&bin);
    print_backward(&bin);

    /* Verify prev pointers: walk back and check each size */
    printf("\nPrev-pointer chain verification:\n");
    node_t *cur = bin.head;
    node_t *prev_saved = NULL;
    while (cur != NULL) {
        printf("  node size=%u prev_size=%s\n",
               cur->size,
               cur->prev ? "ok" : "NULL");
        prev_saved = cur;
        cur = cur->next;
    }

    return 0;
}
