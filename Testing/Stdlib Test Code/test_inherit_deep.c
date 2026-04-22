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
    char* name;
    int age;
} Animal;

typedef struct {
    Animal _base;
    char* breed;
} Dog;

typedef struct {
    Dog _base;
    int weight;
} Puppy;

void show_animal(Animal a);
void show_dog(Dog d);
void show_puppy(Puppy p);
int main(void);

void show_animal(Animal a)
{
    printf("%s\n", a.name);
    printf("%d\n", a.age);
}

void show_dog(Dog d)
{
    printf("%s\n", d._base.name);
    printf("%d\n", d._base.age);
    printf("%s\n", d.breed);
}

void show_puppy(Puppy p)
{
    printf("%s\n", p._base._base.name);
    printf("%d\n", p._base._base.age);
    printf("%s\n", p._base.breed);
    printf("%d\n", p.weight);
}

int main(void)
{
    Animal a;
    a.name = "Tiger";
    a.age = 10;
    show_animal(a);
    Dog d;
    d._base.name = "Rex";
    d._base.age = 5;
    d.breed = "Husky";
    show_dog(d);
    Puppy p;
    p._base._base.name = "Tiny";
    p._base._base.age = 1;
    p._base.breed = "Poodle";
    p.weight = 3;
    show_puppy(p);
    p._base._base.name = "Spot";
    p._base._base.age = 2;
    p._base.breed = "Beagle";
    p.weight = 5;
    show_puppy(p);
    return 0;
}


