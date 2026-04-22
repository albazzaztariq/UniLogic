/* test_adler32_arith.c -- adler32 pure arithmetic core for c2ul
 * Exercises: bit_and, bit_or, bit_left, bit_right, %, while, if/else if, return
 * Avoids char* / pointer arithmetic (known UL wall).
 * Uses only int arithmetic and pre-known byte values.
 */

#include <stdio.h>

#define BASE 65521U

/* adler32 of a fixed 13-byte sequence: "Hello, World!"
 * bytes: 72 101 108 108 111 44 32 87 111 114 108 100 33
 * Computed via loop over hardcoded array index operations.
 */
unsigned long adler32_hello_world(void) {
    unsigned long adler = 1;
    unsigned long sum2 = 0;
    int b;

    /* Process each byte manually (avoids pointer walking) */
    b = 72;  adler += b; sum2 += adler;  /* H */
    b = 101; adler += b; sum2 += adler;  /* e */
    b = 108; adler += b; sum2 += adler;  /* l */
    b = 108; adler += b; sum2 += adler;  /* l */
    b = 111; adler += b; sum2 += adler;  /* o */
    b = 44;  adler += b; sum2 += adler;  /* , */
    b = 32;  adler += b; sum2 += adler;  /* space */
    b = 87;  adler += b; sum2 += adler;  /* W */
    b = 111; adler += b; sum2 += adler;  /* o */
    b = 114; adler += b; sum2 += adler;  /* r */
    b = 108; adler += b; sum2 += adler;  /* l */
    b = 100; adler += b; sum2 += adler;  /* d */
    b = 33;  adler += b; sum2 += adler;  /* ! */

    adler = adler % BASE;
    sum2 = sum2 % BASE;

    return adler | (sum2 << 16);
}

/* adler32 of "" (empty) -> 1 */
unsigned long adler32_empty(void) {
    return 1UL;
}

/* adler32 of "abc" */
unsigned long adler32_abc(void) {
    unsigned long adler = 1;
    unsigned long sum2 = 0;
    int b;

    b = 97;  adler += b; sum2 += adler;
    b = 98;  adler += b; sum2 += adler;
    b = 99;  adler += b; sum2 += adler;

    adler = adler % BASE;
    sum2 = sum2 % BASE;

    return adler | (sum2 << 16);
}

/* Split an adler32 into s1 and s2 components */
unsigned long adler32_lo(unsigned long adler) {
    return adler & 0xffff;
}

unsigned long adler32_hi(unsigned long adler) {
    return (adler >> 16) & 0xffff;
}

/* adler32_combine_(adler1, adler2, len2):
 * combine two adler32 values - pure integer arithmetic.
 */
unsigned long adler32_combine(unsigned long adler1, unsigned long adler2, unsigned long len2) {
    unsigned long sum1;
    unsigned long sum2;
    unsigned long rem;

    rem = len2 % BASE;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    sum2 = sum2 % BASE;
    sum1 = sum1 + (adler2 & 0xffff) + BASE - 1;
    sum2 = sum2 + ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + BASE - rem;
    if (sum1 >= BASE) sum1 = sum1 - BASE;
    if (sum1 >= BASE) sum1 = sum1 - BASE;
    if (sum2 >= (BASE * 2)) sum2 = sum2 - (BASE * 2);
    if (sum2 >= BASE) sum2 = sum2 - BASE;
    return sum1 | (sum2 << 16);
}

/* adler32 of "Hello" for combine test */
unsigned long adler32_hello(void) {
    unsigned long adler = 1;
    unsigned long sum2 = 0;
    int b;
    b = 72;  adler += b; sum2 += adler;
    b = 101; adler += b; sum2 += adler;
    b = 108; adler += b; sum2 += adler;
    b = 108; adler += b; sum2 += adler;
    b = 111; adler += b; sum2 += adler;
    adler = adler % BASE;
    sum2 = sum2 % BASE;
    return adler | (sum2 << 16);
}

/* adler32 of ", World!" for combine test */
unsigned long adler32_world(void) {
    unsigned long adler = 1;
    unsigned long sum2 = 0;
    int b;
    b = 44;  adler += b; sum2 += adler;
    b = 32;  adler += b; sum2 += adler;
    b = 87;  adler += b; sum2 += adler;
    b = 111; adler += b; sum2 += adler;
    b = 114; adler += b; sum2 += adler;
    b = 108; adler += b; sum2 += adler;
    b = 100; adler += b; sum2 += adler;
    b = 33;  adler += b; sum2 += adler;
    adler = adler % BASE;
    sum2 = sum2 % BASE;
    return adler | (sum2 << 16);
}

int main(void) {
    unsigned long r;
    unsigned long lo;
    unsigned long hi;
    unsigned long recombined;
    unsigned long r1;
    unsigned long r2;
    unsigned long combined;

    /* Test 1: empty */
    r = adler32_empty();
    printf("adler32(\"\") = %lu\n", r);

    /* Test 2: abc */
    r = adler32_abc();
    printf("adler32(\"abc\") = %lu\n", r);

    /* Test 3: Hello World */
    r = adler32_hello_world();
    printf("adler32(\"Hello, World!\") = %lu\n", r);

    /* Test 4: bit operations - split and recombine */
    lo = adler32_lo(r);
    hi = adler32_hi(r);
    recombined = lo | (hi << 16);
    printf("lo=%lu hi=%lu recombined=%lu match=%d\n", lo, hi, recombined, recombined == r);

    /* Test 5: combine two halves */
    r1 = adler32_hello();
    r2 = adler32_world();
    combined = adler32_combine(r1, r2, 8);
    printf("adler32(\"Hello\") = %lu\n", r1);
    printf("adler32(\", World!\") = %lu\n", r2);
    printf("combined = %lu\n", combined);
    printf("direct   = %lu\n", r);
    printf("match = %d\n", combined == r);

    /* Test 6: bit operations stress */
    unsigned long x = 0xDEADBEEF;
    unsigned long y = (x >> 16) & 0xffff;
    unsigned long z = x & 0xffff;
    unsigned long w = y | (z << 16);
    printf("bit_swap: x=%lu y=%lu z=%lu w=%lu\n", x, y, z, w);

    return 0;
}
