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

int adler32_simple(int adler, char* buf, int len);
int adler32_combine(int adler1, int adler2, int len2);
int main(void);

int adler32_simple(int adler, char* buf, int len)
{
    int sum2 = 0;
    sum2 = ((adler >> 16) & 65535);
    adler = (adler & 65535);
    if ((strcmp(buf, 0) == 0)) {
        return 1;
    }
    while ((len > 0)) {
        adler = (adler + str_char_at(buf, 0));
        sum2 = (sum2 + adler);
        buf = __ul_strcat(buf, 1);
        len = (len - 1);
    }
    adler = (adler % 65521);
    sum2 = (sum2 % 65521);
    return (adler | (sum2 << 16));
}

int adler32_combine(int adler1, int adler2, int len2)
{
    int sum1 = 0;
    int sum2 = 0;
    int rem = 0;
    if ((len2 < 0)) {
        return 4294967295LL;
    }
    rem = (len2 % 65521);
    sum1 = (adler1 & 65535);
    sum2 = (rem * sum1);
    sum2 = (sum2 % 65521);
    sum1 = (((sum1 + (adler2 & 65535)) + 65521) - 1);
    sum2 = ((((sum2 + ((adler1 >> 16) & 65535)) + ((adler2 >> 16) & 65535)) + 65521) - rem);
    if ((sum1 >= 65521)) {
        sum1 = (sum1 - 65521);
    }
    if ((sum1 >= 65521)) {
        sum1 = (sum1 - 65521);
    }
    if ((sum2 >= (65521 * 2))) {
        sum2 = (sum2 - (65521 * 2));
    }
    if ((sum2 >= 65521)) {
        sum2 = (sum2 - 65521);
    }
    return (sum1 | (sum2 << 16));
}

int main(void)
{
    int r = adler32_simple(1, 0, 0);
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", r);
    printf("%s\n", __ul_strcat("adler32(\"\") = ", _cast_buf_0));
    int a = 0;
    a[0] = 65;
    r = adler32_simple(1, a, 1);
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", r);
    printf("%s\n", __ul_strcat("adler32(\"A\") = ", _cast_buf_1));
    int abc = 0;
    abc[0] = 97;
    abc[1] = 98;
    abc[2] = 99;
    r = adler32_simple(1, abc, 3);
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", r);
    printf("%s\n", __ul_strcat("adler32(\"abc\") = ", _cast_buf_2));
    int hw = 0;
    hw[0] = 72;
    hw[1] = 101;
    hw[2] = 108;
    hw[3] = 108;
    hw[4] = 111;
    hw[5] = 44;
    hw[6] = 32;
    hw[7] = 87;
    hw[8] = 111;
    hw[9] = 114;
    hw[10] = 108;
    hw[11] = 100;
    hw[12] = 33;
    r = adler32_simple(1, hw, 13);
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", r);
    printf("%s\n", __ul_strcat("adler32(\"Hello, World!\") = ", _cast_buf_3));
    int lo = (r & 65535);
    int hi = ((r >> 16) & 65535);
    int recombined = (lo | (hi << 16));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", r);
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", recombined);
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", (r == recombined));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("split-recombine: ", _cast_buf_4), " == "), _cast_buf_5), " -> "), _cast_buf_6));
    int hello = 0;
    hello[0] = 72;
    hello[1] = 101;
    hello[2] = 108;
    hello[3] = 108;
    hello[4] = 111;
    int world = 0;
    world[0] = 44;
    world[1] = 32;
    world[2] = 87;
    world[3] = 111;
    world[4] = 114;
    world[5] = 108;
    world[6] = 100;
    world[7] = 33;
    int r1 = adler32_simple(1, hello, 5);
    int r2 = adler32_simple(1, world, 8);
    int combined = adler32_combine(r1, r2, 8);
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", combined);
    printf("%s\n", __ul_strcat("adler32_combine result = ", _cast_buf_7));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", r);
    printf("%s\n", __ul_strcat("direct full result     = ", _cast_buf_8));
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", (combined == r));
    printf("%s\n", __ul_strcat("combine match: ", _cast_buf_9));
    return 0;
}


