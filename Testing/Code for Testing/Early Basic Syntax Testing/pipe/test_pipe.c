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

int twice(int x);
int add_one(int x);
int flip_sign(int x);
int main(void);

int twice(int x)
{
    return (x * 2);
}

int add_one(int x)
{
    return (x + 1);
}

int flip_sign(int x)
{
    return (0 - x);
}

int main(void)
{
    int a = twice(5);
    printf("%d\n", a);
    int b = add_one(twice(5));
    printf("%d\n", b);
    int c = flip_sign(add_one(twice(3)));
    printf("%d\n", c);
    int d = add_one(10);
    printf("%d\n", d);
    return 0;
}


