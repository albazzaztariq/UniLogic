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

typedef struct { int _f0; int _f1; } _Tuple_int_int;
_Tuple_int_int minmax(int a, int b);
int main(void);

_Tuple_int_int minmax(int a, int b)
{
    if ((a < b)) {
        return (_Tuple_int_int){a, b};
    }
    return (_Tuple_int_int){b, a};
}

int main(void)
{
    _Tuple_int_int _td_0 = minmax(10, 3);
    int lo = _td_0._f0;
    int hi = _td_0._f1;
    printf("%d\n", lo);
    printf("%d\n", hi);
    return 0;
}


