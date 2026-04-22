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

int adler32_do4(int adler, int sum2, int b0, int b1, int b2, int b3);
int do1_step(int adler, int sum2, int byte_val);
int accumulate_flags(void);
int mod_base(int v);
int adler_mod_nodivide(int a);
int sum_stepped(int n);
int main(void);

int adler32_do4(int adler, int sum2, int b0, int b1, int b2, int b3)
{
    adler = (adler + b0);
    sum2 = (sum2 + adler);
    adler = (adler + b1);
    sum2 = (sum2 + adler);
    adler = (adler + b2);
    sum2 = (sum2 + adler);
    adler = (adler + b3);
    sum2 = (sum2 + adler);
    return (adler | (sum2 << 16));
}

int do1_step(int adler, int sum2, int byte_val)
{
    adler = (adler + byte_val);
    sum2 = (sum2 + adler);
    return (adler | (sum2 << 16));
}

int accumulate_flags(void)
{
    int flags = 0;
    flags = (flags + 1);
    flags = (flags + (1 << 2));
    flags = (flags + (2 << 4));
    flags = (flags + (1 << 8));
    flags = (flags + (1 << 16));
    return flags;
}

int mod_base(int v)
{
    return (v % 65521);
}

int adler_mod_nodivide(int a)
{
    int tmp = 0;
    tmp = (a >> 16);
    a = (a & 65535);
    a = ((a + (tmp << 4)) - tmp);
    tmp = (a >> 16);
    a = (a & 65535);
    a = ((a + (tmp << 4)) - tmp);
    if ((a >= 65521)) {
        a = (a - 65521);
    }
    return a;
}

int sum_stepped(int n)
{
    int total = 0;
    int running = 1;
    int i = 0;
    while ((i < n)) {
        running = (running + i);
        total = (total + running);
        i = (i + 1);
    }
    return total;
}

int main(void)
{
    int r = 0;
    r = adler32_do4(1, 0, 72, 101, 108, 108);
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", r);
    printf("%s\n", __ul_strcat("do4(Hell) = ", _cast_buf_0));
    r = do1_step(1, 0, 72);
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", r);
    printf("%s\n", __ul_strcat("do1(H) = ", _cast_buf_1));
    r = accumulate_flags();
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", r);
    printf("%s\n", __ul_strcat("accumulate_flags = ", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", mod_base(65521));
    printf("%s\n", __ul_strcat("mod_base(65521) = ", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", mod_base(65522));
    printf("%s\n", __ul_strcat("mod_base(65522) = ", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", mod_base(130000));
    printf("%s\n", __ul_strcat("mod_base(130000) = ", _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", adler_mod_nodivide(0));
    printf("%s\n", __ul_strcat("adler_mod_nodivide(0) = ", _cast_buf_6));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", adler_mod_nodivide(65521));
    printf("%s\n", __ul_strcat("adler_mod_nodivide(65521) = ", _cast_buf_7));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", adler_mod_nodivide(65522));
    printf("%s\n", __ul_strcat("adler_mod_nodivide(65522) = ", _cast_buf_8));
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", sum_stepped(5));
    printf("%s\n", __ul_strcat("sum_stepped(5) = ", _cast_buf_9));
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", sum_stepped(10));
    printf("%s\n", __ul_strcat("sum_stepped(10) = ", _cast_buf_10));
    return 0;
}


