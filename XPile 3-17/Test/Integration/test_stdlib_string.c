#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
#include <stdlib.h>

int str_len(char* s);
int str_equals(char* a, char* b);
int str_contains(char* s, char* sub);
int str_starts_with(char* s, char* prefix);
int str_ends_with(char* s, char* suffix);
char* str_upper(char* s);
char* str_lower(char* s);
char* str_trim(char* s);
char* str_concat(char* a, char* b);
int main(void);

int str_len(char* s)
{
    return strlen(s);
}

int str_equals(char* a, char* b)
{
    return (strcmp(a, b) == 0);
}

int str_contains(char* s, char* sub)
{
    char* found = strstr(s, sub);
    return (found != NULL);
}

int str_starts_with(char* s, char* prefix)
{
    int plen = strlen(prefix);
    return (strncmp(s, prefix, plen) == 0);
}

int str_ends_with(char* s, char* suffix)
{
    int slen = strlen(s);
    int suflen = strlen(suffix);
    if ((suflen > slen)) {
        return 0;
    }
    int offset = (slen - suflen);
    int i = 0;
    while ((i < suflen)) {
        if ((s[(offset + i)] != suffix[i])) {
            return 0;
        }
        i = (i + 1);
    }
    return 1;
}

char* str_upper(char* s)
{
    int len = strlen(s);
    char* result = malloc((len + 1));
    int i = 0;
    while ((i < len)) {
        int c = s[i];
        if ((c >= 97)) {
            if ((c <= 122)) {
                c = (c - 32);
            }
        }
        result[i] = c;
        i = (i + 1);
    }
    result[len] = 0;
    return result;
}

char* str_lower(char* s)
{
    int len = strlen(s);
    char* result = malloc((len + 1));
    int i = 0;
    while ((i < len)) {
        int c = s[i];
        if ((c >= 65)) {
            if ((c <= 90)) {
                c = (c + 32);
            }
        }
        result[i] = c;
        i = (i + 1);
    }
    result[len] = 0;
    return result;
}

char* str_trim(char* s)
{
    int len = strlen(s);
    int start = 0;
    while ((start < len)) {
        int c = s[start];
        if ((c != 32)) {
            if ((c != 9)) {
                if ((c != 10)) {
                    if ((c != 13)) {
                        break;
                    }
                }
            }
        }
        start = (start + 1);
    }
    int stop = (len - 1);
    while ((stop >= start)) {
        int c = s[stop];
        if ((c != 32)) {
            if ((c != 9)) {
                if ((c != 10)) {
                    if ((c != 13)) {
                        break;
                    }
                }
            }
        }
        stop = (stop - 1);
    }
    int newlen = ((stop - start) + 1);
    char* result = malloc((newlen + 1));
    int i = 0;
    while ((i < newlen)) {
        result[i] = s[(start + i)];
        i = (i + 1);
    }
    result[newlen] = 0;
    return result;
}

char* str_concat(char* a, char* b)
{
    int alen = strlen(a);
    int blen = strlen(b);
    char* result = malloc(((alen + blen) + 1));
    strcpy(result, a);
    strcat(result, b);
    return result;
}

int main(void)
{
    int len = str_len("hello");
    printf("%d\n", len);
    int eq = str_equals("abc", "abc");
    printf("%d\n", eq);
    int neq = str_equals("abc", "xyz");
    printf("%d\n", neq);
    int has = str_contains("hello world", "world");
    printf("%d\n", has);
    int nope = str_contains("hello world", "xyz");
    printf("%d\n", nope);
    int sw = str_starts_with("hello world", "hello");
    printf("%d\n", sw);
    int ew = str_ends_with("hello world", "world");
    printf("%d\n", ew);
    char* up = str_upper("hello");
    printf("%s\n", up);
    char* lo = str_lower("HELLO");
    printf("%s\n", lo);
    char* trimmed = str_trim("  hello  ");
    printf("%s\n", trimmed);
    char* joined = str_concat("hello ", "world");
    printf("%s\n", joined);
    return 0;
}


