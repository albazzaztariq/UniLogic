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

int parse_hex4(char* input);
int main(void);

int parse_hex4(char* input)
{
    int h = 0;
    int i = 0;
    while ((i < 4)) {
        int c = (unsigned char)(input[i]);
        if (((c >= 48) && (c <= 57))) {
            h = (h + ((int)(c - 48)));
        } else {
            if (((c >= 65) && (c <= 70))) {
                h = (h + ((int)((10 + c) - 65)));
            } else {
                if (((c >= 97) && (c <= 102))) {
                    h = (h + ((int)((10 + c) - 97)));
                } else {
                    return 0;
                }
            }
        }
        if ((i < 3)) {
            h = (h << 4);
        }
        i = (i + 1);
    }
    return h;
}

int main(void)
{
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", parse_hex4("0000"));
    printf("%s\n", __ul_strcat("parse_hex4(\"0000\") = ", _cast_buf_0));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", parse_hex4("000F"));
    printf("%s\n", __ul_strcat("parse_hex4(\"000F\") = ", _cast_buf_1));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", parse_hex4("00FF"));
    printf("%s\n", __ul_strcat("parse_hex4(\"00FF\") = ", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", parse_hex4("0FFF"));
    printf("%s\n", __ul_strcat("parse_hex4(\"0FFF\") = ", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", parse_hex4("FFFF"));
    printf("%s\n", __ul_strcat("parse_hex4(\"FFFF\") = ", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", parse_hex4("1A2B"));
    printf("%s\n", __ul_strcat("parse_hex4(\"1A2B\") = ", _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", parse_hex4("dead"));
    printf("%s\n", __ul_strcat("parse_hex4(\"dead\") = ", _cast_buf_6));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", parse_hex4("BEEF"));
    printf("%s\n", __ul_strcat("parse_hex4(\"BEEF\") = ", _cast_buf_7));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", parse_hex4("4E2D"));
    printf("%s\n", __ul_strcat("parse_hex4(\"4E2D\") = ", _cast_buf_8));
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", parse_hex4("00GG"));
    printf("%s\n", __ul_strcat("parse_hex4(\"00GG\") = ", _cast_buf_9));
    return 0;
}


