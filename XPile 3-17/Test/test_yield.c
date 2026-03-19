#include <stdio.h>
#include <stdint.h>
#include <string.h>

void countdown(int n, void (*_yield_cb)(int));
void doubles(int n, void (*_yield_cb)(int));
void main(void);

static void _gen_body_0(int val) {
    printf("%d\n", val);
}

static void _gen_body_1(int d) {
    printf("%d\n", d);
}

void countdown(int n, void (*_yield_cb)(int))
{
    while ((n > 0)) {
        _yield_cb(n);
        n -= 1;
    }
}

void doubles(int n, void (*_yield_cb)(int))
{
    int i = 1;
    while ((i <= n)) {
        _yield_cb((i * 2));
        i += 1;
    }
}

void main(void)
{
    countdown(5, _gen_body_0);
    doubles(4, _gen_body_1);
}

