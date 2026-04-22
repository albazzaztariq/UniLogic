/* test_heap_0005_ptr_arithmetic.c
   Tests: char* pointer arithmetic with sizeof, pointer cast chains,
          uintptr_t cast, multi-level pointer offset calculation.
   Derived from heap.c: get_foot, get_wilderness, heap_alloc split logic.
   Self-contained: uses a static buffer instead of real heap. */

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

footer_t *get_foot(node_t *node) {
    return (footer_t *)((char *)node + sizeof(node_t) + node->size);
}

void create_foot(node_t *head) {
    footer_t *foot = get_foot(head);
    foot->header = head;
}

int main(void) {
    /* Layout in buf:
       [node_t A | 32 bytes data | footer_t A | node_t B | 16 bytes data | footer_t B]
    */
    char buf[1024];
    memset(buf, 0, sizeof(buf));

    /* Sizes must satisfy: a->size > alloc_size + sizeof(node_t) + sizeof(footer_t)
       sizeof(node_t) on x64 = 32, sizeof(footer_t) = 8 => overhead = 40.
       Use a->size = 128 so split->size = 128 - 8 - 32 - 8 = 80 (positive). */

    /* --- Node A --- */
    node_t *a = (node_t *)buf;
    a->hole = 1;
    a->size = 128;
    a->next = NULL;
    a->prev = NULL;
    create_foot(a);

    /* --- Node B sits immediately after A's footer --- */
    size_t stride_a = sizeof(node_t) + a->size + sizeof(footer_t);
    node_t *b = (node_t *)(buf + stride_a);
    b->hole = 1;
    b->size = 16;
    b->next = NULL;
    b->prev = NULL;
    create_foot(b);

    /* Verify get_foot for A */
    footer_t *fa = get_foot(a);
    printf("fa->header == a: %d\n", fa->header == a);

    /* Verify get_foot for B */
    footer_t *fb = get_foot(b);
    printf("fb->header == b: %d\n", fb->header == b);

    /* Simulate get_wilderness: last footer is at (buf_end - sizeof(footer_t)) */
    char *heap_end = buf + stride_a + sizeof(node_t) + b->size + sizeof(footer_t);
    footer_t *wild_foot = (footer_t *)(heap_end - sizeof(footer_t));
    node_t   *wild = wild_foot->header;
    printf("wild == b: %d\n", wild == b);
    printf("wild->size=%u wild->hole=%u\n", wild->size, wild->hole);

    /* Simulate split: from node A, split off a smaller node after size=8 */
    uint alloc_size = 8;
    node_t *split = (node_t *)(((char *)a + sizeof(node_t) + sizeof(footer_t)) + alloc_size);
    split->size = a->size - alloc_size - (uint)sizeof(node_t) - (uint)sizeof(footer_t);
    split->hole = 1;
    split->next = NULL;
    split->prev = NULL;
    a->size = alloc_size;
    create_foot(a);
    create_foot(split);

    printf("after split: a->size=%u split->size=%u\n", a->size, split->size);
    footer_t *fa2 = get_foot(a);
    footer_t *fs  = get_foot(split);
    printf("fa2->header == a: %d\n", fa2->header == a);
    printf("fs->header == split: %d\n", fs->header == split);

    /* Pointer difference / offset check */
    ptrdiff_t offset_a_to_split = (char *)split - (char *)a;
    size_t expected = sizeof(node_t) + sizeof(footer_t) + alloc_size;
    printf("offset_a_to_split=%td expected=%zu match=%d\n",
           offset_a_to_split, expected,
           (size_t)offset_a_to_split == expected);

    /* uintptr_t cast round-trip */
    uintptr_t addr = (uintptr_t)a;
    node_t *back   = (node_t *)addr;
    printf("uintptr_t round-trip: back==a: %d back->size=%u\n",
           back == a, back->size);

    return 0;
}
