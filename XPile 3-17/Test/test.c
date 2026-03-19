#include <stdio.h>
#include <stdint.h>
#include <string.h>

int add(int a, int b);
int factorial(int n);
int is_even(int n);
int clamp(int val, int lo, int hi);
int main(void);

int add(int a, int b)
{
    return (a + b);
}

int factorial(int n)
{
    if ((n <= 1)) {
        return 1;
    }
    return (n * factorial((n - 1)));
}

int is_even(int n)
{
    return ((n % 2) == 0);
}

int clamp(int val, int lo, int hi)
{
    if ((val < lo)) {
        return lo;
    }
    if ((val > hi)) {
        return hi;
    }
    return val;
}

int main(void)
{
    int x = add(3, 7);
    printf("%d\n", x);
    const int limit = 100;
    int i = 0;
    while ((i < 5)) {
        printf("%d\n", i);
        i += 1;
    }
    if (is_even(x)) {
        printf("%d\n", 1);
    } else {
        printf("%d\n", 0);
    }
    float ratio = ((float)x);
    printf("%f\n", ratio);
    int clamped = clamp(x, 0, 8);
    printf("%d\n", clamped);
    int fact = factorial(5);
    printf("%d\n", fact);
    int nums[] = {10, 20, 30, 40, 50};
    for (int _i = 0; _i < (int)(sizeof(nums)/sizeof(nums[0])); _i++) {
        int n = nums[_i];
        printf("%d\n", n);
    }
    return 0;
}

