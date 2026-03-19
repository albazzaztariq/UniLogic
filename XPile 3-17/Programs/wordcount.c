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

void wc(char* text);
int main(void);

void wc(char* text)
{
    int len = strlen(text);
    int lines = 0;
    int words = 0;
    int chars = len;
    int in_word = 0;
    for (int i = 0; i < len; i++) {
        char* ch = str_char_at(text, i);
        int is_space = 0;
        if ((strcmp(ch, " ") == 0)) {
            is_space = 1;
        }
        if ((strcmp(ch, "\t") == 0)) {
            is_space = 1;
        }
        if ((strcmp(ch, "r") == 0)) {
            is_space = 1;
        }
        if ((strcmp(ch, "\n") == 0)) {
            is_space = 1;
            lines = (lines + 1);
        }
        if ((is_space == 0)) {
            if ((in_word == 0)) {
                words = (words + 1);
            }
            in_word = 1;
        } else {
            in_word = 0;
        }
    }
    printf("%s\n", "  {lines} {words} {chars}");
}

int main(void)
{
    printf("%s\n", "Test 1: empty string");
    wc("");
    printf("%s\n", "Test 2: single word");
    wc("hello");
    printf("%s\n", "Test 3: simple sentence");
    wc("hello world");
    printf("%s\n", "Test 4: two lines");
    wc("The quick brown fox\njumps over the lazy dog\n");
    printf("%s\n", "Test 5: paragraphs with blank line");
    wc("First line\nSecond line\n\nFourth line after blank\n");
    printf("%s\n", "Test 6: whitespace only");
    wc("   \n\n   \n");
    return 0;
}


