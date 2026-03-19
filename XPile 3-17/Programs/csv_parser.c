#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* str_substr(const char* s, int start, int len) {
    int slen = (int)strlen(s); if (start < 0) start = 0; if (start >= slen) { static char e[1] = {0}; return e; }
    if (len > slen - start) len = slen - start; char* r = (char*)__ul_malloc(len + 1); memcpy(r, s + start, len); r[len] = 0; return r; }
static int str_index_of(const char* s, const char* sub) { const char* p = strstr(s, sub); return p ? (int)(p - s) : -1; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

int parse_row(char* row, char** out);
int split_rows(char* csv, char** out);
char* get_cell(char** rows, int row, int col);
int get_column_index(char* header_row, char* col_name);
int main(void);

int parse_row(char* row, char** out)
{
    int field_count = 0;
    int pos = 0;
    int row_len = strlen(row);
    char* current = "";
    int in_quotes = 0;
    while ((pos < row_len)) {
        char* ch = str_char_at(row, pos);
        if (in_quotes) {
            if ((strcmp(ch, "\"") == 0)) {
                in_quotes = 0;
            } else {
                current = __ul_strcat(current, ch);
            }
        } else {
            if ((strcmp(ch, "\"") == 0)) {
                in_quotes = 1;
            } else {
                if ((strcmp(ch, ",") == 0)) {
                    out[field_count] = current;
                    field_count = (field_count + 1);
                    current = "";
                } else {
                    if ((strcmp(ch, "\n") != 0)) {
                        current = __ul_strcat(current, ch);
                    }
                }
            }
        }
        pos = (pos + 1);
    }
    out[field_count] = current;
    field_count = (field_count + 1);
    return field_count;
}

int split_rows(char* csv, char** out)
{
    int row_count = 0;
    int pos = 0;
    int csv_len = strlen(csv);
    char* current = "";
    while ((pos < csv_len)) {
        char* ch = str_char_at(csv, pos);
        if ((strcmp(ch, "\n") == 0)) {
            if ((strlen(current) > 0)) {
                out[row_count] = current;
                row_count = (row_count + 1);
            }
            current = "";
        } else {
            current = __ul_strcat(current, ch);
        }
        pos = (pos + 1);
    }
    if ((strlen(current) > 0)) {
        out[row_count] = current;
        row_count = (row_count + 1);
    }
    return row_count;
}

char* get_cell(char** rows, int row, int col)
{
    char* fields[] = {"", "", "", "", "", "", "", ""};
    parse_row(rows[row], fields);
    return fields[col];
}

int get_column_index(char* header_row, char* col_name)
{
    char* headers[] = {"", "", "", "", "", "", "", ""};
    int count = parse_row(header_row, headers);
    for (int i = 0; i < count; i++) {
        if ((strcmp(headers[i], col_name) == 0)) {
            return i;
        }
    }
    return (-1);
}

int main(void)
{
    char* csv = "Name,Age,City,Score\nAlice,30,New York,95\nBob,25,\"Los Angeles, CA\",87\nCharlie,35,Chicago,92\nDiana,28,Houston,88\nEve,32,\"Portland, OR\",91\n";
    char* rows[] = {"", "", "", "", "", "", "", "", "", ""};
    int num_rows = split_rows(csv, rows);
    printf("%s\n", "Total rows (including header):");
    printf("%d\n", num_rows);
    printf("%s\n", "");
    printf("%s\n", "=== CSV Data ===");
    char* headers[] = {"", "", "", "", "", "", "", ""};
    parse_row(rows[0], headers);
    printf("%s\n", headers[0]);
    printf("%s\n", headers[1]);
    printf("%s\n", headers[2]);
    printf("%s\n", headers[3]);
    printf("%s\n", "");
    for (int i = 1; i < num_rows; i++) {
        char* fields[] = {"", "", "", "", "", "", "", ""};
        parse_row(rows[i], fields);
        printf("%s\n", fields[0]);
        printf("%s\n", fields[1]);
        printf("%s\n", fields[2]);
        printf("%s\n", fields[3]);
        printf("%s\n", "---");
    }
    printf("%s\n", "");
    printf("%s\n", "=== Cell Lookups ===");
    char* cell1 = get_cell(rows, 1, 0);
    printf("%s\n", "Row 1, Col 0 (Name):");
    printf("%s\n", cell1);
    char* cell2 = get_cell(rows, 2, 2);
    printf("%s\n", "Row 2, Col 2 (City):");
    printf("%s\n", cell2);
    char* cell3 = get_cell(rows, 4, 3);
    printf("%s\n", "Row 4, Col 3 (Score):");
    printf("%s\n", cell3);
    printf("%s\n", "");
    printf("%s\n", "=== Column by Name ===");
    int city_col = get_column_index(rows[0], "City");
    printf("%s\n", "City column index:");
    printf("%d\n", city_col);
    printf("%s\n", "All cities:");
    for (int i = 1; i < num_rows; i++) {
        char* city = get_cell(rows, i, city_col);
        printf("%s\n", city);
    }
    return 0;
}


