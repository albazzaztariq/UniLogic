#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

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


