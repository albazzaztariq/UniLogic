#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

int validate(int x);
int main(void);

int validate(int x)
{
    if ((x < 0)) {
        return (-1);
    }
    return x;
}

int main(void)
{
    int val = validate(10);
    if ((val < 0)) {
        printf("%d\n", 0);
        return 1;
    }
    printf("%d\n", val);
    return 0;
}


