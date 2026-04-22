/* test_adler32.c -- standalone adler32 core arithmetic test for c2ul
 * Extracted from madler/zlib adler32.c
 * Exercises: arithmetic, bit ops, while loops, if/else, parameter passing
 * NO macros (DO1..DO16 expanded manually for 4 bytes), no zlib typedefs.
 */

#include <stdio.h>

#define BASE 65521U

/* Compute adler32 checksum for a byte buffer.
 * adler: initial value (1 for fresh stream)
 * buf:   pointer to data (treated as unsigned int array here -- simplified)
 * len:   number of bytes
 */
unsigned long adler32_simple(unsigned long adler, const unsigned char *buf, unsigned int len) {
    unsigned long sum2;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler = adler & 0xffff;

    if (buf == 0)
        return 1UL;

    while (len > 0) {
        adler += buf[0];
        sum2 += adler;
        buf = buf + 1;
        len = len - 1;
    }

    adler = adler % BASE;
    sum2 = sum2 % BASE;

    return adler | (sum2 << 16);
}

/* adler32_combine: combine two adler32 values covering len1 and len2 bytes.
 * Extracted from adler32_combine_ in zlib.
 */
unsigned long adler32_combine(unsigned long adler1, unsigned long adler2, unsigned long len2) {
    unsigned long sum1;
    unsigned long sum2;
    unsigned long rem;

    if (len2 < 0)
        return 0xffffffffUL;

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

int main(void) {
    /* Test 1: empty input -> adler = 1 */
    unsigned long r = adler32_simple(1UL, 0, 0);
    printf("adler32(\"\") = %lu\n", r);

    /* Test 2: single byte 'A' (65) */
    unsigned char a[1];
    a[0] = 65;
    r = adler32_simple(1UL, a, 1);
    printf("adler32(\"A\") = %lu\n", r);

    /* Test 3: "abc" */
    unsigned char abc[3];
    abc[0] = 97; abc[1] = 98; abc[2] = 99;
    r = adler32_simple(1UL, abc, 3);
    printf("adler32(\"abc\") = %lu\n", r);

    /* Test 4: "Hello, World!" */
    unsigned char hw[13];
    hw[0]=72; hw[1]=101; hw[2]=108; hw[3]=108; hw[4]=111;
    hw[5]=44; hw[6]=32; hw[7]=87; hw[8]=111; hw[9]=114;
    hw[10]=108; hw[11]=100; hw[12]=33;
    r = adler32_simple(1UL, hw, 13);
    printf("adler32(\"Hello, World!\") = %lu\n", r);

    /* Test 5: bit operations - combine split */
    unsigned long lo = r & 0xffff;
    unsigned long hi = (r >> 16) & 0xffff;
    unsigned long recombined = lo | (hi << 16);
    printf("split-recombine: %lu == %lu -> %d\n", r, recombined, r == recombined);

    /* Test 6: adler32_combine */
    /* adler of "Hello" then " World!" separately */
    unsigned char hello[5];
    hello[0]=72; hello[1]=101; hello[2]=108; hello[3]=108; hello[4]=111;
    unsigned char world[8];
    world[0]=44; world[1]=32; world[2]=87; world[3]=111; world[4]=114;
    world[5]=108; world[6]=100; world[7]=33;
    unsigned long r1 = adler32_simple(1UL, hello, 5);
    unsigned long r2 = adler32_simple(1UL, world, 8);
    unsigned long combined = adler32_combine(r1, r2, 8);
    printf("adler32_combine result = %lu\n", combined);
    printf("direct full result     = %lu\n", r);
    printf("combine match: %d\n", combined == r);

    return 0;
}
