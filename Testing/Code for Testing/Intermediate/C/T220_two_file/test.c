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

int clamp(int val, int lo, int hi);
int abs_val(int x);
int max_of(int a, int b);
char* repeat_string(char* s, int n);
int main(void);

int clamp(int val, int lo, int hi)
{
    if ((val < lo)) {
        return lo;
    }
    if ((val > hi)) {
        return hi;
    }
    return val;
}

int abs_val(int x)
{
    if ((x < 0)) {
        return (0 - x);
    }
    return x;
}

int max_of(int a, int b)
{
    if ((a > b)) {
        return a;
    }
    return b;
}

char* repeat_string(char* s, int n)
{
    char* result = "";
    int i = 0;
    while ((i < n)) {
        result = __ul_strcat(result, s);
        i = (i + 1);
    }
    return result;
}

int main(void)
{
    printf("%d\n", clamp(5, 0, 10));
    printf("%d\n", clamp((-3), 0, 10));
    printf("%d\n", clamp(15, 0, 10));
    printf("%d\n", abs_val(7));
    printf("%d\n", abs_val((-12)));
    printf("%d\n", max_of(3, 8));
    printf("%d\n", max_of(20, 5));
    printf("%s\n", repeat_string("ab", 3));
    int result = clamp(abs_val((-25)), 0, 20);
    printf("%d\n", result);
    return 0;
}


