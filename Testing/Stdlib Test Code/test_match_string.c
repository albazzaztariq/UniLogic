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

void greet(char* lang);
int main(void);

void greet(char* lang)
{
    if (strcmp(lang, "en") == 0) {
        printf("%s\n", "hello");
    } else if (strcmp(lang, "es") == 0) {
        printf("%s\n", "hola");
    } else if (strcmp(lang, "fr") == 0) {
        printf("%s\n", "bonjour");
    } else if (strcmp(lang, "de") == 0) {
        printf("%s\n", "hallo");
    } else {
        printf("%s\n", "unknown");
    }
}

int main(void)
{
    greet("en");
    greet("es");
    greet("fr");
    greet("de");
    greet("jp");
    char* code = "es";
    if (strcmp(code, "en") == 0) {
        printf("%s\n", "english");
    } else if (strcmp(code, "es") == 0) {
        printf("%s\n", "spanish");
    } else {
        printf("%s\n", "other");
    }
    return 0;
}


