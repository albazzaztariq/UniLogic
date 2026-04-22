#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

int is_prime(int n);
int fibonacci(int n);
int count_digits(int n);
int sum_digits(int n);
int reverse_number(int n);
int main(void);

int is_prime(int n)
{
    if ((n < 2)) {
        return 0;
    }
    if ((n == 2)) {
        return 1;
    }
    if (((n % 2) == 0)) {
        return 0;
    }
    int i = 3;
    while (((i * i) <= n)) {
        if (((n % i) == 0)) {
            return 0;
        }
        i = (i + 2);
    }
    return 1;
}

int fibonacci(int n)
{
    if ((n <= 0)) {
        return 0;
    }
    if ((n == 1)) {
        return 1;
    }
    int a = 0;
    int b = 1;
    int i = 2;
    while ((i <= n)) {
        int temp = (a + b);
        a = b;
        b = temp;
        i = (i + 1);
    }
    return b;
}

int count_digits(int n)
{
    if ((n == 0)) {
        return 1;
    }
    int count = 0;
    int val = n;
    if ((val < 0)) {
        val = (0 - val);
    }
    while ((val > 0)) {
        val = (val / 10);
        count = (count + 1);
    }
    return count;
}

int sum_digits(int n)
{
    int total = 0;
    int val = n;
    if ((val < 0)) {
        val = (0 - val);
    }
    while ((val > 0)) {
        total = (total + (val % 10));
        val = (val / 10);
    }
    return total;
}

int reverse_number(int n)
{
    int result = 0;
    int val = n;
    while ((val > 0)) {
        result = ((result * 10) + (val % 10));
        val = (val / 10);
    }
    return result;
}

int main(void)
{
    int i = 2;
    while ((i <= 30)) {
        if (is_prime(i)) {
            printf("%d\n", i);
        }
        i = (i + 1);
    }
    int j = 0;
    while ((j <= 10)) {
        printf("%d\n", fibonacci(j));
        j = (j + 1);
    }
    printf("%d\n", count_digits(12345));
    printf("%d\n", count_digits(0));
    printf("%d\n", sum_digits(12345));
    printf("%d\n", sum_digits(9999));
    printf("%d\n", reverse_number(12345));
    printf("%d\n", reverse_number(100));
    return 0;
}


