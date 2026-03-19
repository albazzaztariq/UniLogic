#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

int main(void);

int main(void)
{
    double val = 144.0;
    double result = sqrt(val);
    printf("%f\n", result);
    puts("Hello from FFI!");
    return 0;
}

