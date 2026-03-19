#include <stdio.h>
#include <stdint.h>
#include <string.h>

int add(int a, int b);
int main(void);

int add(int a, int b)
{
    return (a + b);
}

int main(void)
{
    int x = add(3, 7);
    printf("%d\n", x);
    return 0;
}

