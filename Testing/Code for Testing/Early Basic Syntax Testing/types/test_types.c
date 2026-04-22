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
    int x;
    int y;
} Point;

typedef struct {
    char* name;
    int age;
    Point location;
} Person;

void print_point(Point p);
Point make_point(int x, int y);
int main(void);

void print_point(Point p)
{
    printf("%d\n", p.x);
    printf("%d\n", p.y);
}

Point make_point(int x, int y)
{
    Point result;
    result.x = x;
    result.y = y;
    return result;
}

int main(void)
{
    Point p;
    p.x = 10;
    p.y = 20;
    printf("%d\n", p.x);
    printf("%d\n", p.y);
    Person bob;
    bob.name = "Bob";
    bob.age = 30;
    bob.location.x = 100;
    bob.location.y = 200;
    printf("%s\n", bob.name);
    printf("%d\n", bob.age);
    printf("%d\n", bob.location.x);
    printf("%d\n", bob.location.y);
    print_point(p);
    Point q;
    q = make_point(5, 15);
    printf("%d\n", q.x);
    printf("%d\n", q.y);
    return 0;
}


