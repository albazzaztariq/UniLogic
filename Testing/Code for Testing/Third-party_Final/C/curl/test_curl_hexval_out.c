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

int hex_char_val(int c);
int hex_pair_val(int hi, int lo);
int is_xdigit(int c);
int count_encoded_seqs(int* codes, int len);
int main(void);

int hex_char_val(int c)
{
    if (((c >= 48) && (c <= 57))) {
        return (c - 48);
    }
    if (((c >= 97) && (c <= 102))) {
        return ((c - 97) + 10);
    }
    if (((c >= 65) && (c <= 70))) {
        return ((c - 65) + 10);
    }
    return (-1);
}

int hex_pair_val(int hi, int lo)
{
    int h = hex_char_val(hi);
    int l = hex_char_val(lo);
    if (((h < 0) || (l < 0))) {
        return (-1);
    }
    return ((h << 4) | l);
}

int is_xdigit(int c)
{
    return ((((c >= 48) && (c <= 57)) || ((c >= 97) && (c <= 102))) || ((c >= 65) && (c <= 70)));
}

int count_encoded_seqs(int* codes, int len)
{
    int count = 0;
    int i = 0;
    while ((i < len)) {
        if (((((codes[i] == 37) && ((i + 2) < len)) && is_xdigit(codes[(i + 1)])) && is_xdigit(codes[(i + 2)]))) {
            count = (count + 1);
            i = (i + 3);
        } else {
            i = (i + 1);
        }
    }
    return count;
}

int main(void)
{
    int i = 0;
    printf("%s\n", "=== Hex Char Value Tests ===");
    i = 48;
    while ((i <= 57)) {
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", i);
        static char _cast_buf_1[64];
        snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", hex_char_val(i));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("code=", _cast_buf_0), " val="), _cast_buf_1));
        i = (i + 1);
    }
    i = 97;
    while ((i <= 102)) {
        static char _cast_buf_2[64];
        snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", i);
        static char _cast_buf_3[64];
        snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", hex_char_val(i));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("code=", _cast_buf_2), " val="), _cast_buf_3));
        i = (i + 1);
    }
    i = 65;
    while ((i <= 70)) {
        static char _cast_buf_4[64];
        snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", i);
        static char _cast_buf_5[64];
        snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", hex_char_val(i));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("code=", _cast_buf_4), " val="), _cast_buf_5));
        i = (i + 1);
    }
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", hex_char_val(103));
    printf("%s\n", __ul_strcat("code=103 val=", _cast_buf_6));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", hex_char_val(71));
    printf("%s\n", __ul_strcat("code=71 val=", _cast_buf_7));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", hex_char_val(32));
    printf("%s\n", __ul_strcat("code=32 val=", _cast_buf_8));
    printf("%s\n", "=== Hex Pair Value Tests ===");
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", hex_pair_val(50, 48));
    printf("%s\n", __ul_strcat("hi=50 lo=48 val=", _cast_buf_9));
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", hex_pair_val(52, 49));
    printf("%s\n", __ul_strcat("hi=52 lo=49 val=", _cast_buf_10));
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", hex_pair_val(70, 70));
    printf("%s\n", __ul_strcat("hi=70 lo=70 val=", _cast_buf_11));
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", hex_pair_val(48, 48));
    printf("%s\n", __ul_strcat("hi=48 lo=48 val=", _cast_buf_12));
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", hex_pair_val(97, 98));
    printf("%s\n", __ul_strcat("hi=97 lo=98 val=", _cast_buf_13));
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", hex_pair_val(71, 48));
    printf("%s\n", __ul_strcat("hi=71 lo=48 val=", _cast_buf_14));
    printf("%s\n", "=== Percent-Encoded Detection ===");
    int s1[] = {104, 101, 108, 108, 111, 37, 50, 48, 119, 111, 114, 108, 100};
    int s1len = 13;
    int s2[] = {37, 52, 49, 37, 52, 50, 37, 52, 51};
    int s2len = 9;
    int s3[] = {110, 111, 37, 101, 110, 99, 111, 100, 105, 110, 103};
    int s3len = 11;
    int s4[] = {116, 114, 97, 105, 108, 37, 50};
    int s4len = 7;
    int s5[] = {37, 50, 70, 37, 55, 69, 37, 50, 49};
    int s5len = 9;
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", count_encoded_seqs(s1, s1len));
    printf("%s\n", __ul_strcat("s1 count=", _cast_buf_15));
    static char _cast_buf_16[64];
    snprintf(_cast_buf_16, sizeof(_cast_buf_16), "%d", count_encoded_seqs(s2, s2len));
    printf("%s\n", __ul_strcat("s2 count=", _cast_buf_16));
    static char _cast_buf_17[64];
    snprintf(_cast_buf_17, sizeof(_cast_buf_17), "%d", count_encoded_seqs(s3, s3len));
    printf("%s\n", __ul_strcat("s3 count=", _cast_buf_17));
    static char _cast_buf_18[64];
    snprintf(_cast_buf_18, sizeof(_cast_buf_18), "%d", count_encoded_seqs(s4, s4len));
    printf("%s\n", __ul_strcat("s4 count=", _cast_buf_18));
    static char _cast_buf_19[64];
    snprintf(_cast_buf_19, sizeof(_cast_buf_19), "%d", count_encoded_seqs(s5, s5len));
    printf("%s\n", __ul_strcat("s5 count=", _cast_buf_19));
    return 0;
}


