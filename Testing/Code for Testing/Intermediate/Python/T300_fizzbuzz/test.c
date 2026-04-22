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

char* fizzbuzz(int n);
int count_fizz(int limit);
int count_buzz(int limit);
int count_fizzbuzz(int limit);
int main(void);

char* fizzbuzz(int n)
{
    if (((n % 15) == 0)) {
        return "FizzBuzz";
    } else {
        if (((n % 3) == 0)) {
            return "Fizz";
        } else {
            if (((n % 5) == 0)) {
                return "Buzz";
            } else {
                static char _cast_buf_0[64];
                snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", n);
                return _cast_buf_0;
            }
        }
    }
}

int count_fizz(int limit)
{
    int count = 0;
    int i = 1;
    while ((i <= limit)) {
        if ((strcmp(fizzbuzz(i), "Fizz") == 0)) {
            count = (count + 1);
        }
        i = (i + 1);
    }
    return count;
}

int count_buzz(int limit)
{
    int count = 0;
    int i = 1;
    while ((i <= limit)) {
        if ((strcmp(fizzbuzz(i), "Buzz") == 0)) {
            count = (count + 1);
        }
        i = (i + 1);
    }
    return count;
}

int count_fizzbuzz(int limit)
{
    int count = 0;
    int i = 1;
    while ((i <= limit)) {
        if ((strcmp(fizzbuzz(i), "FizzBuzz") == 0)) {
            count = (count + 1);
        }
        i = (i + 1);
    }
    return count;
}

int main(void)
{
    int i = 1;
    while ((i <= 20)) {
        printf("%s\n", fizzbuzz(i));
        i = (i + 1);
    }
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", count_fizz(100));
    printf("%s\n", __ul_strcat("Fizz count: ", _cast_buf_1));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", count_buzz(100));
    printf("%s\n", __ul_strcat("Buzz count: ", _cast_buf_2));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", count_fizzbuzz(100));
    printf("%s\n", __ul_strcat("FizzBuzz count: ", _cast_buf_3));
    return 0;
}


