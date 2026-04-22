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

typedef struct { char** data; int len; int cap; } __ul_list_string;
static void __ul_list_string_append(__ul_list_string* l, char* val) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 8; l->data = (char**)realloc(l->data, l->cap * sizeof(char*)); }
    l->data[l->len++] = val;
}
static char* __ul_list_string_pop(__ul_list_string* l) { return l->data[--l->len]; }
static char* __ul_list_string_drop(__ul_list_string* l, int i) {
    char* val = l->data[i];
    memmove(&l->data[i], &l->data[i+1], (l->len - i - 1) * sizeof(char*));
    l->len--; return val;
}
static void __ul_list_string_insert(__ul_list_string* l, int i, char* val) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 8; l->data = (char**)realloc(l->data, l->cap * sizeof(char*)); }
    memmove(&l->data[i+1], &l->data[i], (l->len - i) * sizeof(char*));
    l->data[i] = val; l->len++;
}
static void __ul_list_string_remove(__ul_list_string* l, char* val) {
    for (int i = 0; i < l->len; i++) { if (strcmp(l->data[i], val) == 0) {
        memmove(&l->data[i], &l->data[i+1], (l->len - i - 1) * sizeof(char*)); l->len--; return; } }
}
static void __ul_list_string_clear(__ul_list_string* l) { l->len = 0; }
static void __ul_list_string_sort(__ul_list_string* l) { qsort(l->data, l->len, sizeof(char*), __ul_cmp_string); }
static void __ul_list_string_reverse(__ul_list_string* l) {
    for (int i = 0, j = l->len - 1; i < j; i++, j--) { char* tmp = l->data[i]; l->data[i] = l->data[j]; l->data[j] = tmp; }
}
static int __ul_list_string_contains(__ul_list_string* l, char* val) {
    for (int i = 0; i < l->len; i++) if (strcmp(l->data[i], val) == 0) return 1; return 0;
}

int main(void);

int main(void)
{
    int scores[] = {95, 87, 72, 100, 63};
    printf("%d\n", scores[0]);
    printf("%d\n", scores[4]);
    __ul_list_string names = {NULL, 0, 0};
    __ul_list_string_append(&names, "Alice");
    __ul_list_string_append(&names, "Bob");
    __ul_list_string_append(&names, "Carol");
    str_add(names, "Dave");
    printf("%s\n", names.data[0]);
    printf("%s\n", names.data[3]);
    int total = 0;
    for (int _i = 0; _i < (int)(sizeof(scores)/sizeof(scores[0])); _i++) {
        int s = scores[_i];
        total += s;
    }
    printf("%d\n", total);
    for (int _i = 0; _i < names.len; _i++) {
        char* name = names.data[_i];
        printf("%s\n", name);
    }
    return 0;
}


