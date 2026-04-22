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

int cmark_utf8proc_is_space(int uc);
int main(void);

int cmark_utf8proc_is_space(int uc)
{
    return (((((((((((uc == 9) || (uc == 10)) || (uc == 12)) || (uc == 13)) || (uc == 32)) || (uc == 160)) || (uc == 5760)) || ((uc >= 8192) && (uc <= 8202))) || (uc == 8239)) || (uc == 8287)) || (uc == 12288));
}

int main(void)
{
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", cmark_utf8proc_is_space(9));
    printf("%s\n", __ul_strcat("is_space(9)  = ", _cast_buf_0));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", cmark_utf8proc_is_space(10));
    printf("%s\n", __ul_strcat("is_space(10) = ", _cast_buf_1));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", cmark_utf8proc_is_space(12));
    printf("%s\n", __ul_strcat("is_space(12) = ", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", cmark_utf8proc_is_space(13));
    printf("%s\n", __ul_strcat("is_space(13) = ", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", cmark_utf8proc_is_space(32));
    printf("%s\n", __ul_strcat("is_space(32) = ", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", cmark_utf8proc_is_space(160));
    printf("%s\n", __ul_strcat("is_space(160) = ", _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", cmark_utf8proc_is_space(5760));
    printf("%s\n", __ul_strcat("is_space(5760) = ", _cast_buf_6));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", cmark_utf8proc_is_space(8192));
    printf("%s\n", __ul_strcat("is_space(8192) = ", _cast_buf_7));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", cmark_utf8proc_is_space(8202));
    printf("%s\n", __ul_strcat("is_space(8202) = ", _cast_buf_8));
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", cmark_utf8proc_is_space(8239));
    printf("%s\n", __ul_strcat("is_space(8239) = ", _cast_buf_9));
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", cmark_utf8proc_is_space(8287));
    printf("%s\n", __ul_strcat("is_space(8287) = ", _cast_buf_10));
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", cmark_utf8proc_is_space(12288));
    printf("%s\n", __ul_strcat("is_space(12288) = ", _cast_buf_11));
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", cmark_utf8proc_is_space(65));
    printf("%s\n", __ul_strcat("is_space(65) = ", _cast_buf_12));
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", cmark_utf8proc_is_space(0));
    printf("%s\n", __ul_strcat("is_space(0) = ", _cast_buf_13));
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", cmark_utf8proc_is_space(8191));
    printf("%s\n", __ul_strcat("is_space(8191) = ", _cast_buf_14));
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", cmark_utf8proc_is_space(8203));
    printf("%s\n", __ul_strcat("is_space(8203) = ", _cast_buf_15));
    return 0;
}


