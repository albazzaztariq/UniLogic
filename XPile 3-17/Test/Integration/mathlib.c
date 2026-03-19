#include <stdio.h>
#include <stdint.h>
#include <string.h>

int factorial(int n);
int square(int x);
int add(int a, int b);

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


