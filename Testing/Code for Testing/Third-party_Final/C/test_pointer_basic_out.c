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

void swap(int* a, int* b);
void increment(int* val);
int sum_array(int* arr, int n);
int main(void);

void swap(int* a, int* b)
{
    int tmp = (*a);
    (*a) = (*b);
    (*b) = tmp;
}

void increment(int* val)
{
    (*val) = ((*val) + 1);
}

int sum_array(int* arr, int n)
{
    int total = 0;
    int i = 0;
    i = 0;
    while ((i < n)) {
        total = (total + arr[i]);
        i = (i + 1);
    }
    return total;
}

int main(void)
{
    int x = 10;
    int y = 20;
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", x);
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", y);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("before swap: x=", _cast_buf_0), " y="), _cast_buf_1));
    swap((&x), (&y));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", x);
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", y);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("after swap: x=", _cast_buf_2), " y="), _cast_buf_3));
    int val = 5;
    increment((&val));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", val);
    printf("%s\n", __ul_strcat("after increment: val=", _cast_buf_4));
    int nums[] = {1, 2, 3, 4, 5};
    int s = sum_array(nums, 5);
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", s);
    printf("%s\n", __ul_strcat("sum: ", _cast_buf_5));
    int* ptr = (&x);
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", (*ptr));
    printf("%s\n", __ul_strcat("ptr deref: ", _cast_buf_6));
    (*ptr) = 99;
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", x);
    printf("%s\n", __ul_strcat("after deref assign: x=", _cast_buf_7));
    return 0;
}


