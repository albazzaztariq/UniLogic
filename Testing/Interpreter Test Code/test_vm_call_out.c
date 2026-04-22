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
    int result = add(10, 20);
    printf("%d\n", result);
    printf("%d\n", add(3, 4));
    return 0;
}


