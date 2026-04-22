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

int filter_positive(int* data, int count, int* out);
int filter_above(int* data, int count, int* out, int threshold);
int filter_below(int* data, int count, int* out, int threshold);
void transform_double(int* data, int count);
void transform_add(int* data, int count, int value);
void transform_clamp(int* data, int count, int lo, int hi);
int agg_sum(int* data, int count);
int agg_min(int* data, int count);
int agg_max(int* data, int count);
int agg_count_eq(int* data, int count, int target);
void print_array(int* data, int count);
void copy_array(int* src, int* dst, int count);
int main(void);

int filter_positive(int* data, int count, int* out)
{
    int out_count = 0;
    int i = 0;
    while ((i < count)) {
        if ((data[i] > 0)) {
            out[out_count] = data[i];
            out_count = (out_count + 1);
        }
        i = (i + 1);
    }
    return out_count;
}

int filter_above(int* data, int count, int* out, int threshold)
{
    int out_count = 0;
    int i = 0;
    while ((i < count)) {
        if ((data[i] > threshold)) {
            out[out_count] = data[i];
            out_count = (out_count + 1);
        }
        i = (i + 1);
    }
    return out_count;
}

int filter_below(int* data, int count, int* out, int threshold)
{
    int out_count = 0;
    int i = 0;
    while ((i < count)) {
        if ((data[i] < threshold)) {
            out[out_count] = data[i];
            out_count = (out_count + 1);
        }
        i = (i + 1);
    }
    return out_count;
}

void transform_double(int* data, int count)
{
    int i = 0;
    while ((i < count)) {
        data[i] = (data[i] * 2);
        i = (i + 1);
    }
}

void transform_add(int* data, int count, int value)
{
    int i = 0;
    while ((i < count)) {
        data[i] = (data[i] + value);
        i = (i + 1);
    }
}

void transform_clamp(int* data, int count, int lo, int hi)
{
    int i = 0;
    while ((i < count)) {
        if ((data[i] < lo)) {
            data[i] = lo;
        }
        if ((data[i] > hi)) {
            data[i] = hi;
        }
        i = (i + 1);
    }
}

int agg_sum(int* data, int count)
{
    int total = 0;
    int i = 0;
    while ((i < count)) {
        total = (total + data[i]);
        i = (i + 1);
    }
    return total;
}

int agg_min(int* data, int count)
{
    int result = data[0];
    int i = 1;
    while ((i < count)) {
        if ((data[i] < result)) {
            result = data[i];
        }
        i = (i + 1);
    }
    return result;
}

int agg_max(int* data, int count)
{
    int result = data[0];
    int i = 1;
    while ((i < count)) {
        if ((data[i] > result)) {
            result = data[i];
        }
        i = (i + 1);
    }
    return result;
}

int agg_count_eq(int* data, int count, int target)
{
    int result = 0;
    int i = 0;
    while ((i < count)) {
        if ((data[i] == target)) {
            result = (result + 1);
        }
        i = (i + 1);
    }
    return result;
}

void print_array(int* data, int count)
{
    char* result = "";
    int i = 0;
    while ((i < count)) {
        if ((i > 0)) {
            result = __ul_strcat(result, " ");
        }
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", data[i]);
        result = __ul_strcat(result, _cast_buf_0);
        i = (i + 1);
    }
    printf("%s\n", result);
}

void copy_array(int* src, int* dst, int count)
{
    int i = 0;
    while ((i < count)) {
        dst[i] = src[i];
        i = (i + 1);
    }
}

int main(void)
{
    int raw[] = {(-5), 12, 0, 8, (-3), 15, 7, (-1), 20, 3, 0, 11, (-8), 6, 25};
    int raw_count = 15;
    int buf[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    printf("%s\n", "--- Stage 1: Filter positive ---");
    int stage1_count = filter_positive(raw, raw_count, buf);
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", stage1_count);
    printf("%s\n", __ul_strcat("count: ", _cast_buf_1));
    print_array(buf, stage1_count);
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", agg_sum(buf, stage1_count));
    printf("%s\n", __ul_strcat("sum: ", _cast_buf_2));
    printf("%s\n", "--- Stage 2: Transform double ---");
    transform_double(buf, stage1_count);
    print_array(buf, stage1_count);
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", agg_sum(buf, stage1_count));
    printf("%s\n", __ul_strcat("sum: ", _cast_buf_3));
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", agg_min(buf, stage1_count));
    printf("%s\n", __ul_strcat("min: ", _cast_buf_4));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", agg_max(buf, stage1_count));
    printf("%s\n", __ul_strcat("max: ", _cast_buf_5));
    printf("%s\n", "--- Stage 3: Filter above 20 ---");
    int buf2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int stage3_count = filter_above(buf, stage1_count, buf2, 20);
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", stage3_count);
    printf("%s\n", __ul_strcat("count: ", _cast_buf_6));
    print_array(buf2, stage3_count);
    printf("%s\n", "--- Stage 4: Clamp to [25, 40] ---");
    transform_clamp(buf2, stage3_count, 25, 40);
    print_array(buf2, stage3_count);
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", agg_sum(buf2, stage3_count));
    printf("%s\n", __ul_strcat("sum: ", _cast_buf_7));
    printf("%s\n", "--- Pipeline 2: filter below 10, add 100 ---");
    int buf3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int p2_count = filter_below(raw, raw_count, buf3, 10);
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", p2_count);
    printf("%s\n", __ul_strcat("filtered: ", _cast_buf_8));
    transform_add(buf3, p2_count, 100);
    print_array(buf3, p2_count);
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", agg_sum(buf3, p2_count));
    printf("%s\n", __ul_strcat("sum: ", _cast_buf_9));
    printf("%s\n", "--- Aggregation ---");
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", agg_count_eq(raw, raw_count, 0));
    printf("%s\n", __ul_strcat("count 0 in raw: ", _cast_buf_10));
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", agg_count_eq(raw, raw_count, 12));
    printf("%s\n", __ul_strcat("count 12 in raw: ", _cast_buf_11));
    return 0;
}


