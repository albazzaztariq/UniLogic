#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

int main(void);

int main(void)
{
    int counter = 10;
    counter += 5;
    counter *= 2;
    int i = 0;
    i++;
    i++;
    i--;
    printf("%d\n", counter);
    printf("%d\n", i);
    double val = 144.0;
    double deep = sqrt(sqrt(val));
    printf("%f\n", deep);
    int x = 42;
    double y = ((double)((float)x));
    printf("%f\n", y);
    int a = 1;
    int b = 2;
    int c = 3;
    if ((((a > 0) && (b > 0)) && (c > 0))) {
        printf("%d\n", a);
    }
    return 0;
}

