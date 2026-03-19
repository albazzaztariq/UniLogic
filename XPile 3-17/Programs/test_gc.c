// @dr memory = gc

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// @dr memory = gc — Boehm GC (falls back to malloc if gc.h unavailable)
#if __has_include(<gc.h>)
#include <gc.h>
#define __ul_malloc(size) GC_malloc(size)
#define __ul_free(ptr)    /* GC handles it */
#define __UL_GC_AVAILABLE 1
#else
// WARNING: Boehm GC not found — using malloc, GC behavior not available
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
#define __UL_GC_AVAILABLE 0
#endif

static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

typedef struct Node Node;
typedef struct {
} Node_VTable;

struct Node {
    Node_VTable* _vtable;
    const char* __type;
    int value;
    char* label;
};

int make_nodes(int count);
int main(void);
static Node_VTable _Node_vtable;

int make_nodes(int count)
{
    int i = 0;
    int total = 0;
    while ((i < count)) {
        Node n;
        n._vtable = &_Node_vtable;
        n.__type = "Node";
        n.value = i;
        n.label = "node";
        total = (total + n.value);
        i = (i + 1);
    }
    return total;
}

int main(void)
{
    #if __UL_GC_AVAILABLE
        GC_INIT();
    #endif
    int result = make_nodes(100);
    printf("%d\n", result);
    return 0;
}

static Node_VTable _Node_vtable = {};

