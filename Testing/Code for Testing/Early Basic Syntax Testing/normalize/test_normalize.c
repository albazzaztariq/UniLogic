#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
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
    int counter = 10;
    counter += 5;
    counter *= 2;
    int i = 0;
    i++;
    i++;
    i--;
    printf("%d\n", counter);
    printf("%d\n", i);
    double val = 144.0;
    double deep = sqrt(sqrt(val));
    printf("%f\n", deep);
    int x = 42;
    double y = ((double)((float)x));
    printf("%f\n", y);
    int a = 1;
    int b = 2;
    int c = 3;
    if ((((a > 0) && (b > 0)) && (c > 0))) {
        printf("%d\n", a);
    }
    return 0;
}


