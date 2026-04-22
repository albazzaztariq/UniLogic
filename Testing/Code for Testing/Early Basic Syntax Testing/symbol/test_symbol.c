#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

typedef struct {
    int x;
    int y;
} Vec2;

Vec2 vec_add(Vec2 a, Vec2 b);
int main(void);

Vec2 vec_add(Vec2 a, Vec2 b)
{
    Vec2 result;
    result.x = (a.x + b.x);
    result.y = (a.y + b.y);
    return result;
}

int main(void)
{
    Vec2 v1;
    v1.x = 1;
    v1.y = 2;
    Vec2 v2;
    v2.x = 3;
    v2.y = 4;
    Vec2 v3 = vec_add(v1, v2);
    printf("%d\n", v3.x);
    printf("%d\n", v3.y);
    return 0;
}


