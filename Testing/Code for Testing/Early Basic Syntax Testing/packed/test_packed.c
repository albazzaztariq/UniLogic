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

typedef struct __attribute__((packed)) {
    uint8_t nibble_hi : 4;
    uint8_t nibble_lo : 4;
    uint8_t byte_val;
} PackedData;

int main(void);

int main(void)
{
    PackedData d;
    d.nibble_hi = 15;
    d.nibble_lo = 9;
    d.byte_val = 200;
    printf("%d\n", d.nibble_hi);
    printf("%d\n", d.nibble_lo);
    printf("%d\n", d.byte_val);
    return 0;
}


