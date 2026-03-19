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

int outer(void);
int outer__add_ten(int x);
int outer__twice(int x);
int main(void);
char* main__greet(void);

int outer__add_ten(int x)
{
    return (x + 10);
}

int outer__twice(int x)
{
    return (x * 2);
}

int outer(void)
{
    int a = outer__add_ten(5);
    int b = outer__twice(a);
    printf("%d\n", a);
    printf("%d\n", b);
    return b;
}

char* main__greet(void)
{
    return "hello from nested";
}

int main(void)
{
    int result = outer();
    printf("%d\n", result);
    char* msg = main__greet();
    printf("%s\n", msg);
    return 0;
}


