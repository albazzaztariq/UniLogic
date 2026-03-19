#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

int factorial(int n);
int square(int x);
int add(int a, int b);
int main(void);

int factorial(int n)
{
    if ((n <= 1)) {
        return 1;
    }
    return (n * factorial((n - 1)));
}

int square(int x)
{
    return (x * x);
}

int add(int a, int b)
{
    return (a + b);
}

int main(void)
{
    int f = factorial(5);
    printf("%d\n", f);
    int s = square(7);
    printf("%d\n", s);
    int sum = add(10, 20);
    printf("%d\n", sum);
    int composed = square(add(3, 4));
    printf("%d\n", composed);
    return 0;
}


