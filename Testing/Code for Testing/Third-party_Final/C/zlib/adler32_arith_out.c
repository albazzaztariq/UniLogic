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

int adler32_hello_world(void);
int adler32_empty(void);
int adler32_abc(void);
int adler32_lo(int adler);
int adler32_hi(int adler);
int adler32_combine(int adler1, int adler2, int len2);
int adler32_hello(void);
int adler32_world(void);
int main(void);

int adler32_hello_world(void)
{
    int adler = 1;
    int sum2 = 0;
    int b = 0;
    b = 72;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 101;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 108;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 108;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 111;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 44;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 32;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 87;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 111;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 114;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 108;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 100;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 33;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    adler = (adler % 65521);
    sum2 = (sum2 % 65521);
    return (adler | (sum2 << 16));
}

int adler32_empty(void)
{
    return 1;
}

int adler32_abc(void)
{
    int adler = 1;
    int sum2 = 0;
    int b = 0;
    b = 97;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 98;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 99;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    adler = (adler % 65521);
    sum2 = (sum2 % 65521);
    return (adler | (sum2 << 16));
}

int adler32_lo(int adler)
{
    return (adler & 65535);
}

int adler32_hi(int adler)
{
    return ((adler >> 16) & 65535);
}

int adler32_combine(int adler1, int adler2, int len2)
{
    int sum1 = 0;
    int sum2 = 0;
    int rem = 0;
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

int adler32_hello(void)
{
    int adler = 1;
    int sum2 = 0;
    int b = 0;
    b = 72;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 101;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 108;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 108;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 111;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    adler = (adler % 65521);
    sum2 = (sum2 % 65521);
    return (adler | (sum2 << 16));
}

int adler32_world(void)
{
    int adler = 1;
    int sum2 = 0;
    int b = 0;
    b = 44;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 32;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 87;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 111;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 114;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 108;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 100;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    b = 33;
    adler = (adler + b);
    sum2 = (sum2 + adler);
    adler = (adler % 65521);
    sum2 = (sum2 % 65521);
    return (adler | (sum2 << 16));
}

int main(void)
{
    int r = 0;
    int lo = 0;
    int hi = 0;
    int recombined = 0;
    int r1 = 0;
    int r2 = 0;
    int combined = 0;
    r = adler32_empty();
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", r);
    printf("%s\n", __ul_strcat("adler32(\"\") = ", _cast_buf_0));
    r = adler32_abc();
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", r);
    printf("%s\n", __ul_strcat("adler32(\"abc\") = ", _cast_buf_1));
    r = adler32_hello_world();
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", r);
    printf("%s\n", __ul_strcat("adler32(\"Hello, World!\") = ", _cast_buf_2));
    lo = adler32_lo(r);
    hi = adler32_hi(r);
    recombined = (lo | (hi << 16));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", lo);
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", hi);
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", recombined);
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", (recombined == r));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("lo=", _cast_buf_3), " hi="), _cast_buf_4), " recombined="), _cast_buf_5), " match="), _cast_buf_6));
    r1 = adler32_hello();
    r2 = adler32_world();
    combined = adler32_combine(r1, r2, 8);
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", r1);
    printf("%s\n", __ul_strcat("adler32(\"Hello\") = ", _cast_buf_7));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", r2);
    printf("%s\n", __ul_strcat("adler32(\", World!\") = ", _cast_buf_8));
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", combined);
    printf("%s\n", __ul_strcat("combined = ", _cast_buf_9));
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", r);
    printf("%s\n", __ul_strcat("direct   = ", _cast_buf_10));
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", (combined == r));
    printf("%s\n", __ul_strcat("match = ", _cast_buf_11));
    int x = 3735928559LL;
    int y = ((x >> 16) & 65535);
    int z = (x & 65535);
    int w = (y | (z << 16));
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", x);
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", y);
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", z);
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", w);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("bit_swap: x=", _cast_buf_12), " y="), _cast_buf_13), " z="), _cast_buf_14), " w="), _cast_buf_15));
    return 0;
}


