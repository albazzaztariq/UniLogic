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

typedef struct Counter Counter;
typedef struct {
    int (*get_count)(Counter*);
    int (*increment)(Counter*);
} Counter_VTable;

struct Counter {
    Counter_VTable* _vtable;
    const char* __type;
    int count;
};

typedef struct DoubleCounter DoubleCounter;
typedef struct {
    int (*get_count)(DoubleCounter*);
    int (*increment)(DoubleCounter*);
    int (*double_increment)(DoubleCounter*);
} DoubleCounter_VTable;

struct DoubleCounter {
    Counter _base;
    DoubleCounter_VTable* _vtable;
    const char* __type;
};

int main(void);
int Counter_get_count(Counter* self);
int Counter_increment(Counter* self);
int DoubleCounter_double_increment(DoubleCounter* self);
static Counter_VTable _Counter_vtable;
static DoubleCounter_VTable _DoubleCounter_vtable;

int main(void)
{
    Counter c;
    c._vtable = &_Counter_vtable;
    c.__type = "Counter";
    c.count = 0;
    printf("%d\n", c._vtable->get_count(&c));
    printf("%d\n", c._vtable->increment(&c));
    printf("%d\n", c._vtable->increment(&c));
    printf("%d\n", c._vtable->get_count(&c));
    DoubleCounter dc;
    dc._vtable = &_DoubleCounter_vtable;
    dc.__type = "DoubleCounter";
    dc._base.count = 10;
    printf("%d\n", dc._vtable->get_count(&dc));
    printf("%d\n", dc._vtable->double_increment(&dc));
    return 0;
}

int Counter_get_count(Counter* self)
{
    return self->count;
}

int Counter_increment(Counter* self)
{
    self->count = (self->count + 1);
    return self->count;
}

int DoubleCounter_double_increment(DoubleCounter* self)
{
    self->_base.count = (self->_base.count + 2);
    return self->_base.count;
}

static Counter_VTable _Counter_vtable = {(void*)Counter_get_count, (void*)Counter_increment};
static DoubleCounter_VTable _DoubleCounter_vtable = {(void*)Counter_get_count, (void*)Counter_increment, (void*)DoubleCounter_double_increment};

