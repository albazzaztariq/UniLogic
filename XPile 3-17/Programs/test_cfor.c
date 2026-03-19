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

int main(void);

int main(void)
{
    int sum1 = 0;
    {
        int i = 0;
        for (; (i < 5); i++) {
            sum1 = (sum1 + i);
        }
    }
    printf("%d\n", sum1);
    int sum2 = 0;
    {
        int j = 1;
        for (; (j <= 5); j++) {
            sum2 = (sum2 + j);
        }
    }
    printf("%d\n", sum2);
    int last = 0;
    {
        int k = 0;
        for (; (k < 100); k++) {
            if ((k == 7)) {
                last = k;
                break;
            }
        }
    }
    printf("%d\n", last);
    return 0;
}


