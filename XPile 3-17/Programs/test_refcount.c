// @dr memory = refcount

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// @dr memory = refcount -- reference counting
typedef struct __ul_rc_header {
    int refcount;
    void (*destructor)(void*);
} __ul_rc_header;

static void* __ul_rc_alloc(size_t size) {
    __ul_rc_header* hdr = (__ul_rc_header*)malloc(sizeof(__ul_rc_header) + size);
    hdr->refcount = 1;
    hdr->destructor = NULL;
    return (void*)(hdr + 1);
}
static void __ul_retain(void* ptr) {
    if (!ptr) return;
    __ul_rc_header* hdr = ((__ul_rc_header*)ptr) - 1;
    hdr->refcount++;
}
static void __ul_release(void* ptr) {
    if (!ptr) return;
    __ul_rc_header* hdr = ((__ul_rc_header*)ptr) - 1;
    if (--hdr->refcount <= 0) {
        if (hdr->destructor) hdr->destructor(ptr);
        free(hdr);
    }
}
#define __ul_malloc(size) __ul_rc_alloc(size)
#define __ul_free(ptr)    __ul_release(ptr)

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
        __ul_release(s);
    }
    return total;
}

int main(void)
{
    int result = allocate_many();
    printf("%d\n", result);
    return 0;
}


