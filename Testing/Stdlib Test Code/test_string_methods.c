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

int str_len(char* s);
int str_contains(char* s, char* sub);
int main(void);

int str_len(char* s)
{
    return strlen(s);
}

int str_contains(char* s, char* sub)
{
    char* found = strstr(s, sub);
    return (found != NULL);
}

int main(void)
{
    char* s = "Hello World";
    int len = (int)strlen(s);
    printf("%d\n", len);
    int has = str_contains(s, "World");
    printf("%d\n", has);
    return 0;
}


