#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(void);

int main(void)
{
    int squares[(6 - 1)];
    for (int _ci0 = 0, x = 1; x < 6; x++, _ci0++) {
        squares[_ci0] = (x * x);
    }
    printf("%d\n", squares[0]);
    printf("%d\n", squares[4]);
    return 0;
}


