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
static int __ul_in_int(int val, int* arr, int n) { for (int i = 0; i < n; i++) if (arr[i] == val) return 1; return 0; }
static int __ul_in_str(const char* val, char** arr, int n) { for (int i = 0; i < n; i++) if (strcmp(arr[i], val) == 0) return 1; return 0; }

int main(void);

int main(void)
{
    int nums[] = {10, 20, 30, 40, 50};
    if (__ul_in_int(30, nums, sizeof(nums) / sizeof(nums[0]))) {
        printf("%s\n", "found 30");
    }
    if (__ul_in_int(99, nums, sizeof(nums) / sizeof(nums[0]))) {
        printf("%s\n", "should not print");
    }
    char* fruits[] = {"apple", "banana", "mango"};
    if (__ul_in_str("banana", fruits, sizeof(fruits) / sizeof(fruits[0]))) {
        printf("%s\n", "found banana");
    }
    if (__ul_in_str("grape", fruits, sizeof(fruits) / sizeof(fruits[0]))) {
        printf("%s\n", "should not print");
    }
    return 0;
}


