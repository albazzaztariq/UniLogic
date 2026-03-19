#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

