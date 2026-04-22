#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

int area_rect(int w, int h);
int perimeter_rect(int w, int h);
int area_triangle(int base, int height);
int cm_to_mm(int cm);
int m_to_cm(int m);
int kg_to_grams(int kg);
char* format_result(char* label, int value);
char* format_pair(char* label, int a, int b);
int main(void);

int area_rect(int w, int h)
{
    return (w * h);
}

int perimeter_rect(int w, int h)
{
    return (2 * (w + h));
}

int area_triangle(int base, int height)
{
    return ((base * height) / 2);
}

int cm_to_mm(int cm)
{
    return (cm * 10);
}

int m_to_cm(int m)
{
    return (m * 100);
}

int kg_to_grams(int kg)
{
    return (kg * 1000);
}

char* format_result(char* label, int value)
{
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", value);
    return __ul_strcat(__ul_strcat(label, ": "), _cast_buf_0);
}

char* format_pair(char* label, int a, int b)
{
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", a);
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", b);
    return __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(label, ": "), _cast_buf_1), " x "), _cast_buf_2);
}

int main(void)
{
    int w = 5;
    int h = 3;
    printf("%s\n", format_result("Area", area_rect(w, h)));
    printf("%s\n", format_result("Perimeter", perimeter_rect(w, h)));
    printf("%s\n", format_result("Triangle area", area_triangle(10, 6)));
    printf("%s\n", format_result("2m in cm", m_to_cm(2)));
    printf("%s\n", format_result("15cm in mm", cm_to_mm(15)));
    printf("%s\n", format_result("3kg in grams", kg_to_grams(3)));
    printf("%s\n", format_pair("Rectangle", w, h));
    int area_mm = cm_to_mm(cm_to_mm(area_rect(2, 3)));
    printf("%s\n", format_result("Area in mm^2", area_mm));
    return 0;
}


