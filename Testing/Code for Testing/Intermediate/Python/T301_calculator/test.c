#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int safe_divide(int a, int b);
int power(int base, int exp);
int factorial(int n);
int gcd(int a, int b);
int main(void);

int add(int a, int b)
{
    return (a + b);
}

int subtract(int a, int b)
{
    return (a - b);
}

int multiply(int a, int b)
{
    return (a * b);
}

int safe_divide(int a, int b)
{
    if ((b == 0)) {
        return (-1);
    }
    return (a / b);
}

int power(int base, int exp)
{
    int result = 1;
    int i = 0;
    while ((i < exp)) {
        result = (result * base);
        i = (i + 1);
    }
    return result;
}

int factorial(int n)
{
    if ((n <= 1)) {
        return 1;
    }
    return (n * factorial((n - 1)));
}

int gcd(int a, int b)
{
    while ((b != 0)) {
        int temp = b;
        b = (a % b);
        a = temp;
    }
    return a;
}

int main(void)
{
    printf("%d\n", add(10, 25));
    printf("%d\n", subtract(100, 37));
    printf("%d\n", multiply(6, 7));
    printf("%d\n", safe_divide(100, 3));
    printf("%d\n", safe_divide(10, 0));
    printf("%d\n", power(2, 10));
    printf("%d\n", factorial(10));
    printf("%d\n", gcd(48, 18));
    printf("%d\n", gcd(100, 75));
    int result = add(multiply(3, 4), subtract(20, 8));
    printf("%d\n", result);
    return 0;
}


