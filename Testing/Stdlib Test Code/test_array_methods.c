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

int array_sum(int* arr, int n);
int array_contains(int* arr, int n, int val);
int main(void);

int array_sum(int* arr, int n)
{
    int total = 0;
    int i = 0;
    while ((i < n)) {
        total = (total + arr[i]);
        i = (i + 1);
    }
    return total;
}

int array_contains(int* arr, int n, int val)
{
    int i = 0;
    while ((i < n)) {
        if ((arr[i] == val)) {
            return 1;
        }
        i = (i + 1);
    }
    return 0;
}

int main(void)
{
    int nums[] = {3, 1, 4, 1, 5};
    int total = array_sum(nums, 5);
    printf("%d\n", total);
    int found = array_contains(nums, 5, 4);
    printf("%d\n", found);
    int nope = array_contains(nums, 5, 9);
    printf("%d\n", nope);
    return 0;
}


