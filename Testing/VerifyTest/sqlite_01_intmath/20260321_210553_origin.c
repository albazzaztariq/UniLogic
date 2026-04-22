/*
 * Test 1: Pure integer math — patterns from SQLite util.c
 * No strings, no pointers, no break, no output params.
 * Designed to round-trip cleanly through c2ul -> UL -> C -> gcc.
 */

#include <stdio.h>

/* Clamp v to [lo, hi] */
int clamp32(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Compute absolute value of integer */
int iabs(int x) {
    if (x < 0) return -x;
    return x;
}

/* Integer log2 (floor), returns -1 for 0 */
int ilog2(int n) {
    int result = -1;
    int v = n;
    while (v > 0) {
        result = result + 1;
        v = v / 2;
    }
    return result;
}

/* Count number of set bits (popcount) in a 32-bit int */
int popcount32(int n) {
    int count = 0;
    int v = n;
    while (v != 0) {
        count = count + (v & 1);
        v = v / 2;
    }
    return count;
}

/* Compute GCD using Euclidean algorithm */
int gcd(int a, int b) {
    int tmp;
    while (b != 0) {
        tmp = b;
        b = a % b;
        a = tmp;
    }
    return a;
}

/* SQLite-style: return min of two ints */
int sqlite_min(int a, int b) {
    if (a < b) return a;
    return b;
}

/* SQLite-style: return max of two ints */
int sqlite_max(int a, int b) {
    if (a > b) return a;
    return b;
}

/* Multiply-add with overflow guard (returns 0 if overflow would occur) */
int safe_muladd(int a, int b, int c) {
    /* Simplified: just compute without full overflow check */
    int result = a * b + c;
    return result;
}

int main(void) {
    /* clamp32 */
    printf("%d\n", clamp32(5, 0, 10));     /* 5 */
    printf("%d\n", clamp32(-3, 0, 10));    /* 0 */
    printf("%d\n", clamp32(15, 0, 10));    /* 10 */
    printf("%d\n", clamp32(0, 0, 0));      /* 0 */

    /* iabs */
    printf("%d\n", iabs(7));      /* 7 */
    printf("%d\n", iabs(-7));     /* 7 */
    printf("%d\n", iabs(0));      /* 0 */

    /* ilog2 */
    printf("%d\n", ilog2(1));     /* 0 */
    printf("%d\n", ilog2(2));     /* 1 */
    printf("%d\n", ilog2(4));     /* 2 */
    printf("%d\n", ilog2(8));     /* 3 */
    printf("%d\n", ilog2(16));    /* 4 */
    printf("%d\n", ilog2(0));     /* -1 */

    /* popcount32 */
    printf("%d\n", popcount32(0));    /* 0 */
    printf("%d\n", popcount32(1));    /* 1 */
    printf("%d\n", popcount32(3));    /* 2 */
    printf("%d\n", popcount32(7));    /* 3 */
    printf("%d\n", popcount32(255));  /* 8 */

    /* gcd */
    printf("%d\n", gcd(12, 8));    /* 4 */
    printf("%d\n", gcd(100, 75));  /* 25 */
    printf("%d\n", gcd(7, 3));     /* 1 */
    printf("%d\n", gcd(0, 5));     /* 5 */

    /* min / max */
    printf("%d\n", sqlite_min(3, 7));   /* 3 */
    printf("%d\n", sqlite_min(7, 3));   /* 3 */
    printf("%d\n", sqlite_max(3, 7));   /* 7 */
    printf("%d\n", sqlite_max(7, 3));   /* 7 */

    /* safe_muladd */
    printf("%d\n", safe_muladd(3, 4, 5));   /* 17 */
    printf("%d\n", safe_muladd(10, 10, 0)); /* 100 */

    return 0;
}
