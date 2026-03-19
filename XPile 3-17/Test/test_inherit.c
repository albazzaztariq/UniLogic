#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

void describe(Dog d);
void main(void);

void describe(Dog d)
{
    printf("%s\n", d._base.name);
    printf("%d\n", d._base.age);
    printf("%s\n", d.breed);
}

void main(void)
{
    Animal a;
    a.name = "Cat";
    a.age = 5;
    printf("%s\n", a.name);
    printf("%d\n", a.age);
    Dog d;
    d._base.name = "Rex";
    d._base.age = 3;
    d.breed = "Labrador";
    printf("%s\n", d._base.name);
    printf("%d\n", d._base.age);
    printf("%s\n", d.breed);
    describe(d);
    Puppy p;
    p._base._base.name = "Tiny";
    p._base._base.age = 1;
    p._base.breed = "Beagle";
    p.weight = 8;
    printf("%s\n", p._base._base.name);
    printf("%d\n", p._base._base.age);
    printf("%s\n", p._base.breed);
    printf("%d\n", p.weight);
}

