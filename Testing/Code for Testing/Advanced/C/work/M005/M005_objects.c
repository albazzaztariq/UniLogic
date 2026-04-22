#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* __ul_char_from_code(int code) {
    char* r = (char*)__ul_malloc(2); r[0] = (char)code; r[1] = 0; return r; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }
static void __ul_print_float(double v) {
    char _buf[64]; snprintf(_buf, sizeof(_buf), "%.10g", v);
    int _has_dot = 0; for (int _i = 0; _buf[_i]; _i++) if (_buf[_i] == '.' || _buf[_i] == 'e') { _has_dot = 1; break; }
    if (!_has_dot) { int _l = (int)strlen(_buf); _buf[_l] = '.'; _buf[_l+1] = '0'; _buf[_l+2] = 0; }
    printf("%s\n", _buf); }

typedef struct {
    int x;
    int y;
} Point;

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
} Cat_VTable;

struct Cat {
    Animal _base;
    Cat_VTable* _vtable;
    const char* __type;
};

int main(void);
char* Animal_speak(Animal* self);
char* Dog_speak(Dog* self);
char* Dog_fetch(Dog* self);
char* Cat_speak(Cat* self);
static Animal_VTable _Animal_vtable;
static Dog_VTable _Dog_vtable;
static Cat_VTable _Cat_vtable;

int main(void)
{
    Point p = (Point){.x = 10, .y = 20};
    printf("%d\n", p.x);
    printf("%d\n", p.y);
    Animal a;
    a._vtable = &_Animal_vtable;
    a.__type = "Animal";
    a.name = "Generic";
    a.age = 5;
    printf("%s\n", a._vtable->speak(&a));
    Dog d;
    d._vtable = &_Dog_vtable;
    d.__type = "Dog";
    d._base.name = "Rex";
    d._base.age = 3;
    d.breed = "Labrador";
    printf("%s\n", d._vtable->speak(&d));
    printf("%s\n", d._vtable->fetch(&d));
    Cat c;
    c._vtable = &_Cat_vtable;
    c.__type = "Cat";
    c._base.name = "Whiskers";
    c._base.age = 7;
    printf("%s\n", c._vtable->speak(&c));
    return 0;
}

char* Animal_speak(Animal* self)
{
    return __ul_strcat(self->name, " makes a sound");
}

char* Dog_speak(Dog* self)
{
    return __ul_strcat(self->_base.name, " barks");
}

char* Dog_fetch(Dog* self)
{
    return __ul_strcat(self->_base.name, " fetches the ball");
}

char* Cat_speak(Cat* self)
{
    return __ul_strcat(self->_base.name, " meows");
}

static Animal_VTable _Animal_vtable = {(void*)Animal_speak};
static Dog_VTable _Dog_vtable = {(void*)Dog_speak, (void*)Dog_fetch};
static Cat_VTable _Cat_vtable = {(void*)Cat_speak};

