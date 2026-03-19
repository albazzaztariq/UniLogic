#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

void mul_table(int rows, int cols, void (*_yield_cb)(int));
void triangle(int n, void (*_yield_cb)(int));
int main(void);

static void _gen_body_0(int v) {
    printf("%d\n", v);
}

static void _gen_body_1(int t) {
    printf("%d\n", t);
}

void mul_table(int rows, int cols, void (*_yield_cb)(int))
{
    int r = 1;
    while ((r <= rows)) {
        int c = 1;
        while ((c <= cols)) {
            _yield_cb((r * c));
            c += 1;
        }
        r += 1;
    }
}

void triangle(int n, void (*_yield_cb)(int))
{
    int row = 1;
    while ((row <= n)) {
        int col = 1;
        while ((col <= row)) {
            _yield_cb(col);
            col += 1;
        }
        row += 1;
    }
}

int main(void)
{
    mul_table(2, 3, _gen_body_0);
    triangle(3, _gen_body_1);
    return 0;
}


