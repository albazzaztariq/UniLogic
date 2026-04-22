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
static const char* Priority__names[3] = {"low", "medium", "high"};
static int Priority__values[3] = {1, 2, 3};
static const char* Priority__name(int val) {
    for (int i = 0; i < 3; i++) if (Priority__values[i] == val) return Priority__names[i];
    return "unknown";
}
static int Priority__valid(int val) {
    for (int i = 0; i < 3; i++) if (Priority__values[i] == val) return 1;
    return 0;
}

int main(void);

int main(void)
{
    int p = Priority_high;
    printf("%d\n", p);
    int q = Priority_low;
    printf("%d\n", q);
    if ((p > q)) {
        printf("%s\n", "high > low");
    }
    return 0;
}


