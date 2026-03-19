#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
#include <stdlib.h>

int check(int val);
int main(void);

int check(int val)
{
    if ((val < 0)) {
        printf("%d\n", (-1));
        exit(1);
    }
    return (val * 2);
}

int main(void)
{
    int result = check(5);
    printf("%d\n", result);
    exit(0);
    return 0;
}


