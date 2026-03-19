#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

typedef struct {
    int value;
    int code;
} Inner;

typedef struct {
    char* label;
    Inner core;
} Middle;

typedef struct {
    int id;
    Middle mid;
} Outer;

Inner set_inner(int v, int c);
int main(void);

Inner set_inner(int v, int c)
{
    Inner i;
    i.value = v;
    i.code = c;
    return i;
}

int main(void)
{
    Outer o;
    o.id = 1;
    o.mid.label = "test";
    o.mid.core.value = 42;
    o.mid.core.code = 99;
    printf("%d\n", o.id);
    printf("%s\n", o.mid.label);
    printf("%d\n", o.mid.core.value);
    printf("%d\n", o.mid.core.code);
    Outer o2;
    o2.id = 2;
    o2.mid.label = "second";
    o2.mid.core.value = 100;
    o2.mid.core.code = 200;
    printf("%d\n", o2.id);
    printf("%s\n", o2.mid.label);
    printf("%d\n", o2.mid.core.value);
    printf("%d\n", o2.mid.core.code);
    o.mid.core.value = 777;
    printf("%d\n", o.mid.core.value);
    return 0;
}


