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

typedef struct {
    uint8_t ready : 1;
    uint8_t fault : 1;
    uint8_t mode : 3;
    uint8_t priority : 3;
} StatusFlags;

int main(void);

int main(void)
{
    StatusFlags flags;
    flags.ready = 1;
    flags.fault = 0;
    flags.mode = 5;
    flags.priority = 3;
    printf("%d\n", flags.ready);
    printf("%d\n", flags.fault);
    printf("%d\n", flags.mode);
    printf("%d\n", flags.priority);
    return 0;
}


