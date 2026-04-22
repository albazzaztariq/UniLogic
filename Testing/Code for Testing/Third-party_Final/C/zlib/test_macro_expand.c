/* test_macro_expand.c -- test gcc-E macro expansion feeding into c2ul
 * Simulates what happens when adler32's DO1..DO16 macros expand.
 * After gcc -E, macros become compound statements; c2ul must handle them.
 * Also tests: compound statements in loops, chained += assignments,
 *             bit shifts combined with add/mod.
 */

#include <stdio.h>

#define BASE 65521U

/* Simulate DO1 macro: {adler += buf[i]; sum2 += adler;} */
/* We inline the macro expansion manually here for 4 steps (DO4) */
unsigned long adler32_do4(unsigned long adler, unsigned long sum2, int b0, int b1, int b2, int b3) {
    adler += b0; sum2 += adler;
    adler += b1; sum2 += adler;
    adler += b2; sum2 += adler;
    adler += b3; sum2 += adler;
    return adler | (sum2 << 16);
}

/* Test chained compound assignment: a += b; c += a; (DO1 pattern) */
unsigned long do1_step(unsigned long adler, unsigned long sum2, int byte_val) {
    adler += byte_val;
    sum2 += adler;
    return adler | (sum2 << 16);
}

/* Test multiple +=/-= in sequence (flags accumulation like zlibCompileFlags) */
unsigned long accumulate_flags(void) {
    unsigned long flags = 0;
    flags += 1;        /* bit 0 */
    flags += 1 << 2;   /* bit 2 */
    flags += 2 << 4;   /* bits 5:4 */
    flags += 1 << 8;   /* bit 8 */
    flags += 1L << 16; /* bit 16 */
    return flags;
}

/* Test nested macro-style computation: MOD(a) = a %= BASE */
unsigned long mod_base(unsigned long v) {
    return v % BASE;
}

/* Test if-chain without else (like mod with NO_DIVIDE flag) */
unsigned long adler_mod_nodivide(unsigned long a) {
    unsigned long tmp;
    /* Simulate CHOP(a): tmp = a >> 16; a &= 0xffff; a += (tmp << 4) - tmp; */
    tmp = a >> 16;
    a = a & 0xffff;
    a = a + (tmp << 4) - tmp;
    /* Simulate MOD28(a): CHOP(a); if (a >= BASE) a -= BASE; */
    tmp = a >> 16;
    a = a & 0xffff;
    a = a + (tmp << 4) - tmp;
    if (a >= BASE)
        a = a - BASE;
    return a;
}

/* Test compound stmt blocks ({...}) appearing as macro-expanded bodies */
unsigned long sum_stepped(int n) {
    unsigned long total = 0;
    unsigned long running = 1;
    int i = 0;
    while (i < n) {
        /* This { ... } block simulates a DO1 macro expansion */
        {
            running += i;
            total += running;
        }
        i = i + 1;
    }
    return total;
}

int main(void) {
    unsigned long r;

    /* Test 1: DO4-style macro expansion inline */
    r = adler32_do4(1, 0, 72, 101, 108, 108);  /* "Hell" */
    printf("do4(Hell) = %lu\n", r);

    /* Test 2: DO1 step */
    r = do1_step(1, 0, 72);  /* 'H' */
    printf("do1(H) = %lu\n", r);

    /* Test 3: flags accumulation */
    r = accumulate_flags();
    printf("accumulate_flags = %lu\n", r);

    /* Test 4: mod_base */
    printf("mod_base(65521) = %lu\n", mod_base(65521));
    printf("mod_base(65522) = %lu\n", mod_base(65522));
    printf("mod_base(130000) = %lu\n", mod_base(130000));

    /* Test 5: adler_mod_nodivide (CHOP+MOD28 pattern) */
    printf("adler_mod_nodivide(0) = %lu\n", adler_mod_nodivide(0));
    printf("adler_mod_nodivide(65521) = %lu\n", adler_mod_nodivide(65521));
    printf("adler_mod_nodivide(65522) = %lu\n", adler_mod_nodivide(65522));

    /* Test 6: sum_stepped (compound-stmt in while, like expanded DO loop) */
    printf("sum_stepped(5) = %lu\n", sum_stepped(5));
    printf("sum_stepped(10) = %lu\n", sum_stepped(10));

    return 0;
}
