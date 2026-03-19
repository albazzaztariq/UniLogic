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

_Result_int divide(int a, int b);
int main(void);

_Result_int divide(int a, int b)
{
    if ((b == 0)) {
        return (_Result_int){0, 0, "division by zero"};
    }
    return (_Result_int){1, (a / b), ""};
}

int main(void)
{
    _Result_int _r0 = divide(10, 2);
    if (!_r0._ok) {
        fprintf(stderr, "error: %s\n", _r0._error);
        exit(1);
    }
    int r1 = _r0._value;
    printf("%d\n", r1);
    _Result_int _r1 = divide(100, 5);
    if (!_r1._ok) {
        fprintf(stderr, "error: %s\n", _r1._error);
        exit(1);
    }
    int r2 = _r1._value;
    printf("%d\n", r2);
    _Result_int _r2 = divide(42, 7);
    if (!_r2._ok) {
        fprintf(stderr, "error: %s\n", _r2._error);
        exit(1);
    }
    int r3 = _r2._value;
    printf("%d\n", r3);
    return 0;
}


