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

uint8_t mbedtls_ct_uchar_in_range_if(uint8_t lo, uint8_t hi, uint8_t c, uint8_t val);
uint8_t mbedtls_ct_base64_enc_char(uint8_t value);
int mbedtls_ct_base64_dec_value(uint8_t c);
int mbedtls_base64_encode(char* dst, int dlen, int olen, char* src, int slen);
int main(void);

uint8_t mbedtls_ct_uchar_in_range_if(uint8_t lo, uint8_t hi, uint8_t c, uint8_t val)
{
    uint8_t in_range = 255;
    return ((int)(in_range & val));
}

uint8_t mbedtls_ct_base64_enc_char(uint8_t value)
{
    uint8_t digit = 0;
    digit = (digit | mbedtls_ct_uchar_in_range_if(0, 25, value, ((int)(65 + value))));
    digit = (digit | mbedtls_ct_uchar_in_range_if(26, 51, value, ((int)((97 + value) - 26))));
    digit = (digit | mbedtls_ct_uchar_in_range_if(52, 61, value, ((int)((48 + value) - 52))));
    digit = (digit | mbedtls_ct_uchar_in_range_if(62, 62, value, 43));
    digit = (digit | mbedtls_ct_uchar_in_range_if(63, 63, value, 47));
    return digit;
}

int mbedtls_ct_base64_dec_value(uint8_t c)
{
    uint8_t val = 0;
    val = (val | mbedtls_ct_uchar_in_range_if(65, 90, c, ((int)(((c - 65) + 0) + 1))));
    val = (val | mbedtls_ct_uchar_in_range_if(97, 122, c, ((int)(((c - 97) + 26) + 1))));
    val = (val | mbedtls_ct_uchar_in_range_if(48, 57, c, ((int)(((c - 48) + 52) + 1))));
    val = (val | mbedtls_ct_uchar_in_range_if(43, 43, c, ((int)(((c - 43) + 62) + 1))));
    val = (val | mbedtls_ct_uchar_in_range_if(47, 47, c, ((int)(((c - 47) + 63) + 1))));
    return ((int)(val - 1));
}

int mbedtls_base64_encode(char* dst, int dlen, int olen, char* src, int slen)
{
    int i = 0;
    int n = 0;
    int C1 = 0;
    int C2 = 0;
    int C3 = 0;
    char* p = "";
    if ((slen == 0)) {
        olen = 0;
        return 0;
    }
    n = ((slen / 3) + ((slen % 3) != 0));
    if ((n > ((2147483647 - 1) / 4))) {
        olen = 2147483647;
        return (-40);
    }
    n = (n * 4);
    if (((dlen < (n + 1)) || (strcmp(dst, 0) == 0))) {
        olen = (n + 1);
        return (-40);
    }
    n = ((slen / 3) * 3);
    i = 0;
    p = dst;
    while ((i < n)) {
        C1 = src;
        C2 = src;
        C3 = src;
        p = mbedtls_ct_base64_enc_char(((int)((C1 >> 2) & 63)));
        p = mbedtls_ct_base64_enc_char(((int)((((C1 & 3) << 4) + (C2 >> 4)) & 63)));
        p = mbedtls_ct_base64_enc_char(((int)((((C2 & 15) << 2) + (C3 >> 6)) & 63)));
        p = mbedtls_ct_base64_enc_char(((int)(C3 & 63)));
        i = (i + 3);
    }
    if ((i < slen)) {
        C1 = src;
        C2 = ((int)src);
        p = mbedtls_ct_base64_enc_char(((int)((C1 >> 2) & 63)));
        p = mbedtls_ct_base64_enc_char(((int)((((C1 & 3) << 4) + (C2 >> 4)) & 63)));
        if (((i + 1) < slen)) {
            p = mbedtls_ct_base64_enc_char(((int)(((C2 & 15) << 2) & 63)));
        } else {
            p = 61;
        }
        p = 61;
    }
    olen = ((int)(p - dst));
    p = 0;
    return 0;
}

