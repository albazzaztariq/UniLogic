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

int curl_islower(int x);
int curl_isupper(int x);
int curl_isdigit(int x);
int curl_isalpha(int x);
int curl_isalnum(int x);
int curl_islowhexalpha(int x);
int curl_isuphexalpha(int x);
int curl_isxdigit(int x);
int curl_isurlpuntcs(int x);
int curl_isunreserved(int x);
int curl_isblank(int x);
int curl_isspace(int x);
int curl_is_token_char(int x);
int main(void);

int curl_islower(int x)
{
    return ((x >= 97) && (x <= 122));
}

int curl_isupper(int x)
{
    return ((x >= 65) && (x <= 90));
}

int curl_isdigit(int x)
{
    return ((x >= 48) && (x <= 57));
}

int curl_isalpha(int x)
{
    return (curl_islower(x) || curl_isupper(x));
}

int curl_isalnum(int x)
{
    return ((curl_isdigit(x) || curl_islower(x)) || curl_isupper(x));
}

int curl_islowhexalpha(int x)
{
    return ((x >= 97) && (x <= 102));
}

int curl_isuphexalpha(int x)
{
    return ((x >= 65) && (x <= 70));
}

int curl_isxdigit(int x)
{
    return ((curl_isdigit(x) || curl_islowhexalpha(x)) || curl_isuphexalpha(x));
}

int curl_isurlpuntcs(int x)
{
    return ((((x == 45) || (x == 46)) || (x == 95)) || (x == 126));
}

int curl_isunreserved(int x)
{
    return (curl_isalnum(x) || curl_isurlpuntcs(x));
}

int curl_isblank(int x)
{
    return ((x == 32) || (x == 9));
}

int curl_isspace(int x)
{
    return (curl_isblank(x) || ((x >= 10) && (x <= 13)));
}

int curl_is_token_char(int x)
{
    if (((x <= 32) || (x >= 127))) {
        return 0;
    }
    if ((((((((((((((((((x == 40) || (x == 41)) || (x == 44)) || (x == 47)) || (x == 58)) || (x == 59)) || (x == 60)) || (x == 61)) || (x == 62)) || (x == 63)) || (x == 64)) || (x == 91)) || (x == 92)) || (x == 93)) || (x == 123)) || (x == 125)) || (x == 34))) {
        return 0;
    } else {
        return 1;
    }
}

int main(void)
{
    int i = 0;
    int chars[] = {48, 57, 97, 102, 122, 65, 70, 90, 103, 71, 45, 46, 95, 126, 43, 47, 32, 9, 10, 13, 33, 35, 37, 38};
    int n = 24;
    printf("%s\n", "=== curl ctype checks ===");
    i = 0;
    while ((i < n)) {
        int c1 = chars[i];
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", c1);
        static char _cast_buf_1[64];
        snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", curl_isalpha(c1));
        static char _cast_buf_2[64];
        snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", curl_isdigit(c1));
        static char _cast_buf_3[64];
        snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", curl_isxdigit(c1));
        static char _cast_buf_4[64];
        snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", curl_isalnum(c1));
        static char _cast_buf_5[64];
        snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", curl_isunreserved(c1));
        static char _cast_buf_6[64];
        snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", curl_isspace(c1));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("chr=", _cast_buf_0), " alpha="), _cast_buf_1), " digit="), _cast_buf_2), " xdigit="), _cast_buf_3), " alnum="), _cast_buf_4), " unresvd="), _cast_buf_5), " space="), _cast_buf_6));
        i = (i + 1);
    }
    printf("%s\n", "=== token char check ===");
    int tkchars[] = {97, 90, 48, 45, 95, 40, 58, 32, 10};
    int tn = 9;
    i = 0;
    while ((i < tn)) {
        int c2 = tkchars[i];
        static char _cast_buf_7[64];
        snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", c2);
        static char _cast_buf_8[64];
        snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", curl_is_token_char(c2));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("chr=", _cast_buf_7), " token="), _cast_buf_8));
        i = (i + 1);
    }
    return 0;
}


