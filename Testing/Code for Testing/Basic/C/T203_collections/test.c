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
static int __ul_in_str(const char* val, const char** arr, int n) { for (int i = 0; i < n; i++) if (strcmp(arr[i], val) == 0) return 1; return 0; }
#include <stdlib.h>

typedef struct { int* data; int len; int cap; } __ul_list_int;
static void __ul_list_int_append(__ul_list_int* l, int val) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 8; l->data = (int*)realloc(l->data, l->cap * sizeof(int)); }
    l->data[l->len++] = val;
}
static int __ul_list_int_pop(__ul_list_int* l) { return l->data[--l->len]; }
static int __ul_list_int_drop(__ul_list_int* l, int i) {
    int val = l->data[i];
    memmove(&l->data[i], &l->data[i+1], (l->len - i - 1) * sizeof(int));
    l->len--; return val;
}
static void __ul_list_int_insert(__ul_list_int* l, int i, int val) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 8; l->data = (int*)realloc(l->data, l->cap * sizeof(int)); }
    memmove(&l->data[i+1], &l->data[i], (l->len - i) * sizeof(int));
    l->data[i] = val; l->len++;
}
static void __ul_list_int_remove(__ul_list_int* l, int val) {
    for (int i = 0; i < l->len; i++) { if (l->data[i] == val) {
        memmove(&l->data[i], &l->data[i+1], (l->len - i - 1) * sizeof(int)); l->len--; return; } }
}
static void __ul_list_int_clear(__ul_list_int* l) { l->len = 0; }
static void __ul_list_int_sort(__ul_list_int* l) { qsort(l->data, l->len, sizeof(int), __ul_cmp_int); }
static void __ul_list_int_reverse(__ul_list_int* l) {
    for (int i = 0, j = l->len - 1; i < j; i++, j--) { int tmp = l->data[i]; l->data[i] = l->data[j]; l->data[j] = tmp; }
}
static int __ul_list_int_contains(__ul_list_int* l, int val) {
    for (int i = 0; i < l->len; i++) if (l->data[i] == val) return 1; return 0;
}

int main(void);

int main(void)
{
    int nums[] = {10, 20, 30, 40, 50};
    printf("%d\n", nums[0]);
    printf("%d\n", nums[4]);
    int sum = 0;
    for (int _i = 0; _i < (int)(sizeof(nums)/sizeof(nums[0])); _i++) {
        int n = nums[_i];
        sum = (sum + n);
    }
    printf("%d\n", sum);
    __ul_list_int scores = {NULL, 0, 0};
    __ul_list_int_append(&scores, 100);
    __ul_list_int_append(&scores, 200);
    __ul_list_int_append(&scores, 300);
    printf("%d\n", scores.len);
    printf("%d\n", scores.data[0]);
    printf("%d\n", scores.data[2]);
    __ul_list_int_insert(&scores, 1, 150);
    printf("%d\n", scores.data[1]);
    __ul_list_int_remove(&scores, 150);
    printf("%d\n", scores.len);
    int dropped = __ul_list_int_drop(&scores, 0);
    printf("%d\n", dropped);
    __ul_list_int items = {NULL, 0, 0};
    __ul_list_int_append(&items, 5);
    __ul_list_int_append(&items, 10);
    __ul_list_int_append(&items, 15);
    int item_sum = 0;
    for (int _i = 0; _i < items.len; _i++) {
        int it = items.data[_i];
        item_sum = (item_sum + it);
    }
    printf("%d\n", item_sum);
    int _sn1 = (4) - (1);
    int _slice0[_sn1];
    memcpy(_slice0, &nums[1], _sn1 * sizeof(int));
    int* sliced = _slice0;
    printf("%d\n", sliced[0]);
    printf("%d\n", sliced[1]);
    printf("%d\n", sliced[2]);
    if (__ul_in_int(30, nums, sizeof(nums) / sizeof(nums[0]))) {
        printf("%s\n", "found");
    }
    return 0;
}


