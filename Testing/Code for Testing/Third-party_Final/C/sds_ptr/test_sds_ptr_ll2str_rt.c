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

int sds_ll2str(char* s, int value);
int sds_ull2str(char* s, uint64_t v);
int main(void);

int sds_ll2str(char* s, int value)
{
    char* p = "";
    int aux = 0;
    uint64_t v = 0;
    int l = 0;
    if ((value < 0)) {
        if ((value != ((-9223372036854775807LL) - 1))) {
            v = ((uint64_t)(-value));
        } else {
            v = (((uint64_t)9223372036854775807LL) + 1);
        }
    } else {
        v = ((uint64_t)value);
    }
    p = s;
    do {
        (*p) = (48 + ((int)(v % 10)));
        v = (v / 10);
    } while (v);
    if ((value < 0)) {
        (*p) = 45;
    }
    l = ((int)(p - s));
    (*p) = 0;
    p = (p - 1);
    while ((strcmp(s, p) < 0)) {
        aux = (*s);
        (*s) = (*p);
        (*p) = aux;
        s = __ul_strcat(s, 1);
        p = (p - 1);
    }
    return l;
}

int sds_ull2str(char* s, uint64_t v)
{
    char* p = "";
    int aux = 0;
    int l = 0;
    p = s;
    do {
        (*p) = (48 + ((int)(v % 10)));
        v = (v / 10);
    } while (v);
    l = ((int)(p - s));
    (*p) = 0;
    p = (p - 1);
    while ((strcmp(s, p) < 0)) {
        aux = (*s);
        (*s) = (*p);
        (*p) = aux;
        s = __ul_strcat(s, 1);
        p = (p - 1);
    }
    return l;
}

int main(void)
{
    int buf[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int len = 0;
    len = sds_ll2str(buf, 0);
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", len);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("ll2str(0) = \"", buf), "\" len="), _cast_buf_0));
    len = sds_ll2str(buf, 42);
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", len);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("ll2str(42) = \"", buf), "\" len="), _cast_buf_1));
    len = sds_ll2str(buf, (-42));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", len);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("ll2str(-42) = \"", buf), "\" len="), _cast_buf_2));
    len = sds_ll2str(buf, 1234567890);
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", len);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("ll2str(1234567890) = \"", buf), "\" len="), _cast_buf_3));
    len = sds_ll2str(buf, (-9999));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", len);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("ll2str(-9999) = \"", buf), "\" len="), _cast_buf_4));
    len = sds_ull2str(buf, 0);
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", len);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("ull2str(0) = \"", buf), "\" len="), _cast_buf_5));
    len = sds_ull2str(buf, 18446744073709551615ULL);
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", len);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("ull2str(ULLONG_MAX) = \"", buf), "\" len="), _cast_buf_6));
    return 0;
}


