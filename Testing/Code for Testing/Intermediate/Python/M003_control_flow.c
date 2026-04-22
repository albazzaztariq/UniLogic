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

char* classify(int score);
int main(void);

char* classify(int score)
{
    if ((score > 90)) {
        return "A";
    } else {
        if ((score > 80)) {
            return "B";
        } else {
            if ((score > 70)) {
                return "C";
            } else {
                return "F";
            }
        }
    }
}

int main(void)
{
    printf("%s\n", classify(95));
    printf("%s\n", classify(85));
    printf("%s\n", classify(72));
    printf("%s\n", classify(50));
    int count = 0;
    while ((count < 5)) {
        count++;
    }
    printf("%d\n", count);
    __ul_list_int nums = {NULL, 0, 0};
    __ul_list_int_append(&nums, 10);
    __ul_list_int_append(&nums, 20);
    __ul_list_int_append(&nums, 30);
    __ul_list_int_append(&nums, 40);
    __ul_list_int_append(&nums, 50);
    int total = 0;
    for (int _i = 0; _i < nums.len; _i++) {
        int n = nums.data[_i];
        total += n;
    }
    printf("%d\n", total);
    int code = 2;
    if (code == 1) {
        printf("%s\n", "one");
    } else if (code == 2) {
        printf("%s\n", "two");
    } else if (code == 3) {
        printf("%s\n", "three");
    } else {
        printf("%s\n", "other");
    }
    int i = 0;
    while ((i < 100)) {
        if ((i == 3)) {
            break;
        }
        i++;
    }
    printf("%d\n", i);
    return 0;
}


