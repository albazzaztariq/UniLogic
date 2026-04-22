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
    char* (*describe)(Animal*);
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
    char* (*describe)(Dog*);
    char* (*fetch)(Dog*);
} Dog_VTable;

struct Dog {
    Animal _base;
    Dog_VTable* _vtable;
    const char* __type;
    char* breed;
};

typedef struct Cat Cat;
typedef struct {
    char* (*speak)(Cat*);
    char* (*describe)(Cat*);
} Cat_VTable;

struct Cat {
    Animal _base;
    Cat_VTable* _vtable;
    const char* __type;
};

int main(void);
char* Animal_speak(Animal* self);
char* Animal_describe(Animal* self);
char* Dog_speak(Dog* self);
char* Dog_fetch(Dog* self);
char* Cat_speak(Cat* self);
static Animal_VTable _Animal_vtable;
static Dog_VTable _Dog_vtable;
static Cat_VTable _Cat_vtable;

int main(void)
{
    Animal a;
    a._vtable = &_Animal_vtable;
    a.__type = "Animal";
    a.name = "Generic";
    a.age = 5;
    printf("%s\n", a._vtable->speak(&a));
    printf("%s\n", a._vtable->describe(&a));
    Dog d;
    d._vtable = &_Dog_vtable;
    d.__type = "Dog";
    d._base.name = "Rex";
    d._base.age = 3;
    d.breed = "Labrador";
    printf("%s\n", d._vtable->speak(&d));
    printf("%d\n", d._vtable->describe(&d));
    printf("%s\n", d._vtable->fetch(&d));
    Cat c;
    c._vtable = &_Cat_vtable;
    c.__type = "Cat";
    c._base.name = "Whiskers";
    c._base.age = 7;
    printf("%s\n", c._vtable->speak(&c));
    printf("%d\n", c._vtable->describe(&c));
    return 0;
}

char* Animal_speak(Animal* self)
{
    return (self->name + " makes a sound");
}

char* Animal_describe(Animal* self)
{
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", self->age);
    return __ul_strcat((self->name + " age "), _cast_buf_0);
}

char* Dog_speak(Dog* self)
{
    return (self->_base.name + " barks");
}

char* Dog_fetch(Dog* self)
{
    return (self->_base.name + " fetches the ball");
}

char* Cat_speak(Cat* self)
{
    return (self->_base.name + " meows");
}

static Animal_VTable _Animal_vtable = {(void*)Animal_speak, (void*)Animal_describe};
static Dog_VTable _Dog_vtable = {(void*)Dog_speak, (void*)Animal_describe, (void*)Dog_fetch};
static Cat_VTable _Cat_vtable = {(void*)Cat_speak, (void*)Animal_describe};

