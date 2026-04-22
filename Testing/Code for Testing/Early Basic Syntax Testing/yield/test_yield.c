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

void countdown(int n, void (*_yield_cb)(int));
void doubles(int n, void (*_yield_cb)(int));
int main(void);

static void _gen_body_0(int val) {
    printf("%d\n", val);
}

static void _gen_body_1(int d) {
    printf("%d\n", d);
}

void countdown(int n, void (*_yield_cb)(int))
{
    while ((n > 0)) {
        _yield_cb(n);
        n -= 1;
    }
}

void doubles(int n, void (*_yield_cb)(int))
{
    int i = 1;
    while ((i <= n)) {
        _yield_cb((i * 2));
        i += 1;
    }
}

int main(void)
{
    countdown(5, _gen_body_0);
    doubles(4, _gen_body_1);
    return 0;
}


