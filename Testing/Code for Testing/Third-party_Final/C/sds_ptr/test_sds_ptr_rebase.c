/*
 * SDS Pointer Pattern Test 4: Pointer rebasing after realloc
 * Extracted from antirez/sds sdsMakeRoomFor() pattern.
 * Pattern: after realloc(), old pointer is invalid; new pointer must be used.
 * Also tests: buffer pointer arithmetic as copy target (newsh + hdrlen).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Grow a heap buffer and copy old content. Returns new pointer.
 * Simulates the realloc+rebase pattern from sdsMakeRoomFor.
 */
char *grow_buf(char *buf, int oldlen, int newcap) {
    char *newbuf = (char *)realloc(buf, newcap);
    if (!newbuf) return NULL;
    /* Zero the new area */
    memset(newbuf + oldlen, 0, newcap - oldlen);
    return newbuf;
}

/* Append src to dst, returning new pointer (may have been reallocated). */
char *buf_append(char *dst, int *dstlen, int *dstcap, const char *src, int srclen) {
    int needed = *dstlen + srclen + 1;
    if (needed > *dstcap) {
        int newcap = needed * 2;
        char *newdst = (char *)realloc(dst, newcap);
        if (!newdst) return dst;
        dst = newdst;          /* rebase: old ptr invalid, use new */
        *dstcap = newcap;
    }
    memcpy(dst + *dstlen, src, srclen);
    *dstlen += srclen;
    dst[*dstlen] = '\0';
    return dst;
}

int main(void) {
    /* Test 1: grow_buf */
    char *buf = (char *)malloc(8);
    memcpy(buf, "hello", 6);
    buf = grow_buf(buf, 5, 32);
    printf("after grow: \"%s\"\n", buf);
    free(buf);

    /* Test 2: buf_append with realloc rebase */
    int dstlen = 0, dstcap = 8;
    char *dst = (char *)malloc(dstcap);
    dst[0] = '\0';

    dst = buf_append(dst, &dstlen, &dstcap, "hello", 5);
    printf("after append 1: \"%s\" len=%d cap=%d\n", dst, dstlen, dstcap);

    dst = buf_append(dst, &dstlen, &dstcap, " world", 6);
    printf("after append 2: \"%s\" len=%d\n", dst, dstlen);

    dst = buf_append(dst, &dstlen, &dstcap, "! more text here to force realloc", 33);
    printf("after append 3: \"%s\" len=%d\n", dst, dstlen);

    free(dst);
    return 0;
}
