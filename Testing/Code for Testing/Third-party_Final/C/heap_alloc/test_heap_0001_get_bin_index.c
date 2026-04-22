/* test_heap_0001_get_bin_index.c
   Tests: bit-shift pointer arithmetic, comparison, uint typedef, ternary.
   Extracted from heap.c::get_bin_index().
   Self-contained: no includes beyond stdio/stddef. */

#include <stdio.h>
#include <stddef.h>

typedef unsigned int uint;

uint get_bin_index(size_t sz) {
    uint index = 0;
    sz = sz < 4 ? 4 : sz;
    while (sz >>= 1) index++;
    index -= 2;
    if (index > 9 - 1) index = 9 - 1;
    return index;
}

int main(void) {
    /* Mirror the loop from main.c */
    int i;
    for (i = 1; i <= 2048; i += i) {
        printf("size: %d -> bin: %d\n", i, get_bin_index((size_t)i));
    }
    /* A few extra boundary cases */
    printf("size: 4 -> bin: %d\n",   get_bin_index(4));
    printf("size: 8 -> bin: %d\n",   get_bin_index(8));
    printf("size: 128 -> bin: %d\n", get_bin_index(128));
    printf("size: 65536 -> bin: %d\n", get_bin_index(65536));
    return 0;
}
