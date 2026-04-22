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

char* greet(char* who, char* greeting);
int main(void);
int main__add_offset(int x);
char* main__make_label(int n);

char* greet(char* who, char* greeting)
{
    return __ul_strcat(__ul_strcat(greeting, " "), who);
}

int main__add_offset(int x)
{
    int offset = 100;
    return (x + offset);
}

char* main__make_label(int n)
{
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", n);
    return __ul_strcat("item_", _cast_buf_0);
}

int main(void)
{
    printf("%d\n", main__add_offset(5));
    printf("%s\n", main__make_label(42));
    printf("%s\n", greet("World", "Hello"));
    printf("%s\n", greet("World", "Hi"));
    return 0;
}


