// @dr safety = checked

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
#include <stdlib.h>

#define __ul_checked_access(arr, idx, size, name, line) \
    ((unsigned)(idx) < (unsigned)(size) ? (arr)[(idx)] : \
    (fprintf(stderr, "bounds check error: %s[%d] out of range (size %d) at line %d\n", \
        (name), (int)(idx), (int)(size), (line)), exit(1), (arr)[0]))
#include <stdlib.h>

int main(void);

int main(void)
{
    int index = 3;
    int length = 10;
    if (!((index < length))) {
        fprintf(stderr, "killswitch failed at line %d\n", 9);
        exit(1);
    }
    printf("%s\n", "test1: passed");
    if (!((length > 0))) {
        fprintf(stderr, "killswitch failed at line %d\n", 13);
        exit(1);
    }
    printf("%s\n", "test2: passed");
    int x = 5;
    if (!((x == 5))) {
        fprintf(stderr, "killswitch failed at line %d\n", 19);
        exit(1);
    }
    printf("%s\n", "test3: passed");
    return 0;
}


