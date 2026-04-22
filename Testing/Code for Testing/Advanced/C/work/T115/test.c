// @dr safety = checked

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
#include <stdlib.h>

#define __ul_checked_access(arr, idx, size, name, line) \
    ((unsigned)(idx) < (unsigned)(size) ? (arr)[(idx)] : \
    (fprintf(stderr, "bounds check error: %s[%d] out of range (size %d) at line %d\n", \
        (name), (int)(idx), (int)(size), (line)), exit(1), (arr)[0]))
#include <stdlib.h>

int triple(int x);
int add_ten(int x);
int flip_sign(int x);
int main(void);

int triple(int x)
{
    return (x * 3);
}

int add_ten(int x)
{
    return (x + 10);
}

int flip_sign(int x)
{
    return (0 - x);
}

int main(void)
{
    int a = triple(5);
    printf("%d\n", a);
    int b = add_ten(triple(5));
    printf("%d\n", b);
    int c = flip_sign(add_ten(triple(2)));
    printf("%d\n", c);
    int x = 10;
    if (!((x > 0))) {
        fprintf(stderr, "killswitch failed at line %d\n", 32);
        exit(1);
    }
    printf("%s\n", "killswitch passed");
    if (!((x == 10))) {
        fprintf(stderr, "killswitch failed at line %d\n", 35);
        exit(1);
    }
    printf("%s\n", "killswitch 2 passed");
    int r1 = (255 & 15);
    printf("%d\n", r1);
    int r2 = (240 | 15);
    printf("%d\n", r2);
    int r3 = (255 ^ 15);
    printf("%d\n", r3);
    int r4 = (1 << 4);
    printf("%d\n", r4);
    int r5 = (256 >> 4);
    printf("%d\n", r5);
    int r6 = (~0);
    printf("%d\n", r6);
    return 0;
}


