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

char* repeat_char(char* ch, int n);
char* pad_str(char* s, int width);
char* build_line(int* widths, int cols, char* begin_s, char* fill, char* sep, char* end_s);
char* build_row(char** cells, int* widths, int cols, char* begin_s, char* sep, char* end_s, int padding);
void tabulate(char** data, int rows, int cols, char** headers, int fmt);
int main(void);

char* repeat_char(char* ch, int n)
{
    char* result = "";
    for (int i = 0; i < n; i++) {
        result = __ul_strcat(result, ch);
    }
    return result;
}

char* pad_str(char* s, int width)
{
    int slen = strlen(s);
    char* result = s;
    int pad = (width - slen);
    for (int i = 0; i < pad; i++) {
        result = __ul_strcat(result, " ");
    }
    return result;
}

char* build_line(int* widths, int cols, char* begin_s, char* fill, char* sep, char* end_s)
{
    char* line = begin_s;
    for (int c = 0; c < cols; c++) {
        line = __ul_strcat(line, repeat_char(fill, widths[c]));
        if ((c < (cols - 1))) {
            line = __ul_strcat(line, sep);
        }
    }
    line = __ul_strcat(line, end_s);
    return line;
}

char* build_row(char** cells, int* widths, int cols, char* begin_s, char* sep, char* end_s, int padding)
{
    char* line = begin_s;
    for (int c = 0; c < cols; c++) {
        for (int p = 0; p < padding; p++) {
            line = __ul_strcat(line, " ");
        }
        line = __ul_strcat(line, pad_str(cells[c], widths[c]));
        for (int p = 0; p < padding; p++) {
            line = __ul_strcat(line, " ");
        }
        if ((c < (cols - 1))) {
            line = __ul_strcat(line, sep);
        }
    }
    line = __ul_strcat(line, end_s);
    return line;
}

void tabulate(char** data, int rows, int cols, char** headers, int fmt)
{
    int widths[] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int c = 0; c < cols; c++) {
        widths[c] = strlen(headers[c]);
    }
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int cell_len = strlen(data[((r * cols) + c)]);
            if ((cell_len > widths[c])) {
                widths[c] = cell_len;
            }
        }
    }
    if ((fmt == 0)) {
        int padded[] = {0, 0, 0, 0, 0, 0, 0, 0};
        for (int c = 0; c < cols; c++) {
            padded[c] = (widths[c] + 2);
        }
        printf("%s\n", build_line(padded, cols, "+", "-", "+", "+"));
        printf("%s\n", build_row(headers, widths, cols, "|", "|", "|", 1));
        printf("%s\n", build_line(padded, cols, "+", "=", "+", "+"));
        for (int r = 0; r < rows; r++) {
            char* row_cells[] = {"", "", "", "", "", "", "", ""};
            for (int c = 0; c < cols; c++) {
                row_cells[c] = data[((r * cols) + c)];
            }
            printf("%s\n", build_row(row_cells, widths, cols, "|", "|", "|", 1));
            if ((r < (rows - 1))) {
                printf("%s\n", build_line(padded, cols, "+", "-", "+", "+"));
            }
        }
        printf("%s\n", build_line(padded, cols, "+", "-", "+", "+"));
    } else {
        printf("%s\n", build_row(headers, widths, cols, "", "  ", "", 0));
        printf("%s\n", build_line(widths, cols, "", "-", "  ", ""));
        for (int r = 0; r < rows; r++) {
            char* row_cells[] = {"", "", "", "", "", "", "", ""};
            for (int c = 0; c < cols; c++) {
                row_cells[c] = data[((r * cols) + c)];
            }
            printf("%s\n", build_row(row_cells, widths, cols, "", "  ", "", 0));
        }
    }
}

int main(void)
{
    printf("%s\n", "=== Test 1: Grid format ===");
    char* h1[] = {"Name", "Age", "City", "", "", "", "", ""};
    char* t1[] = {"Alice", "30", "New York", "Bob", "25", "Chicago", "Charlie", "35", "Houston", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
    tabulate(t1, 3, 3, h1, 0);
    printf("%s\n", "");
    printf("%s\n", "=== Test 2: Simple format ===");
    tabulate(t1, 3, 3, h1, 1);
    printf("%s\n", "");
    printf("%s\n", "=== Test 3: Wide columns ===");
    char* h3[] = {"Item", "Description", "Price", "Qty", "", "", "", ""};
    char* t3[] = {"Widget", "A small widget", "9.99", "100", "Gizmo", "Large industrial gizmo", "249.50", "5", "Bolt", "M8 hex bolt", "0.50", "10000", "", "", "", "", "", "", "", "", "", "", "", ""};
    tabulate(t3, 3, 4, h3, 0);
    return 0;
}


