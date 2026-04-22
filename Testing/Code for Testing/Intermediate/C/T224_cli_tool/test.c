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

int find_task_index(char** names, int count, char* target);
int count_done(int* done, int count);
int starts_with(char* s, char* prefix);
char* format_line(int index, char* name, int is_done);
int main(void);

int find_task_index(char** names, int count, char* target)
{
    int i = 0;
    while ((i < count)) {
        if ((strcmp(names[i], target) == 0)) {
            return i;
        }
        i = (i + 1);
    }
    return (-1);
}

int count_done(int* done, int count)
{
    int result = 0;
    int i = 0;
    while ((i < count)) {
        if ((done[i] == 1)) {
            result = (result + 1);
        }
        i = (i + 1);
    }
    return result;
}

int starts_with(char* s, char* prefix)
{
    int slen = strlen(s);
    int plen = strlen(prefix);
    if ((plen > slen)) {
        return 0;
    }
    int i = 0;
    while ((i < plen)) {
        char* sc = str_char_at(s, i);
        char* pc = str_char_at(prefix, i);
        if ((strcmp(sc, pc) != 0)) {
            return 0;
        }
        i = (i + 1);
    }
    return 1;
}

char* format_line(int index, char* name, int is_done)
{
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", index);
    char* idx_str = _cast_buf_0;
    char* prefix = "";
    if ((is_done == 1)) {
        prefix = __ul_strcat(idx_str, ". [x] ");
    } else {
        prefix = __ul_strcat(idx_str, ". [ ] ");
    }
    return __ul_strcat(prefix, name);
}

int main(void)
{
    char* names[] = {"", "", "", "", "", "", "", "", "", ""};
    int done[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int task_count = 0;
    printf("%s\n", "--- Adding tasks ---");
    names[0] = "Buy groceries";
    names[1] = "Write report";
    names[2] = "Build prototype";
    names[3] = "Review code";
    names[4] = "Buy hardware";
    task_count = 5;
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", task_count);
    printf("%s\n", __ul_strcat("Tasks added: ", _cast_buf_1));
    printf("%s\n", "--- All tasks ---");
    int i = 0;
    while ((i < task_count)) {
        printf("%s\n", format_line(i, names[i], done[i]));
        i = (i + 1);
    }
    printf("%s\n", "--- Completing tasks ---");
    done[0] = 1;
    done[2] = 1;
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", count_done(done, task_count));
    printf("%s\n", __ul_strcat("Completed: ", _cast_buf_2));
    printf("%s\n", "--- Updated tasks ---");
    i = 0;
    while ((i < task_count)) {
        printf("%s\n", format_line(i, names[i], done[i]));
        i = (i + 1);
    }
    printf("%s\n", "--- Search: Buy ---");
    i = 0;
    while ((i < task_count)) {
        if (starts_with(names[i], "Buy")) {
            printf("%s\n", format_line(i, names[i], done[i]));
        }
        i = (i + 1);
    }
    printf("%s\n", "--- Find ---");
    int idx = find_task_index(names, task_count, "Review code");
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", idx);
    printf("%s\n", __ul_strcat("Found 'Review code' at index: ", _cast_buf_3));
    int idx2 = find_task_index(names, task_count, "Nonexistent");
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", idx2);
    printf("%s\n", __ul_strcat("Found 'Nonexistent' at index: ", _cast_buf_4));
    printf("%s\n", "--- Summary ---");
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", count_done(done, task_count));
    char* comp_str = _cast_buf_5;
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", task_count);
    char* total_str = _cast_buf_6;
    char* summary = __ul_strcat(__ul_strcat(__ul_strcat(comp_str, "/"), total_str), " completed");
    printf("%s\n", summary);
    return 0;
}


