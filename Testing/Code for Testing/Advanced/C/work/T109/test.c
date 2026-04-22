#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* __ul_char_from_code(int code) {
    char* r = (char*)__ul_malloc(2); r[0] = (char)code; r[1] = 0; return r; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }
static void __ul_print_float(double v) {
    char _buf[64]; snprintf(_buf, sizeof(_buf), "%.10g", v);
    int _has_dot = 0; for (int _i = 0; _buf[_i]; _i++) if (_buf[_i] == '.' || _buf[_i] == 'e') { _has_dot = 1; break; }
    if (!_has_dot) { int _l = (int)strlen(_buf); _buf[_l] = '.'; _buf[_l+1] = '0'; _buf[_l+2] = 0; }
    printf("%s\n", _buf); }
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

int double_it(int x);
int is_even(int x);
int main(void);

int double_it(int x)
{
    return (x * 2);
}

int is_even(int x)
{
    return ((x % 2) == 0);
}

int main(void)
{
    printf("%d\n", (10 + 5));
    printf("%d\n", (10 - 3));
    printf("%d\n", (6 * 7));
    printf("%d\n", (20 / 4));
    printf("%d\n", (17 % 5));
    int neg = (0 - 42);
    printf("%d\n", neg);
    printf("%d\n", (7 / 2));
    int result = (2 + (3 * 4));
    printf("%d\n", result);
    __ul_list_int nums = {NULL, 0, 0};
    __ul_list_int_append(&nums, 1);
    __ul_list_int_append(&nums, 2);
    __ul_list_int_append(&nums, 3);
    __ul_list_int_append(&nums, 4);
    __ul_list_int_append(&nums, 5);
    __ul_list_int _map_0 = {NULL, 0, 0};
    for (int _i = 0; _i < nums.len; _i++) __ul_list_int_append(&_map_0, double_it(nums.data[_i]));
    __ul_list_int doubled = _map_0;
    for (int _i = 0; _i < doubled.len; _i++) {
        int d = doubled.data[_i];
        printf("%d\n", d);
    }
    __ul_list_int _filt_1 = {NULL, 0, 0};
    for (int _i = 0; _i < nums.len; _i++) if (is_even(nums.data[_i])) __ul_list_int_append(&_filt_1, nums.data[_i]);
    __ul_list_int evens = _filt_1;
    for (int _i = 0; _i < evens.len; _i++) {
        int e = evens.data[_i];
        printf("%d\n", e);
    }
    __ul_list_int unsorted = {NULL, 0, 0};
    __ul_list_int_append(&unsorted, 5);
    __ul_list_int_append(&unsorted, 2);
    __ul_list_int_append(&unsorted, 8);
    __ul_list_int_append(&unsorted, 1);
    __ul_list_int_append(&unsorted, 9);
    __ul_list_int_sort(&unsorted);
    for (int _i = 0; _i < unsorted.len; _i++) {
        int s = unsorted.data[_i];
        printf("%d\n", s);
    }
    return 0;
}


