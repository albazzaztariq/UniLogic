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

typedef struct {
    Point top_left;
    Point bottom_right;
} Rectangle;

Point make_point(int px, int py);
int distance_sq(Point a, Point b);
int main(void);

Point make_point(int px, int py)
{
    Point p;
    p.x = px;
    p.y = py;
    return p;
}

int distance_sq(Point a, Point b)
{
    int dx = (b.x - a.x);
    int dy = (b.y - a.y);
    return ((dx * dx) + (dy * dy));
}

int main(void)
{
    Point p1;
    p1.x = 3;
    p1.y = 4;
    printf("%d\n", p1.x);
    printf("%d\n", p1.y);
    Point p2 = make_point(10, 20);
    printf("%d\n", p2.x);
    printf("%d\n", p2.y);
    int dist = distance_sq(p1, p2);
    printf("%d\n", dist);
    Rectangle r;
    r.top_left = make_point(0, 0);
    r.bottom_right = make_point(100, 50);
    printf("%d\n", r.top_left.x);
    printf("%d\n", r.bottom_right.y);
    p1.x = 99;
    printf("%d\n", p1.x);
    return 0;
}


