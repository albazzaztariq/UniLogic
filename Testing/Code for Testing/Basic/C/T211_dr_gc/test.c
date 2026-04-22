// @dr types = strict

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

int add_strict(int a, int b);
char* greet_strict(char* who);
int main(void);

int add_strict(int a, int b)
{
    return (a + b);
}

char* greet_strict(char* who)
{
    return __ul_strcat("Hello ", who);
}

int main(void)
{
    int result = add_strict(3, 7);
    printf("%d\n", result);
    char* msg = greet_strict("World");
    printf("%s\n", msg);
    int x = 10;
    if ((x > 5)) {
        printf("%s\n", "strict check passed");
    }
    return 0;
}


