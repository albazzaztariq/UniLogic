#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

int add(int a, int b);
int main(void);

int add(int a, int b)
{
    return (a + b);
}

int main(void)
{
    int x = add(10, 32);
    printf("%d\n", x);
    int y = (x * 2);
    printf("%d\n", y);
    return 0;
}


