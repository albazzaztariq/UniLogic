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

int zipIntEncoding(int64_t value);
char* encName(int enc);
int main(void);

int zipIntEncoding(int64_t value)
{
    if (((value >= 0) && (value <= 12))) {
        return (241 + ((int)value));
    } else {
        if (((value >= (-128)) && (value <= 127))) {
            return 254;
        } else {
            if (((value >= (-32768)) && (value <= 32767))) {
                return (192 | (1 << 4));
            } else {
                if (((value >= (-8388608)) && (value <= 8388607))) {
                    return (192 | (4 << 4));
                } else {
                    if (((value >= (-2147483648LL)) && (value <= 2147483647))) {
                        return (192 | (2 << 4));
                    } else {
                        return (192 | (3 << 4));
                    }
                }
            }
        }
    }
}

char* encName(int enc)
{
    if (((enc >= 241) && (enc <= 253))) {
        return "IMM";
    }
    if ((enc == 254)) {
        return "INT8";
    }
    if ((enc == (192 | (1 << 4)))) {
        return "INT16";
    }
    if ((enc == (192 | (4 << 4)))) {
        return "INT24";
    }
    if ((enc == (192 | (2 << 4)))) {
        return "INT32";
    }
    if ((enc == (192 | (3 << 4)))) {
        return "INT64";
    }
    return "UNKNOWN";
}

int main(void)
{
    int64_t tests[] = {0, 1, 12, 13, (-1), (-128), 127, (-129), 128, (-32768), 32767, (-32769), 32768, (-8388608), 8388607, ((-8388608) - 1), (8388607 + 1), (-2147483648LL), 2147483647, (-2147483649LL), 2147483648LL, ((-9223372036854775807LL) - 1), 9223372036854775807LL};
    int n = ((int)(sizeof(tests) / sizeof(tests[0])));
    int i = 0;
    i = 0;
    while ((i < n)) {
        int enc = zipIntEncoding(tests[i]);
        printf("%s\n", encName(enc));
        i = (i + 1);
    }
    return 0;
}


