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
static char* str_substr(const char* s, int start, int len) {
    int slen = (int)strlen(s); if (start < 0) start = 0; if (start >= slen) { static char e[1] = {0}; return e; }
    if (len > slen - start) len = slen - start; char* r = (char*)__ul_malloc(len + 1); memcpy(r, s + start, len); r[len] = 0; return r; }
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
#include <stdlib.h>

int main(void);

int main(void)
{
    char* hello = __ul_strcat(__ul_strcat("Hello", " "), "World");
    printf("%s\n", hello);
    int len = strlen(hello);
    printf("%d\n", len);
    char* first = str_char_at(hello, 0);
    printf("%s\n", first);
    char* sub = str_substr(hello, 0, 5);
    printf("%s\n", sub);
    char* greeting = "Good Morning";
    int _sl1 = (4) - (0);
    char* _slice0 = (char*)__ul_malloc(_sl1 + 1);
    memcpy(_slice0, greeting + (0), _sl1);
    _slice0[_sl1] = 0;
    char* part = _slice0;
    printf("%s\n", part);
    char* a = "abc";
    char* b = "abc";
    if ((strcmp(a, b) == 0)) {
        printf("%s\n", "equal");
    }
    char* c = "xyz";
    if ((strcmp(a, c) != 0)) {
        printf("%s\n", "not equal");
    }
    char* sentence = "the quick brown fox";
    if ((strstr(sentence, "quick") != NULL)) {
        printf("%s\n", "found quick");
    }
    char* result = "";
    int i = 0;
    while ((i < 3)) {
        static char _cast_buf_1[64];
        snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", i);
        result = __ul_strcat(result, _cast_buf_1);
        i = (i + 1);
    }
    printf("%s\n", result);
    return 0;
}


