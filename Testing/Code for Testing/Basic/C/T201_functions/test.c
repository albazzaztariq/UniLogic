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

int add(int a, int b);
char* greet(char* who);
int square(int n);
int is_positive(int n);
int factorial(int n);
void no_return(void);
int main(void);

int add(int a, int b)
{
    return (a + b);
}

char* greet(char* who)
{
    return __ul_strcat("Hello ", who);
}

int square(int n)
{
    return (n * n);
}

int is_positive(int n)
{
    if ((n > 0)) {
        return 1;
    }
    return 0;
}

int factorial(int n)
{
    if ((n <= 1)) {
        return 1;
    }
    return (n * factorial((n - 1)));
}

void no_return(void)
{
    printf("%s\n", "side effect");
}

int main(void)
{
    printf("%d\n", add(3, 7));
    printf("%s\n", greet("World"));
    printf("%d\n", square(6));
    printf("%d\n", is_positive(5));
    printf("%d\n", is_positive((-3)));
    printf("%d\n", factorial(5));
    no_return();
    printf("%d\n", add(square(2), square(3)));
    return 0;
}


