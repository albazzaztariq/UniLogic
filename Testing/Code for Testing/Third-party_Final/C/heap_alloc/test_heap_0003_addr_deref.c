/* test_heap_0003_addr_deref.c
   Tests: address-of (&), pointer dereference (*), pointer-to-struct,
          cast to pointer type, assigning through dereferenced pointer.
   Derived from create_foot / get_foot patterns in heap.c. */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef unsigned int uint;

typedef struct node_t {
    uint hole;
    uint size;
    struct node_t *next;
    struct node_t *prev;
} node_t;

typedef struct {
    node_t *header;
} footer_t;

/* Place a footer immediately after the node's data region */
footer_t *get_foot(node_t *node) {
    return (footer_t *)((char *)node + sizeof(node_t) + node->size);
}

void create_foot(node_t *head) {
    footer_t *foot = get_foot(head);
    foot->header = head;
}

int main(void) {
    /* Allocate a raw buffer large enough for node + data + footer */
    char buf[512];
    memset(buf, 0, sizeof(buf));

    node_t *n = (node_t *)buf;
    n->hole = 1;
    n->size = 64;
    n->next = NULL;
    n->prev = NULL;

    create_foot(n);

    footer_t *f = get_foot(n);
    printf("foot->header == n: %d\n", f->header == n);

    /* Verify address arithmetic: foot should be at buf + sizeof(node_t) + 64 */
    size_t expected_offset = sizeof(node_t) + 64;
    size_t actual_offset   = (size_t)((char *)f - buf);
    printf("expected_offset=%zu actual_offset=%zu match=%d\n",
           expected_offset, actual_offset,
           expected_offset == actual_offset);

    /* Pointer round-trip: go forward to foot, come back via header */
    node_t *recovered = f->header;
    printf("recovered->size=%u recovered->hole=%u\n",
           recovered->size, recovered->hole);

    /* Test address-of scalar and deref */
    uint x = 42;
    uint *px = &x;
    printf("x=%u *px=%u\n", x, *px);
    *px = 99;
    printf("after *px=99: x=%u\n", x);

    /* Pointer comparison */
    printf("px == &x: %d\n", px == &x);

    return 0;
}
