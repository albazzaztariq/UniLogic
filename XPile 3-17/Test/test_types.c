#include <stdio.h>
#include <stdint.h>
#include <string.h>

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
void main(void);

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

void main(void)
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
}

