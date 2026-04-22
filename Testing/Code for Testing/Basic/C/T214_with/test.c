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

int get_value(void);
char* get_name(void);
int main(void);

int get_value(void)
{
    return 42;
}

char* get_name(void)
{
    return "resource";
}

int main(void)
{
    { /* with */
        int val = get_value();
        printf("%d\n", val);
        /* end with — cleanup val */
    }
    { /* with */
        int v = get_value();
        int doubled = (v * 2);
        printf("%d\n", doubled);
        /* end with — cleanup v */
    }
    { /* with */
        char* name = get_name();
        printf("%s\n", __ul_strcat("got: ", name));
        /* end with — cleanup name */
    }
    { /* with */
        int a = get_value();
        { /* with */
            int b = get_value();
            printf("%d\n", (a + b));
            /* end with — cleanup b */
        }
        /* end with — cleanup a */
    }
    printf("%s\n", "after with");
    return 0;
}


