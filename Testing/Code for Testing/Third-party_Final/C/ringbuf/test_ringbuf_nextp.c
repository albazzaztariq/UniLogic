/*
 * test_ringbuf_nextp.c
 * Tests pointer arithmetic in ringbuf_nextp:
 *   return rb->buf + ((++p - rb->buf) % ringbuf_buffer_size(rb));
 * This combines: pre-increment of pointer, pointer subtraction to get offset,
 * modulus for wrap, and pointer-plus-offset to re-anchor.
 * Also tests the output-pointer-nulling pattern (*rb = 0) from ringbuf_free.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 4

struct ringbuf_t {
    uint8_t *buf;
    uint8_t *head;
    uint8_t *tail;
    size_t   size;
};

static size_t ringbuf_buffer_size(const struct ringbuf_t *rb)
{
    return rb->size;
}

static const uint8_t *ringbuf_end(const struct ringbuf_t *rb)
{
    return rb->buf + ringbuf_buffer_size(rb);
}

/*
 * Return next logical pointer after p in the ring buffer.
 * Wraps around using modulus: buf + ((++p - buf) % size)
 */
static uint8_t *ringbuf_nextp(struct ringbuf_t *rb, const uint8_t *p)
{
    return rb->buf + ((++p - rb->buf) % ringbuf_buffer_size(rb));
}

/*
 * Output-pointer-nulling: ringbuf_free sets *rb = 0.
 */
static void ringbuf_free_sim(struct ringbuf_t **rb)
{
    free((*rb)->buf);
    free(*rb);
    *rb = 0;
}

int main(void)
{
    uint8_t storage[CAPACITY + 1];
    struct ringbuf_t rb;
    rb.buf  = storage;
    rb.head = storage;
    rb.tail = storage;
    rb.size = CAPACITY + 1;

    const uint8_t *end = ringbuf_end(&rb);
    printf("buf=%p size=%d end=%p\n",
           (void *)rb.buf, (int)rb.size, (void *)end);

    /* Walk through every position and confirm nextp wraps correctly */
    size_t i;
    for (i = 0; i < rb.size; i++) {
        const uint8_t *p = rb.buf + i;
        /* Only valid to call nextp on positions [buf, end) */
        if (p < end) {
            uint8_t *next = ringbuf_nextp(&rb, p);
            int expected_offset = (int)((i + 1) % rb.size);
            int actual_offset   = (int)(next - rb.buf);
            printf("nextp(offset=%d) -> offset=%d (expected=%d) %s\n",
                   (int)i, actual_offset, expected_offset,
                   actual_offset == expected_offset ? "OK" : "FAIL");
        }
    }

    /* Test output-pointer-nulling via ringbuf_free_sim */
    struct ringbuf_t *rbp = (struct ringbuf_t *)malloc(sizeof(struct ringbuf_t));
    rbp->size = CAPACITY + 1;
    rbp->buf  = (uint8_t *)malloc(rbp->size);
    rbp->head = rbp->buf;
    rbp->tail = rbp->buf;

    printf("before free: rbp=%s\n", rbp ? "non-null" : "null");
    ringbuf_free_sim(&rbp);
    printf("after free: rbp=%s\n", rbp == 0 ? "null" : "non-null");

    return 0;
}
