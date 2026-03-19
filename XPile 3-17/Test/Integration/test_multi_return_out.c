#include <stdio.h>
#include <stdint.h>
#include <string.h>

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


