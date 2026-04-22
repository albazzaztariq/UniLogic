#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* __ul_char_from_code(int code) {
    char* r = (char*)__ul_malloc(2); r[0] = (char)code; r[1] = 0; return r; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

int main(void);

int main(void)
{
    int x = 10;
    int y = 20;
    int sum = (x + y);
    printf("%d\n", sum);
    float pi = 3.14;
    printf("%f\n", pi);
    char* name = "UniLogic";
    printf("%s\n", name);
    int flag = 1;
    printf("%d\n", flag);
    int a = 100;
    a += 50;
    a -= 25;
    a *= 2;
    printf("%d\n", a);
    int mod = (17 % 5);
    printf("%d\n", mod);
    char* greeting = __ul_strcat("Hello ", "World");
    printf("%s\n", greeting);
    const int MAX = 999;
    printf("%d\n", MAX);
    return 0;
}


