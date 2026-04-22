#define MYLIB_VERSION "1.2.3.4"
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

typedef struct {
    int x;
    int y;
} Point;

char* errmsg[] = {"ok", "stream error", "data error", "mem error", "buf error"};
char* get_err_msg(int code);
char* my_version(void);
int size_flag(int sz);
int category(int c);
char* decode_code(int code);
int sum_bytes_dowhile(int count);
int point_dist_sq(Point p);
Point point_add(Point a, Point b);
int combine_flags(int a, int b);
int main(void);

char* get_err_msg(int code)
{
    int idx = 0;
    if (((code < 0) || (code > 4))) {
        idx = 4;
    } else {
        idx = code;
    }
    return errmsg[idx];
}

char* my_version(void)
{
    return MYLIB_VERSION;
}

int size_flag(int sz)
{
    int flag = 0;
    if ((sz == 2)) {
    } else {
        if ((sz == 4)) {
            flag = 1;
        } else {
            if ((sz == 8)) {
                flag = 2;
            } else {
                flag = 3;
            }
        }
    }
    return flag;
}

int category(int c)
{
    int result = 0;
    if ((((c == 1) || (c == 2)) || (c == 3))) {
        result = 10;
    } else {
        if (((c == 4) || (c == 5))) {
            result = 20;
        } else {
            result = 99;
        }
    }
    return result;
}

char* decode_code(int code)
{
    if ((code == 0)) {
        return "ok";
    } else {
        if ((code == 1)) {
            return "stream end";
        } else {
            if ((code == (-1))) {
                return "errno";
            } else {
                if ((code == (-2))) {
                    return "stream error";
                } else {
                    if ((code == (-3))) {
                        return "data error";
                    } else {
                        return "unknown";
                    }
                }
            }
        }
    }
}

int sum_bytes_dowhile(int count)
{
    int n = 0;
    int sum = 0;
    do {
        sum = (sum + n);
        n = (n + 1);
    } while ((n < count));
    return sum;
}

int point_dist_sq(Point p)
{
    return ((p.x * p.x) + (p.y * p.y));
}

Point point_add(Point a, Point b)
{
    Point result = {0};
    result.x = (a.x + b.x);
    result.y = (a.y + b.y);
    return result;
}

int combine_flags(int a, int b)
{
    return (((int)a) + ((int)b));
}

int main(void)
{
    printf("%s\n", __ul_strcat("version: ", my_version()));
    printf("%s\n", __ul_strcat("errmsg[0]: ", get_err_msg(0)));
    printf("%s\n", __ul_strcat("errmsg[2]: ", get_err_msg(2)));
    printf("%s\n", __ul_strcat("errmsg[-1]: ", get_err_msg((-1))));
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", size_flag(2));
    printf("%s\n", __ul_strcat("size_flag(2)=", _cast_buf_0));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", size_flag(4));
    printf("%s\n", __ul_strcat("size_flag(4)=", _cast_buf_1));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", size_flag(8));
    printf("%s\n", __ul_strcat("size_flag(8)=", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", size_flag(1));
    printf("%s\n", __ul_strcat("size_flag(1)=", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", category(1));
    printf("%s\n", __ul_strcat("category(1)=", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", category(2));
    printf("%s\n", __ul_strcat("category(2)=", _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", category(4));
    printf("%s\n", __ul_strcat("category(4)=", _cast_buf_6));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", category(9));
    printf("%s\n", __ul_strcat("category(9)=", _cast_buf_7));
    printf("%s\n", __ul_strcat("decode(0): ", decode_code(0)));
    printf("%s\n", __ul_strcat("decode(1): ", decode_code(1)));
    printf("%s\n", __ul_strcat("decode(-2): ", decode_code((-2))));
    printf("%s\n", __ul_strcat("decode(99): ", decode_code(99)));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", sum_bytes_dowhile(5));
    printf("%s\n", __ul_strcat("sum_bytes_dowhile(5) = ", _cast_buf_8));
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", sum_bytes_dowhile(1));
    printf("%s\n", __ul_strcat("sum_bytes_dowhile(1) = ", _cast_buf_9));
    Point p = {0};
    p.x = 3;
    p.y = 4;
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", point_dist_sq(p));
    printf("%s\n", __ul_strcat("dist_sq(3,4) = ", _cast_buf_10));
    Point a = {0};
    a.x = 1;
    a.y = 2;
    Point b = {0};
    b.x = 10;
    b.y = 20;
    Point c = point_add(a, b);
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", c.x);
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", c.y);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("point_add: (", _cast_buf_11), ","), _cast_buf_12), ")"));
    int flags = combine_flags(100, 200);
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", flags);
    printf("%s\n", __ul_strcat("combine_flags(100,200) = ", _cast_buf_13));
    return 0;
}


