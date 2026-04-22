#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* str_substr(const char* s, int start, int len) {
    int slen = (int)strlen(s); if (start < 0) start = 0; if (start >= slen) { static char e[1] = {0}; return e; }
    if (len > slen - start) len = slen - start; char* r = (char*)__ul_malloc(len + 1); memcpy(r, s + start, len); r[len] = 0; return r; }
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

int count_char(char* s, char* target);
char* first_word(char* s);
int main(void);

int count_char(char* s, char* target)
{
    int len = strlen(s);
    int count = 0;
    int i = 0;
    while ((i < len)) {
        char* ch = str_char_at(s, i);
        if ((strcmp(ch, target) == 0)) {
            count = (count + 1);
        }
        i = (i + 1);
    }
    return count;
}

char* first_word(char* s)
{
    int len = strlen(s);
    int i = 0;
    while ((i < len)) {
        char* ch = str_char_at(s, i);
        if ((strcmp(ch, " ") == 0)) {
            return str_substr(s, 0, i);
        }
        i = (i + 1);
    }
    return s;
}

int main(void)
{
    char* msg = "hello world";
    printf("%d\n", strlen(msg));
    char* ch0 = str_char_at(msg, 0);
    printf("%s\n", ch0);
    char* ch6 = str_char_at(msg, 6);
    printf("%s\n", ch6);
    char* sub = str_substr(msg, 0, 5);
    printf("%s\n", sub);
    printf("%d\n", count_char(msg, "l"));
    printf("%d\n", count_char(msg, "o"));
    printf("%s\n", first_word("foo bar baz"));
    printf("%s\n", first_word("single"));
    int data[] = {10, 20, 30, 40, 50};
    int total = 0;
    for (int _i = 0; _i < (int)(sizeof(data)/sizeof(data[0])); _i++) {
        int val = data[_i];
        total = (total + val);
    }
    printf("%d\n", total);
    __ul_list_int nums = {NULL, 0, 0};
    __ul_list_int_append(&nums, 9);
    __ul_list_int_append(&nums, 1);
    __ul_list_int_append(&nums, 5);
    __ul_list_int_append(&nums, 3);
    __ul_list_int_append(&nums, 7);
    __ul_list_int_sort(&nums);
    printf("%d\n", nums.data[0]);
    printf("%d\n", nums.data[4]);
    return 0;
}


