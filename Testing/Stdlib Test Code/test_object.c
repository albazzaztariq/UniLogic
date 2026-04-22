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

typedef struct Animal Animal;
typedef struct {
    char* (*speak)(Animal*);
} Animal_VTable;

struct Animal {
    Animal_VTable* _vtable;
    const char* __type;
    char* name;
    int age;
};

typedef struct Dog Dog;
typedef struct {
    char* (*speak)(Dog*);
    char* (*info)(Dog*);
} Dog_VTable;

struct Dog {
    Animal _base;
    Dog_VTable* _vtable;
    const char* __type;
    char* breed;
};

int main(void);
char* Animal_speak(Animal* self);
char* Dog_speak(Dog* self);
char* Dog_info(Dog* self);
static Animal_VTable _Animal_vtable;
static Dog_VTable _Dog_vtable;

int main(void)
{
    Dog d;
    d._vtable = &_Dog_vtable;
    d.__type = "Dog";
    d._base.name = "Rex";
    d._base.age = 3;
    d.breed = "Labrador";
    printf("%s\n", d._base.name);
    printf("%d\n", d._base.age);
    printf("%s\n", d._vtable->speak(&d));
    printf("%s\n", d._vtable->info(&d));
    return 0;
}

char* Animal_speak(Animal* self)
{
    return "...";
}

char* Dog_speak(Dog* self)
{
    return "Woof!";
}

char* Dog_info(Dog* self)
{
    return self->breed;
}

static Animal_VTable _Animal_vtable = {(void*)Animal_speak};
static Dog_VTable _Dog_vtable = {(void*)Dog_speak, (void*)Dog_info};

