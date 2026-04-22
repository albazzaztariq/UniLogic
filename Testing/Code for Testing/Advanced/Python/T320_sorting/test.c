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

int bubble_sort(int* arr, int count);
int selection_sort(int* arr, int count);
int insertion_sort(int* arr, int count);
void array_copy(int* src, int* dst, int count);
int is_sorted(int* arr, int count);
void print_array(int* arr, int count);
int array_sum(int* arr, int count);
void test_sort(char* name, int* original, int* work, int count, int sort_type);
int main(void);

int bubble_sort(int* arr, int count)
{
    int comparisons = 0;
    int i = 0;
    while ((i < (count - 1))) {
        int j = 0;
        while ((j < ((count - 1) - i))) {
            comparisons = (comparisons + 1);
            if ((arr[j] > arr[(j + 1)])) {
                int temp = arr[j];
                arr[j] = arr[(j + 1)];
                arr[(j + 1)] = temp;
            }
            j = (j + 1);
        }
        i = (i + 1);
    }
    return comparisons;
}

int selection_sort(int* arr, int count)
{
    int comparisons = 0;
    int i = 0;
    while ((i < (count - 1))) {
        int min_idx = i;
        int j = (i + 1);
        while ((j < count)) {
            comparisons = (comparisons + 1);
            if ((arr[j] < arr[min_idx])) {
                min_idx = j;
            }
            j = (j + 1);
        }
        if ((min_idx != i)) {
            int temp = arr[i];
            arr[i] = arr[min_idx];
            arr[min_idx] = temp;
        }
        i = (i + 1);
    }
    return comparisons;
}

int insertion_sort(int* arr, int count)
{
    int comparisons = 0;
    int i = 1;
    while ((i < count)) {
        int key = arr[i];
        int j = (i - 1);
        int done = 0;
        while (((j >= 0) && (!done))) {
            comparisons = (comparisons + 1);
            if ((arr[j] > key)) {
                arr[(j + 1)] = arr[j];
                j = (j - 1);
            } else {
                done = 1;
            }
        }
        arr[(j + 1)] = key;
        i = (i + 1);
    }
    return comparisons;
}

void array_copy(int* src, int* dst, int count)
{
    int i = 0;
    while ((i < count)) {
        dst[i] = src[i];
        i = (i + 1);
    }
}

int is_sorted(int* arr, int count)
{
    int i = 0;
    while ((i < (count - 1))) {
        if ((arr[i] > arr[(i + 1)])) {
            return 0;
        }
        i = (i + 1);
    }
    return 1;
}

void print_array(int* arr, int count)
{
    char* result = "";
    int i = 0;
    while ((i < count)) {
        if ((i > 0)) {
            result = __ul_strcat(result, " ");
        }
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", arr[i]);
        result = __ul_strcat(result, _cast_buf_0);
        i = (i + 1);
    }
    printf("%s\n", result);
}

int array_sum(int* arr, int count)
{
    int total = 0;
    int i = 0;
    while ((i < count)) {
        total = (total + arr[i]);
        i = (i + 1);
    }
    return total;
}

void test_sort(char* name, int* original, int* work, int count, int sort_type)
{
    array_copy(original, work, count);
    int comparisons = 0;
    if ((sort_type == 1)) {
        comparisons = bubble_sort(work, count);
    }
    if ((sort_type == 2)) {
        comparisons = selection_sort(work, count);
    }
    if ((sort_type == 3)) {
        comparisons = insertion_sort(work, count);
    }
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", comparisons);
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", is_sorted(work, count));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", array_sum(work, count));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(name, ": comps="), _cast_buf_1), " sorted="), _cast_buf_2), " sum="), _cast_buf_3));
}

int main(void)
{
    int data[] = {64, 34, 25, 12, 22, 11, 90, 45, 0, 0};
    int count = 8;
    int work[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    printf("%s\n", "--- Dataset 1 ---");
    print_array(data, count);
    test_sort("Bubble", data, work, count, 1);
    test_sort("Selection", data, work, count, 2);
    test_sort("Insertion", data, work, count, 3);
    int data2[] = {5, 1, 4, 2, 8, 3, 7, 6, 9, 10};
    int count2 = 10;
    printf("%s\n", "--- Dataset 2 ---");
    print_array(data2, count2);
    test_sort("Bubble", data2, work, count2, 1);
    test_sort("Selection", data2, work, count2, 2);
    test_sort("Insertion", data2, work, count2, 3);
    int sorted_data[] = {1, 2, 3, 4, 5, 6, 7, 8, 0, 0};
    int count3 = 8;
    printf("%s\n", "--- Already sorted ---");
    test_sort("Bubble", sorted_data, work, count3, 1);
    test_sort("Insertion", sorted_data, work, count3, 3);
    int rev_data[] = {8, 7, 6, 5, 4, 3, 2, 1, 0, 0};
    printf("%s\n", "--- Reverse sorted ---");
    test_sort("Bubble", rev_data, work, count3, 1);
    test_sort("Insertion", rev_data, work, count3, 3);
    int single[] = {42, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    printf("%s\n", "--- Single ---");
    test_sort("Bubble", single, work, 1, 1);
    return 0;
}


