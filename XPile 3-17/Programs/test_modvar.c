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

int MAX_SIZE = 1024;
float PI = 3.14159;
char* GREETING = "hello";
int PRIMES[] = {2, 3, 5, 7, 11};
int double_max(void);
int main(void);

int double_max(void)
{
    return (MAX_SIZE * 2);
}

int main(void)
{
    printf("%d\n", MAX_SIZE);
    printf("%f\n", PI);
    printf("%s\n", GREETING);
    printf("%d\n", PRIMES[0]);
    printf("%d\n", PRIMES[4]);
    printf("%d\n", double_max());
    MAX_SIZE = 2048;
    printf("%d\n", MAX_SIZE);
    return 0;
}


