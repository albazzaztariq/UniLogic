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
    __ul_list_int scores = {NULL, 0, 0};
    __ul_list_int_append(&scores, 10);
    __ul_list_int_append(&scores, 20);
    __ul_list_int_append(&scores, 30);
    printf("%d\n", scores.len);
    printf("%d\n", scores.data[0]);
    printf("%d\n", scores.data[1]);
    printf("%d\n", scores.data[2]);
    __ul_list_int_insert(&scores, 1, 15);
    printf("%d\n", scores.data[1]);
    printf("%d\n", scores.len);
    __ul_list_int_remove(&scores, 15);
    printf("%d\n", scores.len);
    int dropped = __ul_list_int_drop(&scores, 0);
    printf("%d\n", dropped);
    printf("%d\n", scores.len);
    __ul_list_int nums = {NULL, 0, 0};
    __ul_list_int_append(&nums, 5);
    __ul_list_int_append(&nums, 3);
    __ul_list_int_append(&nums, 1);
    __ul_list_int_append(&nums, 4);
    __ul_list_int_append(&nums, 2);
    printf("%d\n", nums.len);
    __ul_list_int_sort(&nums);
    printf("%d\n", nums.data[0]);
    printf("%d\n", nums.data[4]);
    __ul_list_int_reverse(&nums);
    printf("%d\n", nums.data[0]);
    printf("%d\n", nums.data[4]);
    if (__ul_list_int_contains(&nums, 3)) {
        printf("%s\n", "found 3");
    }
    __ul_list_int_clear(&nums);
    printf("%d\n", nums.len);
    __ul_list_int items = {NULL, 0, 0};
    __ul_list_int_append(&items, 100);
    __ul_list_int_append(&items, 200);
    __ul_list_int_append(&items, 300);
    int total = 0;
    for (int _i = 0; _i < items.len; _i++) {
        int x = items.data[_i];
        total = (total + x);
    }
    printf("%d\n", total);
    return 0;
}


