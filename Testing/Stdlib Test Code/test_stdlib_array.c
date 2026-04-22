#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

int array_get(int* arr, int n, int idx);
int array_set(int* arr, int n, int idx, int val);
int array_contains(int* arr, int n, int val);
int array_index_of(int* arr, int n, int val);
int array_sum(int* arr, int n);
int array_min(int* arr, int n);
int array_max(int* arr, int n);
int array_reverse(int* arr, int n);
int array_sort(int* arr, int n);
int array_count(int* arr, int n, int val);
int main(void);

int array_get(int* arr, int n, int idx)
{
    if ((idx < 0)) {
        return 0;
    }
    if ((idx >= n)) {
        return 0;
    }
    return arr[idx];
}

int array_set(int* arr, int n, int idx, int val)
{
    if ((idx < 0)) {
        return 0;
    }
    if ((idx >= n)) {
        return 0;
    }
    arr[idx] = val;
    return 1;
}

int array_contains(int* arr, int n, int val)
{
    int i = 0;
    while ((i < n)) {
        if ((arr[i] == val)) {
            return 1;
        }
        i = (i + 1);
    }
    return 0;
}

int array_index_of(int* arr, int n, int val)
{
    int i = 0;
    while ((i < n)) {
        if ((arr[i] == val)) {
            return i;
        }
        i = (i + 1);
    }
    return (0 - 1);
}

int array_sum(int* arr, int n)
{
    int total = 0;
    int i = 0;
    while ((i < n)) {
        total = (total + arr[i]);
        i = (i + 1);
    }
    return total;
}

int array_min(int* arr, int n)
{
    int m = arr[0];
    int i = 1;
    while ((i < n)) {
        if ((arr[i] < m)) {
            m = arr[i];
        }
        i = (i + 1);
    }
    return m;
}

int array_max(int* arr, int n)
{
    int m = arr[0];
    int i = 1;
    while ((i < n)) {
        if ((arr[i] > m)) {
            m = arr[i];
        }
        i = (i + 1);
    }
    return m;
}

int array_reverse(int* arr, int n)
{
    int lo = 0;
    int hi = (n - 1);
    while ((lo < hi)) {
        int tmp = arr[lo];
        arr[lo] = arr[hi];
        arr[hi] = tmp;
        lo = (lo + 1);
        hi = (hi - 1);
    }
    return 0;
}

int array_sort(int* arr, int n)
{
    int i = 1;
    while ((i < n)) {
        int key = arr[i];
        int j = (i - 1);
        while ((j >= 0)) {
            if ((arr[j] > key)) {
                arr[(j + 1)] = arr[j];
                j = (j - 1);
            } else {
                break;
            }
        }
        arr[(j + 1)] = key;
        i = (i + 1);
    }
    return 0;
}

int array_count(int* arr, int n, int val)
{
    int count = 0;
    int i = 0;
    while ((i < n)) {
        if ((arr[i] == val)) {
            count = (count + 1);
        }
        i = (i + 1);
    }
    return count;
}

int main(void)
{
    int nums[] = {10, 30, 20, 50, 40};
    int val = array_get(nums, 5, 2);
    printf("%d\n", val);
    array_set(nums, 5, 2, 99);
    printf("%d\n", nums[2]);
    int has50 = array_contains(nums, 5, 50);
    printf("%d\n", has50);
    int has77 = array_contains(nums, 5, 77);
    printf("%d\n", has77);
    int idx = array_index_of(nums, 5, 50);
    printf("%d\n", idx);
    int data[] = {1, 2, 3, 4};
    int total = array_sum(data, 4);
    printf("%d\n", total);
    int lo = array_min(data, 4);
    printf("%d\n", lo);
    int hi = array_max(data, 4);
    printf("%d\n", hi);
    int reps[] = {1, 2, 3, 2, 2, 1};
    int twos = array_count(reps, 6, 2);
    printf("%d\n", twos);
    int rev[] = {1, 2, 3, 4};
    array_reverse(rev, 4);
    printf("%d\n", rev[0]);
    printf("%d\n", rev[1]);
    printf("%d\n", rev[2]);
    printf("%d\n", rev[3]);
    int unsorted[] = {5, 3, 1, 4, 2};
    array_sort(unsorted, 5);
    printf("%d\n", unsorted[0]);
    printf("%d\n", unsorted[1]);
    printf("%d\n", unsorted[2]);
    printf("%d\n", unsorted[3]);
    printf("%d\n", unsorted[4]);
    return 0;
}


