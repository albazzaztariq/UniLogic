/* test_crc32.c -- CRC32 table-driven lookup for c2ul
 * Exercises: static const table (large int array init), XOR ops, while loop,
 *            bit_right, bit_and, function pointer callback pattern (simplified),
 *            typedef via int alias.
 * Adapted from madler/zlib crc32.c -- table-driven CRC32 algorithm.
 * Uses standard CRC32 polynomial 0xEDB88320 (reflected).
 */

#include <stdio.h>

/* CRC32 table: 256 entries computed from polynomial 0xEDB88320 */
/* These are the actual zlib CRC table values for indices 0..15 (first 16) */
static unsigned long crc32_table[16] = {
    0x00000000UL, 0x77073096UL, 0xEE0E612CUL, 0x990951BAUL,
    0x076DC419UL, 0x706AF48FUL, 0xE963A535UL, 0x9E6495A3UL,
    0x0EDB8832UL, 0x79DCB8A5UL, 0xE0D5E91BUL, 0x97D2D988UL,
    0x09B64C2BUL, 0x7EB35CBCUL, 0xE7B82D09UL, 0x90BF1CBFUL
};

/* Compute CRC32 of a byte value using the table (one step) */
unsigned long crc32_byte(unsigned long crc, int byte) {
    unsigned long idx;
    unsigned long entry;
    idx = (crc ^ byte) & 0x0f;  /* use lower 4 bits to index our 16-entry table */
    entry = crc32_table[idx];
    crc = (crc >> 4) ^ entry;
    return crc;
}

/* get_crc_table: return entry from table by index */
unsigned long get_crc_entry(int idx) {
    if (idx < 0)
        return 0;
    if (idx > 15)
        return 0;
    return crc32_table[idx];
}

/* XOR folding -- typical zlib CRC step */
unsigned long crc32_xor_fold(unsigned long a, unsigned long b) {
    return a ^ b;
}

/* Do/while with break-like pattern using flag */
unsigned long crc32_loop_test(unsigned long init) {
    unsigned long result = init;
    int n = 0;
    int found = 0;

    while (n < 16) {
        result = result ^ crc32_table[n];
        if (result == 0) {
            found = 1;
            n = 16;  /* exit loop */
        } else {
            n = n + 1;
        }
    }
    return result;
}

/* Fibonacci using while loop -- structural test */
int fib(int n) {
    int a = 0;
    int b = 1;
    int tmp;
    while (n > 0) {
        tmp = b;
        b = a + b;
        a = tmp;
        n = n - 1;
    }
    return a;
}

/* Bitwise stress test: reflect a byte (bit reversal) */
unsigned long reflect8(unsigned long v) {
    unsigned long result = 0;
    int i = 0;
    while (i < 8) {
        result = (result << 1) | (v & 1);
        v = v >> 1;
        i = i + 1;
    }
    return result;
}

int main(void) {
    int i;
    unsigned long r;

    /* Test 1: table entries */
    printf("crc32_table[0] = %lu\n", crc32_table[0]);
    printf("crc32_table[1] = %lu\n", crc32_table[1]);
    printf("crc32_table[7] = %lu\n", crc32_table[7]);
    printf("crc32_table[15] = %lu\n", crc32_table[15]);

    /* Test 2: get_crc_entry */
    printf("entry[0] = %lu\n", get_crc_entry(0));
    printf("entry[8] = %lu\n", get_crc_entry(8));
    printf("entry[-1] = %lu\n", get_crc_entry(-1));
    printf("entry[16] = %lu\n", get_crc_entry(16));

    /* Test 3: XOR fold */
    printf("xor_fold(0xDEAD, 0xBEEF) = %lu\n", crc32_xor_fold(0xDEAD, 0xBEEF));
    printf("xor_fold(0, 0) = %lu\n", crc32_xor_fold(0, 0));
    printf("xor_fold(0xFFFF, 0xFFFF) = %lu\n", crc32_xor_fold(0xFFFF, 0xFFFF));

    /* Test 4: crc32_byte single steps */
    r = crc32_byte(0xFFFFFFFFUL, 0);
    printf("crc32_byte(0xFFFFFFFF, 0) = %lu\n", r);
    r = crc32_byte(0, 1);
    printf("crc32_byte(0, 1) = %lu\n", r);

    /* Test 5: loop test */
    r = crc32_loop_test(0x12345678UL);
    printf("crc32_loop_test(0x12345678) = %lu\n", r);

    /* Test 6: fibonacci (while loop + multiple assign) */
    printf("fib(0)=%d fib(1)=%d fib(5)=%d fib(10)=%d\n",
           fib(0), fib(1), fib(5), fib(10));

    /* Test 7: reflect8 -- while loop, bit shifts, OR */
    printf("reflect8(0x01) = %lu\n", reflect8(1));
    printf("reflect8(0x80) = %lu\n", reflect8(0x80));
    printf("reflect8(0xAB) = %lu\n", reflect8(0xAB));

    /* Test 8: table XOR chain */
    r = 0;
    i = 0;
    while (i < 8) {
        r = r ^ crc32_table[i];
        i = i + 1;
    }
    printf("xor_chain[0..7] = %lu\n", r);

    return 0;
}