int main(void)
{
    int i = 0;
    int ret = 0;
    int dec = 0;
    printf("%s\n", "=== mbedtls_ct_base64_enc_char ===");
    char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int pass = 1;
    i = 0;
    while ((i < 64)) {
        uint8_t got = mbedtls_ct_base64_enc_char(((int)i));
        if ((got != ((int)str_char_at(alphabet, i)))) {
            static char _cast_buf_0[64];
            snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", i);
            static char _cast_buf_1[64];
            snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", got);
            static char _cast_buf_2[64];
            snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%s", str_char_at(alphabet, i));
            printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("FAIL enc_char(", _cast_buf_0), "): got "), _cast_buf_1), " expected "), _cast_buf_2));
            pass = 0;
        }
        i = (i + 1);
    }
    printf("%s\n", __ul_strcat("enc_char 0..63: ", "PASS"));
    printf("%s\n", "\n=== mbedtls_ct_base64_dec_value ===");
    pass = 1;
    i = 0;
    while ((i < 64)) {
        uint8_t ch = ((int)str_char_at(alphabet, i));
        dec = mbedtls_ct_base64_dec_value(ch);
        if ((((int)dec) != i)) {
            static char _cast_buf_3[64];
            snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", ch);
            static char _cast_buf_4[64];
            snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", ch);
            static char _cast_buf_5[64];
            snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", dec);
            static char _cast_buf_6[64];
            snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", i);
            printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("FAIL dec_value('", _cast_buf_3), "'="), _cast_buf_4), "): got "), _cast_buf_5), " expected "), _cast_buf_6));
            pass = 0;
        }
        i = (i + 1);
    }
    printf("%s\n", __ul_strcat("dec_value round-trip: ", "PASS"));
    printf("%s\n", "\n=== dec_value invalid chars ===");
    uint8_t invalid[] = {33, 64, 35, 32, 10, 0, 128, 255};
    pass = 1;
    i = 0;
    while ((i < ((int)0))) {
        dec = mbedtls_ct_base64_dec_value(invalid[i]);
        if ((dec != (-1))) {
            static char _cast_buf_7[64];
            snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", invalid[i]);
            static char _cast_buf_8[64];
            snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", dec);
            printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("FAIL dec_value(0x", _cast_buf_7), "): got "), _cast_buf_8), " expected -1"));
            pass = 0;
        }
        i = (i + 1);
    }
    printf("%s\n", __ul_strcat("dec_value invalid: ", "PASS"));
    printf("%s\n", "\n=== mbedtls_base64_encode ===");
    uint8_t src[] = {0, 1, 2, 3, 4, 5};
    uint8_t dst = 0;
    int olen = 0;
    ret = mbedtls_base64_encode(dst, 0, olen, src, 6);
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", ret);
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", olen);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("encode ret=", _cast_buf_9), " olen="), _cast_buf_10), " out="), dst));
    printf("%s\n", __ul_strcat("encode 6 bytes: ", "PASS"));
    dst = 0;
    olen = 0;
    ret = mbedtls_base64_encode(dst, 0, olen, ((int)""), 0);
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", ret);
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", olen);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("encode empty: ret=", _cast_buf_11), " olen="), _cast_buf_12), " "), "PASS"));
    dst = 0;
    olen = 0;
    ret = mbedtls_base64_encode(dst, 4, olen, ((int)"Hello"), 5);
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", ret);
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", olen);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("encode too-small: ret=", _cast_buf_13), " olen="), _cast_buf_14), " "), "PASS"));
    dst = 0;
    olen = 0;
    ret = mbedtls_base64_encode(dst, 0, olen, ((int)"Man"), 3);
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", ret);
    static char _cast_buf_16[64];
    snprintf(_cast_buf_16, sizeof(_cast_buf_16), "%d", olen);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("encode 'Man': ret=", _cast_buf_15), " olen="), _cast_buf_16), " out="), dst), " "), "PASS"));
    return 0;
}


