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
#include <stdlib.h>

typedef struct {
    int _ok;
    int _value;
    char* _error;
} _Result_int;

_Result_int divide(int a, int b);
_Result_int safe_calc(int x, int y);
int main(void);

_Result_int divide(int a, int b)
{
    if ((b == 0)) {
        return (_Result_int){0, 0, "division by zero"};
    }
    return (_Result_int){1, (a / b), ""};
}

_Result_int safe_calc(int x, int y)
{
    _Result_int _r0 = divide(x, y);
    if (!_r0._ok) {
        return (_Result_int){0, 0, _r0._error};
    }
    int result = _r0._value;
    return (_Result_int){1, (result * 2), ""};
}

int main(void)
{
    _Result_int _r1 = divide(10, 2);
    if (!_r1._ok) {
        fprintf(stderr, "error: %s\n", _r1._error);
        exit(1);
    }
    int a = _r1._value;
    printf("%d\n", a);
    _Result_int _r2 = safe_calc(20, 4);
    if (!_r2._ok) {
        fprintf(stderr, "error: %s\n", _r2._error);
        exit(1);
    }
    int b = _r2._value;
    printf("%d\n", b);
    _Result_int _r3 = divide(5, 0);
    if (!_r3._ok) {
        fprintf(stderr, "error: %s\n", _r3._error);
        exit(1);
    }
    int c = _r3._value;
    printf("%d\n", c);
    return 0;
}


