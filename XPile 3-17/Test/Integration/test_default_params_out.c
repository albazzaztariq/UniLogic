#include <stdio.h>
#include <stdint.h>
#include <string.h>

char* greet(char* name, char* greeting);
int add(int a, int b);
int main(void);

char* greet(char* name, char* greeting)
{
    static char _ibuf0[1024];
    snprintf(_ibuf0, sizeof(_ibuf0), "%s, %s!", greeting, name);
    return _ibuf0;
}

int add(int a, int b)
{
    return (a + b);
}

int main(void)
{
    printf("%s\n", greet("World", "Hello"));
    printf("%s\n", greet("World", "Hi"));
    printf("%d\n", add(5, 10));
    printf("%d\n", add(5, 20));
    return 0;
}


