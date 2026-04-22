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

char* http_status_category(int code);
int http_status_is_redirect(int code);
int http_status_is_informational(int code);
int http_status_is_success(int code);
int http_should_retry(int code);
int main(void);

char* http_status_category(int code)
{
    if (((code / 100) == 1)) {
        return "informational";
    } else {
        if (((code / 100) == 2)) {
            return "success";
        } else {
            if (((code / 100) == 3)) {
                return "redirect";
            } else {
                if (((code / 100) == 4)) {
                    return "client_error";
                } else {
                    if (((code / 100) == 5)) {
                        return "server_error";
                    } else {
                        return "unknown";
                    }
                }
            }
        }
    }
}

int http_status_is_redirect(int code)
{
    if ((((((code == 301) || (code == 302)) || (code == 303)) || (code == 307)) || (code == 308))) {
        return 1;
    } else {
        return 0;
    }
}

int http_status_is_informational(int code)
{
    __auto_type _tern_1 = 0;
    if (((code >= 100) && (code < 200))) {
        _tern_1 = 1;
    }
    return _tern_1;
}

int http_status_is_success(int code)
{
    __auto_type _tern_2 = 0;
    if (((code >= 200) && (code < 300))) {
        _tern_2 = 1;
    }
    return _tern_2;
}

int http_should_retry(int code)
{
    if (((code == 429) || (code == 503))) {
        return 1;
    }
    return 0;
}

int main(void)
{
    int codes[] = {100, 101, 200, 201, 204, 206, 301, 302, 303, 304, 307, 308, 400, 401, 403, 404, 405, 408, 429, 500, 502, 503, 504, 0, 999};
    int n = 25;
    int i = 0;
    printf("%s\n", "=== HTTP Status Classification ===");
    i = 0;
    while ((i < n)) {
        int c = codes[i];
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", c);
        static char _cast_buf_1[64];
        snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", http_status_is_redirect(c));
        static char _cast_buf_2[64];
        snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", http_status_is_informational(c));
        static char _cast_buf_3[64];
        snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", http_status_is_success(c));
        static char _cast_buf_4[64];
        snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", http_should_retry(c));
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("code=", _cast_buf_0), " cat="), http_status_category(c)), " redirect="), _cast_buf_1), " info="), _cast_buf_2), " success="), _cast_buf_3), " retry="), _cast_buf_4));
        i = (i + 1);
    }
    printf("%s\n", "=== Range Tests ===");
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", (199 / 100));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", (200 / 100));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", (299 / 100));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", (300 / 100));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("199/100=", _cast_buf_5), " 200/100="), _cast_buf_6), " 299/100="), _cast_buf_7), " 300/100="), _cast_buf_8));
    return 0;
}


