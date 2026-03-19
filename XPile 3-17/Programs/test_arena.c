// @dr memory = arena

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// @dr memory = arena — bump allocator, bulk free on exit
typedef struct { char* base; size_t offset; size_t capacity; } __ul_arena_t;
static __ul_arena_t __ul_arena;
void __ul_arena_init(size_t capacity) {
    __ul_arena.base = (char*)malloc(capacity); __ul_arena.offset = 0; __ul_arena.capacity = capacity;
}
void* __ul_arena_alloc(size_t size) {
    size = (size + 7) & ~7;
    if (__ul_arena.offset + size > __ul_arena.capacity) { fprintf(stderr, "arena out of memory\n"); exit(1); }
    void* ptr = __ul_arena.base + __ul_arena.offset; __ul_arena.offset += size; return ptr;
}
void __ul_arena_reset() { __ul_arena.offset = 0; }
void __ul_arena_free() { free(__ul_arena.base); __ul_arena.base = NULL; }
#define __ul_malloc(size) __ul_arena_alloc(size)
#define __ul_free(ptr)    /* arena — no individual frees */

static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

int allocate_many(void);
int main(void);

int allocate_many(void)
{
    int total = 0;
    int i = 0;
    while ((i < 1000)) {
        static char _cast_buf_0[64];
        snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", i);
        char* s = __ul_strcat("item ", _cast_buf_0);
        total = (total + (int)strlen(s));
        i = (i + 1);
    }
    return total;
}

int main(void)
{
    __ul_arena_init(64 * 1024 * 1024);  /* 64 MB arena */
    int result = allocate_many();
    printf("%d\n", result);
    __ul_arena_free();
    return 0;
}


