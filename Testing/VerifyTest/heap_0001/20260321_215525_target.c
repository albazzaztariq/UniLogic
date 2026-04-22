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

uint32_t get_bin_index(uint64_t sz);
int main(void);

uint32_t get_bin_index(uint64_t sz)
{
    uint32_t index = 0;
    __auto_type _tern_1 = sz;
    if ((sz < 4)) {
        _tern_1 = 4;
    }
    sz = _tern_1;
    sz = (sz >> 1);
    while (sz) {
        index = (index + 1);
        sz = (sz >> 1);
    }
    index = (index - 2);
    if ((index > (9 - 1))) {
        index = (9 - 1);
    }
    return index;
}

int main(void)
{
    int i = 0;
    i = 1;
    while ((i <= 2048)) {
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", i);
        static char _cast_buf_1[64];
        snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%u", get_bin_index(((uint64_t)i)));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("size: ", _cast_buf_0), " -> bin: "), _cast_buf_1));
        i = (i + i);
    }
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%u", get_bin_index(4));
    printf("%s\n", __ul_strcat("size: 4 -> bin: ", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%u", get_bin_index(8));
    printf("%s\n", __ul_strcat("size: 8 -> bin: ", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%u", get_bin_index(128));
    printf("%s\n", __ul_strcat("size: 128 -> bin: ", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%u", get_bin_index(65536));
    printf("%s\n", __ul_strcat("size: 65536 -> bin: ", _cast_buf_5));
    return 0;
}


