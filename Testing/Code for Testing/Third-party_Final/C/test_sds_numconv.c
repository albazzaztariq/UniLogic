/*
 * Extracted and simplified from antirez/sds — sds.c
 * sdsll2str and sdsull2str, standalone (no SDS type system needed).
 * Pure integer arithmetic + char array manipulation.
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>

/* Convert a long long into a string, return the number of chars written. */
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

    /* Reverse the string */
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

/* Convert unsigned long long into a string. */
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
