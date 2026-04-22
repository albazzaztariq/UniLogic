/*
 * SDS Pointer Pattern Test 1: Two-pointer swap
 * Extracted from antirez/sds sdsll2str() - the swap step isolated.
 * Pattern: swap *s and *p via aux, advance s forward, step p backward.
 * This is the CORE of the backward pointer walk in sds_ll2str.
 */
#include <stdio.h>
#include <string.h>

/* Reverse a char array in-place using two-pointer swap.
 * s points to start of array, p points to last char.
 */
void reverse_buf(char *s, char *p) {
    char aux;
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
}

int main(void) {
    /* Test 1: reverse "hello" */
    char buf1[6] = {'h','e','l','l','o','\0'};
    reverse_buf(buf1, buf1 + 4);
    printf("reverse hello = %s\n", buf1);

    /* Test 2: reverse "abcde" */
    char buf2[6] = {'a','b','c','d','e','\0'};
    reverse_buf(buf2, buf2 + 4);
    printf("reverse abcde = %s\n", buf2);

    /* Test 3: single char (p == s, no swap) */
    char buf3[2] = {'z','\0'};
    reverse_buf(buf3, buf3);
    printf("reverse z = %s\n", buf3);

    /* Test 4: two chars */
    char buf4[3] = {'a','b','\0'};
    reverse_buf(buf4, buf4 + 1);
    printf("reverse ab = %s\n", buf4);

    return 0;
}
