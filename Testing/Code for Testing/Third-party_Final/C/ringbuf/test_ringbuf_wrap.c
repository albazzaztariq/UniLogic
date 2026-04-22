/*
 * test_ringbuf_wrap.c
 * Tests pointer wrap-around: advance head to end of buffer, wrap back to start.
 * Pattern: if (ptr == bufend) ptr = buf;
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

static const uint8_t *ringbuf_end(const struct ringbuf_t *rb)
{
    return rb->buf + rb->size;
}

/* Advance head by n bytes, wrapping at end of buffer. */
static void advance_head(struct ringbuf_t *rb, size_t n)
{
    size_t i;
    const uint8_t *bufend = ringbuf_end(rb);
    for (i = 0; i < n; i++) {
        rb->buf[rb->head - rb->buf] = (uint8_t)(i + 1);
        rb->head++;
        if (rb->head == bufend)
            rb->head = rb->buf;
    }
}

int main(void)
{
    uint8_t storage[CAPACITY + 1];
    struct ringbuf_t rb;
    rb.buf  = storage;
    rb.head = storage;
    rb.tail = storage;
    rb.size = CAPACITY + 1;

    const uint8_t *bufend = ringbuf_end(&rb);

    printf("buf=%p end=%p size=%d\n",
           (void *)rb.buf, (void *)bufend, (int)rb.size);

    /* Write CAPACITY bytes; head should wrap back to start on last step. */
    advance_head(&rb, CAPACITY);

    printf("after advance CAPACITY: head_offset=%d\n",
           (int)(rb.head - rb.buf));

    /* Write one more; head should wrap to 0 at wrap point, then advance to 1 */
    /* (We reset first so head is at end - 1) */
    rb.head = (uint8_t *)bufend - 1;
    rb.head++;
    if (rb.head == bufend)
        rb.head = rb.buf;

    printf("after manual wrap: head_offset=%d\n",
           (int)(rb.head - rb.buf));

    /* Verify tail pointer wrap too */
    rb.tail = (uint8_t *)bufend - 1;
    rb.tail++;
    if (rb.tail == bufend)
        rb.tail = rb.buf;

    printf("after tail wrap: tail_offset=%d\n",
           (int)(rb.tail - rb.buf));

    return 0;
}
