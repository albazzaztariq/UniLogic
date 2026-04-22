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

int crc32_table[] = {0, 1996959894, 3993919788LL, 2567524794LL, 124634137, 1886057615, 3915621685LL, 2657392035LL, 249268274, 2044508325, 3772115227LL, 2547177864LL, 162941995, 2125683900, 3887607049LL, 2428443839LL};
int crc32_byte(int crc, int byte);
int get_crc_entry(int idx);
int crc32_xor_fold(int a, int b);
int crc32_loop_test(int init);
int fib(int n);
int reflect8(int v);
int main(void);

int crc32_byte(int crc, int byte)
{
    int idx = 0;
    int entry = 0;
    idx = ((crc ^ byte) & 15);
    entry = crc32_table[idx];
    crc = ((crc >> 4) ^ entry);
    return crc;
}

int get_crc_entry(int idx)
{
    if ((idx < 0)) {
        return 0;
    }
    if ((idx > 15)) {
        return 0;
    }
    return crc32_table[idx];
}

int crc32_xor_fold(int a, int b)
{
    return (a ^ b);
}

int crc32_loop_test(int init)
{
    int result = init;
    int n = 0;
    int found = 0;
    while ((n < 16)) {
        result = (result ^ crc32_table[n]);
        if ((result == 0)) {
            found = 1;
            n = 16;
        } else {
            n = (n + 1);
        }
    }
    return result;
}

int fib(int n)
{
    int a = 0;
    int b = 1;
    int tmp = 0;
    while ((n > 0)) {
        tmp = b;
        b = (a + b);
        a = tmp;
        n = (n - 1);
    }
    return a;
}

int reflect8(int v)
{
    int result = 0;
    int i = 0;
    while ((i < 8)) {
        result = ((result << 1) | (v & 1));
        v = (v >> 1);
        i = (i + 1);
    }
    return result;
}

int main(void)
{
    int i = 0;
    int r = 0;
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", crc32_table[0]);
    printf("%s\n", __ul_strcat("crc32_table[0] = ", _cast_buf_0));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", crc32_table[1]);
    printf("%s\n", __ul_strcat("crc32_table[1] = ", _cast_buf_1));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", crc32_table[7]);
    printf("%s\n", __ul_strcat("crc32_table[7] = ", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", crc32_table[15]);
    printf("%s\n", __ul_strcat("crc32_table[15] = ", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", get_crc_entry(0));
    printf("%s\n", __ul_strcat("entry[0] = ", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", get_crc_entry(8));
    printf("%s\n", __ul_strcat("entry[8] = ", _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", get_crc_entry((-1)));
    printf("%s\n", __ul_strcat("entry[-1] = ", _cast_buf_6));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", get_crc_entry(16));
    printf("%s\n", __ul_strcat("entry[16] = ", _cast_buf_7));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", crc32_xor_fold(57005, 48879));
    printf("%s\n", __ul_strcat("xor_fold(0xDEAD, 0xBEEF) = ", _cast_buf_8));
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", crc32_xor_fold(0, 0));
    printf("%s\n", __ul_strcat("xor_fold(0, 0) = ", _cast_buf_9));
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", crc32_xor_fold(65535, 65535));
    printf("%s\n", __ul_strcat("xor_fold(0xFFFF, 0xFFFF) = ", _cast_buf_10));
    r = crc32_byte(4294967295LL, 0);
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", r);
    printf("%s\n", __ul_strcat("crc32_byte(0xFFFFFFFF, 0) = ", _cast_buf_11));
    r = crc32_byte(0, 1);
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", r);
    printf("%s\n", __ul_strcat("crc32_byte(0, 1) = ", _cast_buf_12));
    r = crc32_loop_test(305419896);
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", r);
    printf("%s\n", __ul_strcat("crc32_loop_test(0x12345678) = ", _cast_buf_13));
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", fib(0));
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", fib(1));
    static char _cast_buf_16[64];
    snprintf(_cast_buf_16, sizeof(_cast_buf_16), "%d", fib(5));
    static char _cast_buf_17[64];
    snprintf(_cast_buf_17, sizeof(_cast_buf_17), "%d", fib(10));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("fib(0)=", _cast_buf_14), " fib(1)="), _cast_buf_15), " fib(5)="), _cast_buf_16), " fib(10)="), _cast_buf_17));
    static char _cast_buf_18[64];
    snprintf(_cast_buf_18, sizeof(_cast_buf_18), "%d", reflect8(1));
    printf("%s\n", __ul_strcat("reflect8(0x01) = ", _cast_buf_18));
    static char _cast_buf_19[64];
    snprintf(_cast_buf_19, sizeof(_cast_buf_19), "%d", reflect8(128));
    printf("%s\n", __ul_strcat("reflect8(0x80) = ", _cast_buf_19));
    static char _cast_buf_20[64];
    snprintf(_cast_buf_20, sizeof(_cast_buf_20), "%d", reflect8(171));
    printf("%s\n", __ul_strcat("reflect8(0xAB) = ", _cast_buf_20));
    r = 0;
    i = 0;
    while ((i < 8)) {
        r = (r ^ crc32_table[i]);
        i = (i + 1);
    }
    static char _cast_buf_21[64];
    snprintf(_cast_buf_21, sizeof(_cast_buf_21), "%d", r);
    printf("%s\n", __ul_strcat("xor_chain[0..7] = ", _cast_buf_21));
    return 0;
}


