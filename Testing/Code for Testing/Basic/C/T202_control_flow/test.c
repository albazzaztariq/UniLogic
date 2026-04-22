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
    int x = 15;
    if ((x > 10)) {
        printf("%s\n", "greater");
    } else {
        printf("%s\n", "not greater");
    }
    int val = 50;
    if ((val > 100)) {
        printf("%s\n", "big");
    } else {
        if ((val > 25)) {
            printf("%s\n", "medium");
        } else {
            printf("%s\n", "small");
        }
    }
    int code = 2;
    if (code == 1) {
        printf("%s\n", "one");
    } else if (code == 2) {
        printf("%s\n", "two");
    } else if (code == 3) {
        printf("%s\n", "three");
    }
    int i = 0;
    int total = 0;
    while ((i < 5)) {
        total = (total + i);
        i = (i + 1);
    }
    printf("%d\n", total);
    int sum = 0;
    {
        int j = 1;
        for (; (j <= 5); j++) {
            sum = (sum + j);
        }
    }
    printf("%d\n", sum);
    int nums[] = {10, 20, 30};
    int arr_sum = 0;
    for (int _i = 0; _i < (int)(sizeof(nums)/sizeof(nums[0])); _i++) {
        int n = nums[_i];
        arr_sum = (arr_sum + n);
    }
    printf("%d\n", arr_sum);
    int count = 0;
    do {
        count = (count + 1);
    } while ((count < 3));
    printf("%d\n", count);
    int last = 0;
    int k = 0;
    while ((k < 100)) {
        if ((k == 5)) {
            last = k;
            break;
        }
        k = (k + 1);
    }
    printf("%d\n", last);
    return 0;
}


