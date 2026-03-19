#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

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


