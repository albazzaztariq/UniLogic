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

char* reverse_string(char* s);
int is_palindrome(char* s);
int count_char(char* s, char* ch);
char* to_upper(char* s);
char* repeat_string(char* s, int times);
int main(void);

char* reverse_string(char* s)
{
    char* result = "";
    int i = ((int)strlen(s) - 1);
    while ((i >= 0)) {
        result = __ul_strcat(result, str_char_at(s, i));
        i = (i - 1);
    }
    return result;
}

int is_palindrome(char* s)
{
    return (strcmp(s, reverse_string(s)) == 0);
}

int count_char(char* s, char* ch)
{
    int count = 0;
    int i = 0;
    while ((i < (int)strlen(s))) {
        if ((strcmp(str_char_at(s, i), ch) == 0)) {
            count = (count + 1);
        }
        i = (i + 1);
    }
    return count;
}

char* to_upper(char* s)
{
    char* result = "";
    int i = 0;
    while ((i < (int)strlen(s))) {
        int c = ((int)(unsigned char)s[i]);
        if (((c >= 97) && (c <= 122))) {
            result = __ul_strcat(result, (__ul_char_from_code((c - 32))));
        } else {
            result = __ul_strcat(result, str_char_at(s, i));
        }
        i = (i + 1);
    }
    return result;
}

char* repeat_string(char* s, int times)
{
    char* result = "";
    int i = 0;
    while ((i < times)) {
        result = __ul_strcat(result, s);
        i = (i + 1);
    }
    return result;
}

int main(void)
{
    printf("%s\n", reverse_string("hello"));
    printf("%s\n", reverse_string("abcdef"));
    if (is_palindrome("racecar")) {
        printf("%s\n", "racecar is palindrome");
    } else {
        printf("%s\n", "racecar is not palindrome");
    }
    if (is_palindrome("hello")) {
        printf("%s\n", "hello is palindrome");
    } else {
        printf("%s\n", "hello is not palindrome");
    }
    printf("%d\n", count_char("mississippi", "s"));
    printf("%d\n", count_char("abcabc", "a"));
    printf("%s\n", to_upper("hello world"));
    printf("%s\n", to_upper("Python 3.12"));
    printf("%s\n", repeat_string("ab", 4));
    printf("%s\n", repeat_string("xyz", 2));
    return 0;
}


