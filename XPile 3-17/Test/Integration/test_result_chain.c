#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
#include <stdlib.h>

typedef struct {
    int _ok;
    int _value;
    char* _error;
} _Result_int;

_Result_int level1(int x);
_Result_int level2(int x);
_Result_int level3(int x);
int main(void);

_Result_int level1(int x)
{
    if ((x < 0)) {
        return (_Result_int){0, 0, "negative input"};
    }
    return (_Result_int){1, (x * 10), ""};
}

_Result_int level2(int x)
{
    _Result_int _r0 = level1(x);
    if (!_r0._ok) {
        return (_Result_int){0, 0, _r0._error};
    }
    int a = _r0._value;
    return (_Result_int){1, (a + 1), ""};
}

_Result_int level3(int x)
{
    _Result_int _r1 = level2(x);
    if (!_r1._ok) {
        return (_Result_int){0, 0, _r1._error};
    }
    int b = _r1._value;
    return (_Result_int){1, (b + 2), ""};
}

int main(void)
{
    _Result_int _r2 = level3(5);
    if (!_r2._ok) {
        fprintf(stderr, "error: %s\n", _r2._error);
        exit(1);
    }
    int r1 = _r2._value;
    printf("%d\n", r1);
    _Result_int _r3 = level3(0);
    if (!_r3._ok) {
        fprintf(stderr, "error: %s\n", _r3._error);
        exit(1);
    }
    int r2 = _r3._value;
    printf("%d\n", r2);
    _Result_int _r4 = level3((-1));
    if (!_r4._ok) {
        fprintf(stderr, "error: %s\n", _r4._error);
        exit(1);
    }
    int r3 = _r4._value;
    printf("%d\n", r3);
    return 0;
}


