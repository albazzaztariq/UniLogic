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
    int a = 42;
    float b = ((float)a);
    printf("%f\n", b);
    float c = 3.7;
    int d = ((int)c);
    printf("%d\n", d);
    int e = 123;
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", e);
    char* f = _cast_buf_0;
    printf("%s\n", f);
    int g = 1;
    int h = ((int)g);
    printf("%d\n", h);
    int i = 0;
    int j = ((i) != 0);
    printf("%d\n", j);
    int k = 1;
    int l = ((k) != 0);
    printf("%d\n", l);
    return 0;
}


