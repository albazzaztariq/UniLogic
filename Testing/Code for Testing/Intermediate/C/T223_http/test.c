#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

int parse_csv_line(char* line);
int main(void);

int parse_csv_line(char* line)
{
    int fields = 1;
    int len = strlen(line);
    int i = 0;
    while ((i < len)) {
        char* ch = str_char_at(line, i);
        if ((strcmp(ch, ",") == 0)) {
            fields = (fields + 1);
        }
        i = (i + 1);
    }
    return fields;
}

int main(void)
{
    char* response_status = "200";
    printf("%s\n", __ul_strcat("status: ", response_status));
    char* header = "name,age,city,score";
    char* row1 = "Alice,30,London,95";
    char* row2 = "Bob,25,Paris,88";
    char* row3 = "Charlie,35,Tokyo,92";
    int header_fields = parse_csv_line(header);
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", header_fields);
    printf("%s\n", __ul_strcat("fields: ", _cast_buf_0));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", parse_csv_line(row1));
    printf("%s\n", __ul_strcat("row1 fields: ", _cast_buf_1));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", parse_csv_line(row2));
    printf("%s\n", __ul_strcat("row2 fields: ", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", parse_csv_line(row3));
    printf("%s\n", __ul_strcat("row3 fields: ", _cast_buf_3));
    int total_rows = 3;
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", total_rows);
    printf("%s\n", __ul_strcat("total rows: ", _cast_buf_4));
    int valid = 1;
    if ((parse_csv_line(row1) != header_fields)) {
        valid = 0;
    }
    if ((parse_csv_line(row2) != header_fields)) {
        valid = 0;
    }
    if ((parse_csv_line(row3) != header_fields)) {
        valid = 0;
    }
    if (valid) {
        printf("%s\n", "data valid");
    } else {
        printf("%s\n", "data invalid");
    }
    return 0;
}


