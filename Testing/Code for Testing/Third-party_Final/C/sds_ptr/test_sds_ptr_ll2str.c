/*
 * SDS Pointer Pattern Test 2: sdsll2str - integer to string with pointer reversal
 * Extracted from antirez/sds sds.c sdsll2str() and sdsull2str().
 * Pattern:
 *   1. p = s; do { *p++ = '0'+(v%10); v/=10; } while(v);  -- write-through + advance
 *   2. l = p - s;                                           -- pointer subtraction
 *   3. *p = '\0';                                           -- deref write
 *   4. p--; while(s < p) { swap *s/*p; s++; p--; }        -- backward pointer walk
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>

int sds_ll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    int l;

    if (value < 0) {
        if (value != LLONG_MIN) {
            v = (unsigned long long)(-value);
        } else {
            v = ((unsigned long long)LLONG_MAX) + 1;
        }
    } else {
        v = (unsigned long long)value;
    }

    p = s;
    do {
        *p++ = '0' + (int)(v % 10);
        v /= 10;
    } while (v);
    if (value < 0) *p++ = '-';

    l = (int)(p - s);
    *p = '\0';

    p--;
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

int sds_ull2str(char *s, unsigned long long v) {
    char *p, aux;
    int l;

    p = s;
    do {
        *p++ = '0' + (int)(v % 10);
        v /= 10;
    } while (v);

    l = (int)(p - s);
    *p = '\0';

    p--;
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

int main(void) {
    char buf[32];
    int len;

    len = sds_ll2str(buf, 0);
    printf("ll2str(0) = \"%s\" len=%d\n", buf, len);

    len = sds_ll2str(buf, 42);
    printf("ll2str(42) = \"%s\" len=%d\n", buf, len);

    len = sds_ll2str(buf, -42);
    printf("ll2str(-42) = \"%s\" len=%d\n", buf, len);

    len = sds_ll2str(buf, 1234567890);
    printf("ll2str(1234567890) = \"%s\" len=%d\n", buf, len);

    len = sds_ll2str(buf, -9999);
    printf("ll2str(-9999) = \"%s\" len=%d\n", buf, len);

    len = sds_ull2str(buf, 0);
    printf("ull2str(0) = \"%s\" len=%d\n", buf, len);

    len = sds_ull2str(buf, 18446744073709551615ULL);
    printf("ull2str(ULLONG_MAX) = \"%s\" len=%d\n", buf, len);

    return 0;
}
