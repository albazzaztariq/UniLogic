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

enum Priority { Priority_low = 1, Priority_medium = 2, Priority_high = 3 };

enum Color { Color_red = 10, Color_green = 20, Color_blue = 30 };

int main(void);

int main(void)
{
    int p = Priority_high;
    printf("%d\n", p);
    int total = ((Priority_low + Priority_medium) + Priority_high);
    printf("%d\n", total);
    if ((p == Priority_high)) {
        printf("%d\n", 1);
    }
    int c = Color_green;
    printf("%d\n", c);
    int val = Priority_medium;
    if (val == 1) {
        printf("%d\n", 100);
    } else if (val == 2) {
        printf("%d\n", 200);
    } else if (val == 3) {
        printf("%d\n", 300);
    }
    return 0;
}


