// @dr python_compat = true
// @dr pyimport_0 = enum__IntEnum
// @dr pyimport_1 = enum__unique

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

enum ExitStatus { ExitStatus_SUCCESS = 0, ExitStatus_ERROR = 1, ExitStatus_ERROR_TIMEOUT = 2, ExitStatus_ERROR_HTTP_3XX = 3, ExitStatus_ERROR_HTTP_4XX = 4, ExitStatus_ERROR_HTTP_5XX = 5, ExitStatus_ERROR_TOO_MANY_REDIRECTS = 6, ExitStatus_PLUGIN_ERROR = 7, ExitStatus_ERROR_CTRL_C = 130 };
typedef enum ExitStatus ExitStatus;

ExitStatus http_status_to_exit_status(int http_status, int follow);
int main(void);

ExitStatus http_status_to_exit_status(int http_status, int follow)
{
    if ((((300 <= http_status) && (http_status <= 399)) && (!follow))) {
        return ExitStatus_ERROR_HTTP_3XX;
    } else {
        if (((400 <= http_status) && (http_status <= 499))) {
            return ExitStatus_ERROR_HTTP_4XX;
        } else {
            if (((500 <= http_status) && (http_status <= 599))) {
                return ExitStatus_ERROR_HTTP_5XX;
            } else {
                return ExitStatus_SUCCESS;
            }
        }
    }
}

int main(void)
{
    __auto_type cases = /* unknown expr ArrayLiteral */;
    for (int _i = 0; _i < (int)(sizeof(cases)/sizeof(cases[0])); _i++) {
        int __pair0 = cases[_i];
        __auto_type code = __pair0[0];
        __auto_type follow = __pair0[1];
        __auto_type result = http_status_to_exit_status(code, follow);
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", code);
        static char _cast_buf_1[64];
        snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", follow);
        static char _cast_buf_2[64];
        snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", result.name);
        static char _cast_buf_3[64];
        snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", result.value);
        printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("http_status_to_exit_status(", _cast_buf_0), ", "), _cast_buf_1), ") = "), _cast_buf_2), " ("), _cast_buf_3), ")"));
    }
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%s", "ExitStatus.SUCCESS:");
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", ExitStatus_SUCCESS.value);
    printf("%s\n", __ul_strcat(__ul_strcat(_cast_buf_4, " "), _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%s", "ExitStatus.ERROR_CTRL_C:");
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", ExitStatus_ERROR_CTRL_C.value);
    printf("%s\n", __ul_strcat(__ul_strcat(_cast_buf_6, " "), _cast_buf_7));
    return 0;
}


