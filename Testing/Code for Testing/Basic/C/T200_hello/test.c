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

int main(void);

int main(void)
{
    printf("%s\n", "Hello, UniLogic!");
    int x = 42;
    printf("%d\n", x);
    float pi = 3.14;
    printf("%f\n", pi);
    char* name = "UniLogic";
    printf("%s\n", name);
    int flag = 1;
    printf("%d\n", flag);
    int sum = (10 + 20);
    printf("%d\n", sum);
    char* greeting = __ul_strcat("Hello ", "World");
    printf("%s\n", greeting);
    int a = 5;
    int b = 10;
    int c = (a + b);
    printf("%d\n", c);
    return 0;
}


