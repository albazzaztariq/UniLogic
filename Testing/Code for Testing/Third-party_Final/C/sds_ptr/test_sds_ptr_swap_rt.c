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

void reverse_buf(char* s, char* p);
int main(void);

void reverse_buf(char* s, char* p)
{
    int aux = 0;
    while ((strcmp(s, p) < 0)) {
        aux = (*s);
        (*s) = (*p);
        (*p) = aux;
        s = __ul_strcat(s, 1);
        p = (p - 1);
    }
}

int main(void)
{
    int buf1[] = {104, 101, 108, 108, 111, 0};
    reverse_buf(buf1, (buf1 + 4));
    printf("%s\n", __ul_strcat("reverse hello = ", buf1));
    int buf2[] = {97, 98, 99, 100, 101, 0};
    reverse_buf(buf2, (buf2 + 4));
    printf("%s\n", __ul_strcat("reverse abcde = ", buf2));
    int buf3[] = {122, 0};
    reverse_buf(buf3, buf3);
    printf("%s\n", __ul_strcat("reverse z = ", buf3));
    int buf4[] = {97, 98, 0};
    reverse_buf(buf4, (buf4 + 1));
    printf("%s\n", __ul_strcat("reverse ab = ", buf4));
    return 0;
}


