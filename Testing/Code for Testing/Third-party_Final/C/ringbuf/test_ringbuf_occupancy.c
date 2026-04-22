/*
 * test_ringbuf_occupancy.c
 * Tests pointer subtraction for occupancy calculation.
 * Pattern:
 *   if (head >= tail) free = capacity - (head - tail)
 *   else              free = tail - head - 1
 * Also: used = capacity - free
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 8

struct ringbuf_t {
    uint8_t *buf;
    uint8_t *head;
    uint8_t *tail;
    size_t   size;  /* internal size = capacity + 1 */
};

static size_t ringbuf_buffer_size(const struct ringbuf_t *rb)
{
    return rb->size;
}

static size_t ringbuf_capacity(const struct ringbuf_t *rb)
{
    return ringbuf_buffer_size(rb) - 1;
}

static size_t ringbuf_bytes_free(const struct ringbuf_t *rb)
{
    if (rb->head >= rb->tail)
        return ringbuf_capacity(rb) - (rb->head - rb->tail);
    else
        return rb->tail - rb->head - 1;
}

static size_t ringbuf_bytes_used(const struct ringbuf_t *rb)
{
    return ringbuf_capacity(rb) - ringbuf_bytes_free(rb);
}

int main(void)
{
    uint8_t storage[CAPACITY + 1];
    struct ringbuf_t rb;
    rb.buf  = storage;
    rb.size = CAPACITY + 1;

    /* Case 1: empty (head == tail == buf) */
    rb.head = storage;
    rb.tail = storage;
    printf("empty: free=%d used=%d\n",
           (int)ringbuf_bytes_free(&rb),
           (int)ringbuf_bytes_used(&rb));

    /* Case 2: head advanced 3 bytes, tail at 0 */
    rb.head = storage + 3;
    rb.tail = storage;
    printf("head+3 tail+0: free=%d used=%d\n",
           (int)ringbuf_bytes_free(&rb),
           (int)ringbuf_bytes_used(&rb));

    /* Case 3: head == tail+capacity (full) */
    /* Full: head is one step ahead of tail (wrapping) */
    rb.head = storage + 5;
    rb.tail = storage + 6;  /* tail one ahead of head => full */
    printf("full (tail=head+1): free=%d used=%d\n",
           (int)ringbuf_bytes_free(&rb),
           (int)ringbuf_bytes_used(&rb));

    /* Case 4: wrapped — tail > head */
    rb.head = storage + 2;
    rb.tail = storage + 6;
    printf("wrapped head+2 tail+6: free=%d used=%d\n",
           (int)ringbuf_bytes_free(&rb),
           (int)ringbuf_bytes_used(&rb));

    /* Case 5: pointer subtraction gives exact used count */
    rb.head = storage + 7;
    rb.tail = storage + 0;
    size_t used = (size_t)(rb.head - rb.tail);
    printf("subtraction head-tail=%d\n", (int)used);

    return 0;
}
