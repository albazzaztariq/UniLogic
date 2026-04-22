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

int read16_le(int* buf);
int read16_be(int* buf);
int read32_le(int* buf);
int read32_be(int* buf);
int main(void);

int read16_le(int* buf)
{
    return (buf[0] | (buf[1] << 8));
}

int read16_be(int* buf)
{
    return ((buf[0] << 8) | buf[1]);
}

int read32_le(int* buf)
{
    return (((buf[0] | (buf[1] << 8)) | (buf[2] << 16)) | (buf[3] << 24));
}

int read32_be(int* buf)
{
    return ((((buf[0] << 24) | (buf[1] << 16)) | (buf[2] << 8)) | buf[3]);
}

int main(void)
{
    int data1[] = {1, 2, 3, 4};
    int data2[] = {5, 0, 0, 0};
    int data3[] = {0, 0, 0, 0};
    int data4[] = {127, 127, 0, 0};
    printf("%s\n", "=== Endian Read Tests ===");
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", read16_le(data1));
    printf("%s\n", __ul_strcat("LE16 [01,02] = ", _cast_buf_0));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", read16_le(data2));
    printf("%s\n", __ul_strcat("LE16 [05,00] = ", _cast_buf_1));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", read16_le(data3));
    printf("%s\n", __ul_strcat("LE16 [00,00] = ", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", read16_le(data4));
    printf("%s\n", __ul_strcat("LE16 [7F,7F] = ", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", read16_be(data1));
    printf("%s\n", __ul_strcat("BE16 [01,02] = ", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", read16_be(data2));
    printf("%s\n", __ul_strcat("BE16 [05,00] = ", _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", read16_be(data3));
    printf("%s\n", __ul_strcat("BE16 [00,00] = ", _cast_buf_6));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", read16_be(data4));
    printf("%s\n", __ul_strcat("BE16 [7F,7F] = ", _cast_buf_7));
    int data5[] = {1, 0, 0, 0};
    int data6[] = {0, 0, 0, 1};
    int data7[] = {1, 2, 3, 4};
    int data8[] = {16, 32, 48, 64};
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", read32_le(data5));
    printf("%s\n", __ul_strcat("LE32 [01,00,00,00] = ", _cast_buf_8));
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", read32_le(data6));
    printf("%s\n", __ul_strcat("LE32 [00,00,00,01] = ", _cast_buf_9));
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", read32_le(data7));
    printf("%s\n", __ul_strcat("LE32 [01,02,03,04] = ", _cast_buf_10));
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", read32_le(data8));
    printf("%s\n", __ul_strcat("LE32 [10,20,30,40] = ", _cast_buf_11));
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", read32_be(data5));
    printf("%s\n", __ul_strcat("BE32 [01,00,00,00] = ", _cast_buf_12));
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", read32_be(data6));
    printf("%s\n", __ul_strcat("BE32 [00,00,00,01] = ", _cast_buf_13));
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", read32_be(data7));
    printf("%s\n", __ul_strcat("BE32 [01,02,03,04] = ", _cast_buf_14));
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", read32_be(data8));
    printf("%s\n", __ul_strcat("BE32 [10,20,30,40] = ", _cast_buf_15));
    return 0;
}


