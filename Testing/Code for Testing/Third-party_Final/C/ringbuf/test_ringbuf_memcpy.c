/*
 * test_ringbuf_memcpy.c
 * Tests two-segment memcpy with wraparound.
 * Pattern (from ringbuf_memcpy_into / ringbuf_memcpy_from):
 *   while (nread != count) {
 *       n = MIN(bufend - head, count - nread);
 *       memcpy(head, src + nread, n);
 *       head += n;
 *       nread += n;
 *       if (head == bufend) head = buf;
 *   }
 * Also tests output-pointer-nulling on underflow:
 *   if (count > bytes_used) return 0;
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 8

#define MIN(a, b) ((a) < (b) ? (a) : (b))

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

static size_t ringbuf_capacity(const struct ringbuf_t *rb)
{
    return rb->size - 1;
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

/* Copy src[0..count) into ring buffer, wrapping at bufend. */
static void *rb_memcpy_into(struct ringbuf_t *dst, const void *src, size_t count)
{
    const uint8_t *u8src = (const uint8_t *)src;
    const uint8_t *bufend = ringbuf_end(dst);
    size_t nread = 0;

    while (nread != count) {
        size_t n = MIN((size_t)(bufend - dst->head), count - nread);
        memcpy(dst->head, u8src + nread, n);
        dst->head += n;
        nread += n;
        if (dst->head == bufend)
            dst->head = dst->buf;
    }
    return dst->head;
}

/* Copy count bytes out of ring buffer into dst, wrapping at bufend.
   Returns 0 (null) if count > bytes_used (underflow guard). */
static void *rb_memcpy_from(void *dst, struct ringbuf_t *src, size_t count)
{
    size_t bytes_used = ringbuf_bytes_used(src);
    if (count > bytes_used)
        return 0;

    uint8_t *u8dst = (uint8_t *)dst;
    const uint8_t *bufend = ringbuf_end(src);
    size_t nwritten = 0;

    while (nwritten != count) {
        size_t n = MIN((size_t)(bufend - src->tail), count - nwritten);
        memcpy(u8dst + nwritten, src->tail, n);
        src->tail += n;
        nwritten += n;
        if (src->tail == bufend)
            src->tail = src->buf;
    }
    return src->tail;
}

int main(void)
{
    uint8_t storage[CAPACITY + 1];
    uint8_t out[CAPACITY];
    struct ringbuf_t rb;
    rb.buf  = storage;
    rb.size = CAPACITY + 1;
    rb.head = storage;
    rb.tail = storage;

    /* Write 6 bytes into buffer starting at offset 0 */
    uint8_t data_in[6] = {'A','B','C','D','E','F'};
    rb_memcpy_into(&rb, data_in, 6);
    printf("after write 6: head_offset=%d used=%d\n",
           (int)(rb.head - rb.buf),
           (int)ringbuf_bytes_used(&rb));

    /* Read 3 bytes out */
    memset(out, 0, sizeof(out));
    rb_memcpy_from(out, &rb, 3);
    printf("read 3: [%c%c%c] tail_offset=%d\n",
           out[0], out[1], out[2],
           (int)(rb.tail - rb.buf));

    /* Write 5 more bytes — this WILL wrap head around */
    uint8_t data_in2[5] = {'G','H','I','J','K'};
    rb_memcpy_into(&rb, data_in2, 5);
    printf("after write 5 (wrap): head_offset=%d used=%d\n",
           (int)(rb.head - rb.buf),
           (int)ringbuf_bytes_used(&rb));

    /* Read all used bytes out — this crosses the wrap boundary */
    size_t used = ringbuf_bytes_used(&rb);
    memset(out, 0, sizeof(out));
    rb_memcpy_from(out, &rb, used);
    printf("read %d (across wrap): ", (int)used);
    size_t i;
    for (i = 0; i < used; i++)
        printf("%c", out[i]);
    printf("\n");

    /* Underflow: try to read more bytes than available */
    void *ret = rb_memcpy_from(out, &rb, 1);
    printf("underflow read returns null: %s\n", ret == 0 ? "yes" : "no");

    return 0;
}
