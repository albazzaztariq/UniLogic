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

int array_sum(int* data, int count);
int array_min(int* data, int count);
int array_max(int* data, int count);
int array_range(int* data, int count);
void bubble_sort(int* data, int count);
int median_val(int* data, int count);
int main(void);

int array_sum(int* data, int count)
{
    int total = 0;
    int i = 0;
    while ((i < count)) {
        total = (total + data[i]);
        i = (i + 1);
    }
    return total;
}

int array_min(int* data, int count)
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

int array_max(int* data, int count)
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

int array_range(int* data, int count)
{
    return (array_max(data, count) - array_min(data, count));
}

void bubble_sort(int* data, int count)
{
    int i = 0;
    while ((i < (count - 1))) {
        int j = 0;
        while ((j < ((count - 1) - i))) {
            if ((data[j] > data[(j + 1)])) {
                int temp = data[j];
                data[j] = data[(j + 1)];
                data[(j + 1)] = temp;
            }
            j = (j + 1);
        }
        i = (i + 1);
    }
}

int median_val(int* data, int count)
{
    bubble_sort(data, count);
    return data[(count / 2)];
}

int main(void)
{
    int data[] = {23, 7, 42, 15, 8, 31, 19, 3, 50, 12};
    int count = 10;
    printf("%d\n", array_sum(data, count));
    printf("%d\n", array_min(data, count));
    printf("%d\n", array_max(data, count));
    printf("%d\n", array_range(data, count));
    printf("%d\n", median_val(data, count));
    printf("%d\n", data[0]);
    printf("%d\n", data[9]);
    int data2[] = {100, 200, 300, 400, 500};
    int count2 = 5;
    printf("%d\n", array_sum(data2, count2));
    printf("%d\n", array_min(data2, count2));
    printf("%d\n", array_max(data2, count2));
    return 0;
}


